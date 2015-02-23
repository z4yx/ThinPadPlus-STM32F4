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

static WebSocketDataHandler* ObtainDataHandler(const char* url)
{
  return handler.IsConnected() ? NULL : &handler; //prohibit multi-connection
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
}

void SerialRedirect_Open(int baud)
{
    USART_Config(THINPAD_SERIAL_UART, baud);
}

void SerialRedirect_ToThinpad(uint8_t* data, int len)
{
    for (int i = 0; i < len; ++i)
    {
        if(!CircularBuffer_Push(to_tp_buffer, data[i])){
            ERR_MSG("Buffer is full");
            break;
        }
    }
}

void SerialRedirect_Task(void)
{
    uint8_t byte;
    while(CircularBuffer_Pop(to_tp_buffer, &byte)){
        USART_putchar(THINPAD_SERIAL_UART, byte);
        putchar(byte);
    }
}
