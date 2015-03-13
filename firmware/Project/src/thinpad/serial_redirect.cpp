#include "serial_redirect.h"
#include "HTTPServer.h"
#include "HTTPWebSocketHandler.h"
#include "websock_app.h"
#include "usart.h"
#include "circular_buffer.h"
#include "double_buffer.h"
#include "common.h"
#include "tasks.h"

static SerialDataHandler handler;
static struct CircularBuffer* to_tp_buffer = NULL;
static struct DoubleBuffer* from_tp_buffer = NULL;
static struct DoubleBuffer* acquisition_buffer = NULL;
static volatile bool tx_running, serial_open;
static bool network_sending, pending_ac_stop;
static uint8_t sending_type;

static volatile uint32_t acquisition_cnt;
static volatile uint8_t acquisition_level;

static void acquisition_timer_config(void)
{
    RCC_APB1PeriphClockCmd(THINPAD_SERIAL_AC_TIMER_RCC, ENABLE);

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Period = 2-1;
    TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/2/1000000/2-1;
    // TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV2;
    TIM_TimeBaseInit(THINPAD_SERIAL_AC_TIMER, &TIM_TimeBaseStructure);
    TIM_Cmd(THINPAD_SERIAL_AC_TIMER, DISABLE);

    TIM_ITConfig(THINPAD_SERIAL_AC_TIMER, TIM_IT_Update, ENABLE);
}

static void acquisition_timer(bool start)
{
    TIM_Cmd(THINPAD_SERIAL_AC_TIMER, start ? ENABLE : DISABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = THINPAD_SERIAL_AC_TIMER_IRQ; //指定中断源
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = THINPAD_SERIAL_AC_PreemptionPriority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   // 指定响应优先级别
    NVIC_InitStructure.NVIC_IRQChannelCmd = start ? ENABLE : DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static WebSocketDataHandler* ObtainDataHandler(const char* url)
{
  return handler.IsConnected() ? NULL : &handler; //prohibit multi-connection
}

static void NVIC_Config(bool bEnabled)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = THINPAD_SERIAL_UART_IRQ; //指定中断源
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = THINPAD_SERIAL_UART_PreemptionPriority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   // 指定响应优先级别
    NVIC_InitStructure.NVIC_IRQChannelCmd = (bEnabled ? ENABLE : DISABLE);
    NVIC_Init(&NVIC_InitStructure);
}

void SerialRedirect_Init(HTTPServer* httpd)
{
    httpd->addHandler(new HTTPWebSocketHandler("/thinpad/serial", ObtainDataHandler));
    if(!to_tp_buffer){
        to_tp_buffer = CircularBuffer_New(THINPAD_SERIAL_N2T_BUF_SIZE);
        if(!to_tp_buffer){
            ERR_MSG("Cannot allocate memory for N2T buffer!");
        }
    }else{
        CircularBuffer_Clear(to_tp_buffer);
    }
    if(!from_tp_buffer){
        from_tp_buffer = DoubleBuffer_New(THINPAD_SERIAL_T2N_BUF_SIZE);
        if(!from_tp_buffer){
            ERR_MSG("Cannot allocate memory for T2N buffer!");
        }
    }else{
        DoubleBuffer_Clear(from_tp_buffer);
    }
    if(!acquisition_buffer){
        acquisition_buffer = DoubleBuffer_New(THINPAD_SERIAL_AC_BUF_SIZE);
        if(!acquisition_buffer){
            ERR_MSG("Cannot allocate memory for acquisition buffer!");
        }
    }else{
        DoubleBuffer_Clear(acquisition_buffer);
    }
    acquisition_timer_config();
}

void SerialRedirect_Close(void)
{
    USART_Cmd(THINPAD_SERIAL_UART, DISABLE);
    serial_open = false;
}

void SerialRedirect_Open(int baud)
{
    if(baud < THINPAD_SERIAL_BAUD_MIN || baud > THINPAD_SERIAL_BAUD_MAX){
        ERR_MSG("Incorrect baudrate");
        return;
    }
    USART_Config(THINPAD_SERIAL_UART, baud);
    USART_ITConfig(THINPAD_SERIAL_UART, USART_IT_TC, ENABLE);
    USART_ITConfig(THINPAD_SERIAL_UART, USART_IT_RXNE, ENABLE);
    NVIC_Config(true);
    serial_open = true;
}

void SerialRedirect_ToThinpad(uint8_t* data, int len)
{
    if(!serial_open){
        ERR_MSG("Serial is not open");
        return;
    }
    for (int i = 0; i < len; ++i)
    {
        if(!CircularBuffer_Push(to_tp_buffer, data[i])){
            ERR_MSG("Buffer is full");
            break;
        }
    }
    uint8_t byte;
    if(!tx_running && CircularBuffer_Pop(to_tp_buffer, &byte)){
        tx_running = true;
        fputc('s',stderr);
        USART_putchar(THINPAD_SERIAL_UART, byte);
    }
}

void SerialRedirect_Acquisition(bool enable)
{
    DBG_MSG("bool %d", enable);
    if(enable){
        acquisition_cnt = 0;
        acquisition_level = 1;
        acquisition_timer(true);
    }else{
        acquisition_timer(false);
        pending_ac_stop = true;
    }
}


void SerialRedirect_Task(void)
{
    if(!network_sending) {
        static char tmp[16];

        if(DoubleBuffer_Size(from_tp_buffer) > 0){
            uint8_t *ptr;
            network_sending = true;
            fputc('y',stderr);
            sprintf(tmp, "S%u", 1);
            sending_type = 1;
            handler.SendFrameAsync((void*)tmp, strlen(tmp));
        }
        else if(DoubleBuffer_Size(acquisition_buffer) > 0){
            network_sending = true;
            fputc('a',stderr);
            sending_type = 2;
            handler.SendFrameAsync((void*)"A", 1);
        }
        else if(pending_ac_stop){
            pending_ac_stop = false;

            sprintf(tmp, "E%u", acquisition_cnt);
            
            network_sending = true;
            sending_type = 0;
            handler.SendFrameAsync((void*)tmp, strlen(tmp));
        }
    }
}

void SerialRedirect_FrameSent(void)
{
    if(sending_type == 1){
        if(DoubleBuffer_Size(from_tp_buffer) > 0){
            uint8_t *ptr;
            int size = DoubleBuffer_SwapBuffer(from_tp_buffer, &ptr);
            network_sending = true;
            fputc('t',stderr);
            handler.SendFrameAsync((void*)ptr, size, false);
        }else{
            network_sending = false;
            fputc('d',stderr);
        }
    }else if(sending_type == 2){
        if(DoubleBuffer_Size(acquisition_buffer) > 0){
            uint8_t *ptr;
            int size = DoubleBuffer_SwapBuffer(acquisition_buffer, &ptr);
            network_sending = true;
            fputc('t',stderr);
            handler.SendFrameAsync((void*)ptr, size, false);
        }else{
            network_sending = false;
            fputc('d',stderr);
        }
    }else{
        network_sending = false;
    }
}

void SerialRedirect_AcquisitionTimerInterrupt(void)
{
    if (TIM_GetITStatus(THINPAD_SERIAL_AC_TIMER, TIM_IT_Update) != RESET)
    {
        uint8_t bit = GPIO_ReadInputDataBit(THINPAD_SERIAL_AC_GPIO, THINPAD_SERIAL_AC_PIN);
        // fputc('i',stderr);
        if(bit ^ acquisition_level){
            // fputc('x',stderr);
            do{
                uint8_t byte = acquisition_cnt & 0x7f;
                acquisition_cnt >>= 7;
                if(acquisition_cnt > 0)
                    byte |= 0x80;
                DoubleBuffer_Push(acquisition_buffer, byte);
            }while(acquisition_cnt > 0);

            acquisition_level = bit;
            // acquisition_cnt = 0;
        }else{
            acquisition_cnt++;
        }
        TIM_ClearITPendingBit(THINPAD_SERIAL_AC_TIMER, TIM_IT_Update);
    }
}

void SerialRedirect_UsartInterrupt(void)
{
    uint8_t byte;
    if(USART_GetITStatus(THINPAD_SERIAL_UART, USART_IT_TC) != RESET){
        if(CircularBuffer_Pop(to_tp_buffer, &byte)) //fetch the next byte to transmit
            USART_putchar(THINPAD_SERIAL_UART, byte);
        else{
            USART_ClearFlag(THINPAD_SERIAL_UART, USART_FLAG_TC);
            fputc('e',stderr);
            tx_running = false;
        }
    }
    if(USART_GetITStatus(THINPAD_SERIAL_UART, USART_IT_RXNE) != RESET){
        byte = USART_getchar(THINPAD_SERIAL_UART);
        DoubleBuffer_Push(from_tp_buffer, byte);
    }
}
