/*
 * =====================================================================================
 *
 *       Filename:  common.h
 *
 *    Description:  项目的公共函数和参数定义
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
#ifndef __COMMON__H__
#define __COMMON__H__

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>


#define DBG_MSG(format, ...) printf("[Debug]%s: " format "\r\n", __func__, ##__VA_ARGS__)
#define INFO_MSG(format, ...) printf("[Info]%s: " format "\r\n", __func__, ##__VA_ARGS__)
#define ERR_MSG(format, ...) printf("[Error]%s: " format "\r\n", __func__, ##__VA_ARGS__)

void RCC_GPIOClockCmd(GPIO_TypeDef* GPIOx, FunctionalState state);

typedef void (*Task_t)(void);

#endif /* __COMMON__H__ */