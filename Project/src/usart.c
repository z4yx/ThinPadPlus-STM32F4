/*
 * =====================================================================================
 *
 *       Filename:  usart.c
 *
 *    Description:  USART收发与调试输出
 *
 *        Version:  
 *        Created:  
 *       Revision:  
 *       Compiler:  
 *
 *         Author:  zhangyuxiang
 *   Organization:  
 *
 * =====================================================================================
 */

#include "usart.h"
#include <stdarg.h>

/*
 * 初始化串口配置
 */
void USART_Config(USART_TypeDef* USARTx, u32 USART_BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    GPIO_TypeDef *USART_GPIO;
    uint16_t USART_Rx, USART_Tx;
    uint16_t PinSource_Rx, PinSource_Tx;
    uint8_t  GPIO_AF;

    switch((u32)USARTx) {
        case (u32)USART1:
            USART_GPIO = GPIOA;
            USART_Tx = GPIO_Pin_9;
            USART_Rx = GPIO_Pin_10;
            PinSource_Tx = GPIO_PinSource9;
            PinSource_Rx = GPIO_PinSource10;
            GPIO_AF  = GPIO_AF_USART1;
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
            break;
        case (u32)USART2:
            USART_GPIO = GPIOA;
            USART_Tx = GPIO_Pin_2;
            USART_Rx = GPIO_Pin_3;
            PinSource_Tx = GPIO_PinSource2;
            PinSource_Rx = GPIO_PinSource3;
            GPIO_AF  = GPIO_AF_USART2;
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
            break;
        case (u32)USART3:
            USART_GPIO = GPIOB;
            USART_Tx = GPIO_Pin_10;
            USART_Rx = GPIO_Pin_11;
            PinSource_Tx = GPIO_PinSource10;
            PinSource_Rx = GPIO_PinSource11;
            GPIO_AF  = GPIO_AF_USART3;
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
            break;

        default:
            return;
    }

    GPIO_PinAFConfig(USART_GPIO, PinSource_Rx, GPIO_AF);
    GPIO_PinAFConfig(USART_GPIO, PinSource_Tx, GPIO_AF);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART_Tx;

    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(USART_GPIO, &GPIO_InitStructure);

    /* Configure USART Rx as input*/
    GPIO_InitStructure.GPIO_Pin = USART_Rx;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(USART_GPIO, &GPIO_InitStructure);

    /* USART mode config */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);
    USART_Cmd(USARTx, ENABLE);
}

/*
 * 通过串口发送一个字节
 */
int USART_putchar(USART_TypeDef* USARTx, int ch)
{

    USART_SendData(USARTx, (unsigned char) ch);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);

    return (ch);
}

/*
 * 通过串口接收一个字节
 */
int USART_getchar(USART_TypeDef* USARTx)
{
    uint8_t ch;
    while (SET != USART_GetFlagStatus(USARTx, USART_FLAG_RXNE));
    ch = USART_ReceiveData(USARTx);
    USART_ClearFlag(USARTx, USART_FLAG_RXNE);
    return (ch);
}

/*
 * 数值转字符串函数
 */
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 1000000000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

/*
 * 通过串口发送格式化字符串
 */
void USART_printf(USART_TypeDef* USARTx, char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, Data);

    while ( *Data != 0)
    {
        if ( *Data == '\\' )
        {
            switch ( *++Data )
            {
            case 'r':
                USART_SendData(USARTx, 0x0d);
                Data ++;
                break;

            case 'n':
                USART_SendData(USARTx, 0x0a);
                Data ++;
                break;

            default:
                Data ++;
                break;
            }
        }
        else if ( *Data == '%')
        {
            switch ( *++Data )
            {
            case 's':
                s = va_arg(ap, const char *);
                for ( ; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while ( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
                }
                Data++;
                break;

            case 'd':
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    USART_SendData(USARTx, *s);
                    while ( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
                }
                Data++;
                break;
            default:
                Data++;
                break;
            }
        } /* end of else if */
        else USART_SendData(USARTx, *Data++);
        while ( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
    }
}
