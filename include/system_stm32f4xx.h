#ifndef __SYSTEM_STM32F4XX_H
#define __SYSTEM_STM32F4XX_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"

#define HSI_VALUE    ((uint32_t)100000000U) // Internal high speed oscillator
#define HSE_VALUE    ((uint32_t)8000000U)  // External high speed oscillator
#define LSI_VALUE    ((uint32_t)32000U)    // Internal low speed oscillator
#define LSE_VALUE    ((uint32_t)32768U)    // External low speed oscillator

#define PLL_M      8
#define PLL_N      384
#define PLL_P      4
#define PLL_Q      8

void __init_data(void);
void __Reset_Handler(void);
void SystemInit(void);
void set_sysclk_to_100(void);
/* bring main */
extern int main(void);

#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_STM32F4XX_H */
