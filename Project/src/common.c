/*
 * =====================================================================================
 *
 *       Filename:  common.c
 *
 *    Description:  项目的公共函数实现
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

#include "common.h"

void RCC_GPIOClockCmd(GPIO_TypeDef* GPIOx, FunctionalState state)
{
    uint32_t rcc;
    switch((uint32_t)GPIOx) {
        case (uint32_t)GPIOA:
            rcc = RCC_AHB1Periph_GPIOA;
            break;
        case (uint32_t)GPIOB:
            rcc = RCC_AHB1Periph_GPIOB;
            break;
        case (uint32_t)GPIOC:
            rcc = RCC_AHB1Periph_GPIOC;
            break;
        case (uint32_t)GPIOD:
            rcc = RCC_AHB1Periph_GPIOD;
            break;
        case (uint32_t)GPIOE:
            rcc = RCC_AHB1Periph_GPIOE;
            break;
        case (uint32_t)GPIOF:
            rcc = RCC_AHB1Periph_GPIOF;
            break;
        case (uint32_t)GPIOG:
            rcc = RCC_AHB1Periph_GPIOG;
            break;
        case (uint32_t)GPIOH:
            rcc = RCC_AHB1Periph_GPIOH;
            break;
        case (uint32_t)GPIOI:
            rcc = RCC_AHB1Periph_GPIOI;
            break;
        default:
            /* Invalid argument! */
            return;
    }
    RCC_AHB1PeriphClockCmd(rcc, state);
}


