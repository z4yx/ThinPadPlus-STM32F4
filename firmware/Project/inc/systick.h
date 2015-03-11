#ifndef __SYSTICK_H
#define	__SYSTICK_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint64_t SysTick_t;

void SysTick_Init(void);
void IncSysTickCounter(void);
SysTick_t GetSystemTick(void);
void Delay_ms(unsigned int ms);
void Delay_us(unsigned int us);

#ifdef __cplusplus
};
#endif

#endif