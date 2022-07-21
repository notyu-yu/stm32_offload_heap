/*
 * Modified from Furkan Cayci's STM32F407 timer.c example
 */
#include "mcu_timer.h"
#include "memlib.h"
#include "mcu_mm.h"
#include "uart.h"

#define MAXLINE 1024

static size_t systime = 0;

// Hardfault Handler - Send exit signal
void HardFault_Handler(void) {
	// Force reset stack pointer in case of overflow
	sp_reset = (void *)0x20005000;
	asm volatile ("mov sp, %0" : "+r" (sp_reset));

	char err[] = "Hard Fault";
	var_print(err);
	mm_finish();
	loop();
}

// Timer interrupt stopped running - Send exit signal
void WWDG_IRQHandler(void) {
	// Force reset stack pointer in case of overflow
	sp_reset = (void *)0x20005000;
	asm volatile ("mov sp, %0" : "+r" (sp_reset));

	char err[] = "WWDG error";
	var_print(err);
	mm_finish();
	loop();
}

/*************************************************
* timer 2 interrupt handler
*************************************************/
void TIM2_IRQHandler(void)
{
	systime++;
	register size_t * stack_top asm("sp");

	// Reset watchdog bits
	WWDG->CR |= 0x7F;

	// Stall if stack is overflowing to heap
	if (mem_heap_hi() > (void *)(stack_top)) {
		sprintf(msg, "Stack overflow detected");
		var_print(msg);
		mm_finish();
		loop();
	}

    // clear interrupt status
    if (TIM2->DIER & 0x01) {
        if (TIM2->SR & 0x01) {
            TIM2->SR &= ~(1U << 0);
        }
    }
}

// Returns system time in ms
size_t get_time(void) {
	return systime;
}

// Initialize WWDG
void wwdg_init(void) {
	// Enable clock
	RCC->APB1ENR |= (1<<11);

	// timeout (ms) = T_PCLK1(with ms) * 4096 * 2 ^(P) * (T + 1)
	// Max and min timeout calculated with T being 0x3F (63) and 0x00
	// APB1 Frequency 25 Mhz - Period: T_PCLK1 = 1/25,000 (in ms)
	// Min timeout: (1/25,000) * 4096 * 2^1 * 1 = 0.328 ms
	// Max timeout: (1/25,000) * 4096 * 2^1 * (63 + 1) = 20.972 ms

	WWDG->CFR |= (0x1 << 7); // Set timer base/prescaler - P
	WWDG->CFR |= (0x70); // Window countdown value - T
	WWDG->CR |= (0xFF); // Enable WDGA

	NVIC_SetPriority(WWDG_IRQn, 1);
	NVIC_EnableIRQ(WWDG_IRQn);
}

/*************************************************
* main code starts from here
*************************************************/
void timer_init(void)
{
    /* set system clock to 100 Mhz */
    set_sysclk_to_100();

    // enable TIM2 clock (bit0)
    RCC->APB1ENR |= (1 << 0);

	// For STM32F411: 100M/4*2 = 50M, 50M/4999+1 = 10 khz clock speed
    TIM2->PSC = 4999;

	// Set auto reload value to 10 to give 1 ms timer interrupts
    TIM2->ARR = 10;

    // Update Interrupt Enable
    TIM2->DIER |= (1 << 0);

    NVIC_SetPriority(TIM2_IRQn, 2); // Priority level 2
    // enable TIM2 IRQ from NVIC
    NVIC_EnableIRQ(TIM2_IRQn);

    // Enable Timer 2 module (CEN, bit0)
    TIM2->CR1 |= (1 << 0);

	wwdg_init();
}
