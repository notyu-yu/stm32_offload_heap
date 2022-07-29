/*
 * Modified from Furkan Cayci's STM32F407 timer.c example
 */
#include "mcu_timer.h"
#include "memlib.h"
#include "mcu_mm.h"
#include "uart.h"

#define MAXLINE 1024

// Current systime runtime in ms
static size_t systime = 0;

// TIM2 interrupt handler - Update system time
void TIM2_IRQHandler(void)
{
	systime++;

	// Reset watchdog bits
	// WWDG->CR |= 0x7F;

    // clear interrupt status
    if (TIM2->DIER & 0x01) {
        if (TIM2->SR & 0x01) {
            TIM2->SR &= ~(1U << 0);
        }
    }
}

// TIM3 interrupt handler - Check for stack overflow
void TIM3_IRQHandler(void)
{
	// Check for stack overflow
	register size_t * stack_top asm("sp");

	// Stall if stack is overflowing to heap
	if (mem_heap_hi() > (void *)(stack_top)) {
		char err[] = "Stack overflow detected";
		var_print(err);
		mm_finish();
		loop();
	}	

    // clear interrupt status
    if (TIM3->DIER & 0x01) {
        if (TIM3->SR & 0x01) {
            TIM3->SR &= ~(1U << 0);
        }
    }

}

// Returns system time in 0.1 ms
size_t get_time(void) {
	return systime;
}

// Initialize timers
void timer_init(void)
{
	// TIM2 - Keeps track of system time

    // enable TIM2 clock (bit0)
    RCC->APB1ENR |= (1 << 0);
	// For STM32F411: 100M/4*2 = 50M, 50M/(4999+1) = 10 khz clock speed
    TIM2->PSC = 4999;
	// Set auto reload value to 10 to give 1ms timer interrupts
    TIM2->ARR = 10;
    // Update Interrupt Enable
    TIM2->DIER |= (1 << 0);
    NVIC_SetPriority(TIM2_IRQn, 2);
    // enable TIM2 IRQ from NVIC
    NVIC_EnableIRQ(TIM2_IRQn);
	// Set to upcounting mode
	TIM2->CR1 &= ~(1 << 4);
    // Enable Timer 2 module (CEN, bit0)
    TIM2->CR1 |= (1 << 0);

	// TIM3 - Checks for stack overflow
    
	// Enable TIM3 clock (bit1)
    RCC->APB1ENR |= (1 << 1);
	// For STM32F411: 100M/4*2 = 50M, 50M/4999+1 = 10 khz clock speed
    TIM3->PSC = 4999;
	// Set auto reload value to 100 to give 10 ms timer interrupts
    TIM3->ARR = 100;
    // Update Interrupt Enable
    TIM3->DIER |= (1 << 0);
    NVIC_SetPriority(TIM3_IRQn, 3);
    // enable TIM3 IRQ from NVIC
    NVIC_EnableIRQ(TIM3_IRQn);
	// Set to upcounting mode
	TIM3->CR1 &= ~(1 << 4);
    // Enable Timer 3 module (CEN, bit0)
    TIM3->CR1 |= (1 << 0);
}
