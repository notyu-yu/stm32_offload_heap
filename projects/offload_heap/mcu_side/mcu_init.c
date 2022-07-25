#include "mcu_init.h"
#include "mcu_mm.h"
#include "mcu_timer.h"

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

// Memory fault handler
void MemManage_Handler(void) {
	// Force reset stack pointer in case of overflow
	sp_reset = (void *)0x20005000;
	asm volatile ("mov sp, %0" : "+r" (sp_reset));

	char err[] = "Memory error";
	var_print(err);
	mm_finish();
	loop();
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

	NVIC_SetPriority(WWDG_IRQn, 7);
	NVIC_EnableIRQ(WWDG_IRQn);
}

// Initialize memory fault handler
void memfault_init(void) {
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk; // Enable memfault, bit 16
	NVIC_SetPriority(MemoryManagement_IRQn, 0);
}

// Turn on LED
void led_on(led l) {
	GPIOD->ODR |= (1U<<l);
}

// Turn off LED
void led_off(led l) {
	GPIOD->ODR &= ~(1U<<l);
}

// Toggle LED
void led_toggle(led l) {
	GPIOD->ODR ^= (1U<<l);
}

// Setup LED GPIO
void led_init(void) {
	// Enable GPIOD clock
	RCC->AHB1ENR |= 0x00000008;

	// Turn on output mode
	GPIOD->MODER &= 0x00FFFFFF;
	GPIOD->MODER |= 0x55000000;

	// Turn off LEDs
	GPIOD->ODR &= 0x0FFF;
}

// Set up LED and fault handlers
void mcu_init(void) {
	//wwdg_init();
	memfault_init();
	led_init();
	// Make SVC call priority 3
	NVIC_SetPriority(SVCall_IRQn, 3);
}
