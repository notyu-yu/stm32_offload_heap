/*
 * Modified from Furkan Cayci's STM32F407 timer.c example
 */
#include "mcu_timer.h"
#include "memlib.h"

#define MAXLINE 1024

static size_t systime = 0;

// TIM2 interrupt - count system time
void TIM2_IRQHandler(void)
{
	register size_t * stack_top asm("sp");

	// Stall if stack is overflowing to heap
	if (mem_heap_hi() > (void *)(stack_top)) {
		sprintf(msg, "Stack overflow detected");
		var_print(msg);
		loop();
	}

	systime++;

    // clear interrupt status
    if (TIM2->DIER & 0x01) {
        if (TIM2->SR & 0x01) {
            TIM2->SR &= ~(1U << 0);
        }
    }
}

// Returns system time in 1 ms
size_t get_time(void) {
	return systime;
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
    NVIC_SetPriority(TIM2_IRQn, 35); // Priority level 2
    // enable TIM2 IRQ from NVIC
    NVIC_EnableIRQ(TIM2_IRQn);
    // Enable Timer 2 module (CEN, bit0)
    TIM2->CR1 |= (1 << 0);
}
