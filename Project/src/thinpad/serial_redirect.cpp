#include "serial_redirect.h"
#include "HTTPServer.h"
#include "HTTPWebSocketHandler.h"
#include "websock_app.h"
#include "usart.h"
#include "circular_buffer.h"
#include "common.h"
#include "tasks.h"

static SerialDataHandler handler;
static struct CircularBuffer* to_tp_buffer = NULL;
static volatile bool tx_running, serial_open;

static WebSocketDataHandler* ObtainDataHandler(const char* url)
{
  return handler.IsConnected() ? NULL : &handler; //prohibit multi-connection
}

static void NVIC_Config(bool bEnabled)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = THINPAD_SERIAL_UART_IRQ; //指定中断源
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
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

void SerialRedirect_Task(void)
{
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
    }
}
