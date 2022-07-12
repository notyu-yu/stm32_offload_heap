/*
 * Based on uart.c example by Furkan Cayci
 */

#include "uart.h"

char msg_buffer[BUFFERSIZE] = {0};

// Send content of pointer through uart
void uart_send(void * data, size_t size) {
	for (size_t i=0; i<size; i++){
		// Send character
		USART2->DR = ((char *)data)[i];
		// Wait for transmit complete
		while(!(USART2->SR & (1 << 6)));
	}
}

// Receive size bytes of content from uart and write it to buffer
void uart_receive(void * buffer, size_t size)  {
	// USART CR2 configure stop bit count, default 1
	// USART2->CR2 &= ~(0x3U << 12);
	// USART2->CR2 != (0x0U << 12);
	for (size_t i=0; i < size; i++) {
		// Wait until RXNE bit is set
		while (!(USART2->SR & (0x1U << 5))){};
		// Receive character
		((char *)buffer)[i] = USART2->DR;
	}
}

// Setup GPIO A2 and A3 pins for UART
static void uart_pin_setup(void) {
    // Enable GPIOA clock, bit 0 on AHB1ENR
    RCC->AHB1ENR |= (1 << 0);

    // Set pin modes as alternate mode 7 (pins 2 and 3)
    // USART2 TX and RX pins are PA2 and PA3 respectively
    GPIOA->MODER &= ~(0xFU << 4); // Reset bits 4:5 for PA2 and 6:7 for PA3
    GPIOA->MODER |=  (0xAU << 4); // Set   bits 4:5 for PA2 and 6:7 for PA3 to alternate mode (10)

    // Set pin modes as high speed
    GPIOA->OSPEEDR |= 0x000000A0; // Set pin 2/3 to high speed mode (0b10)

    // Choose AF7 for USART2 in Alternate Function registers
    GPIOA->AFR[0] |= (0x7 << 8); // for pin A2
    GPIOA->AFR[0] |= (0x7 << 12); // for pin A3
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

// Initialize UART 2
static void uart_enable(void) {
    // enable USART2 clock, bit 17 on APB1ENR
    RCC->APB1ENR |= (1 << 17);

    // USART2 word length M, bit 12
    // USART2->CR1 |= (0 << 12); // 0 - 1,8,n

    // USART2 parity control, bit 9
    // USART2->CR1 |= (0 << 9); // 0 - no parity

    // USART2 RX enable, RE bit 2
    USART2->CR1 |= (1 << 2);
    // USART2 TX enable, TE bit 3
    USART2->CR1 |= (1 << 3);

    // Enable usart2 - UE, bit 13
    USART2->CR1 |= (1 << 13);

    // baud rate = fCK / (8 * (2 - OVER8) * USARTDIV)
	// For STM32F411: fCK = 25 Mhz (Sysclk/4), Baudrate = 115200, OVER8 = 0
	// USARTDIV = fCK / baud / 8 * (2-OVER8)
	// USARTDIV = 25Mhz / 115200 / 16 = 13.5633
	// Fraction: 0.5633*16 = 9
	// Mantissa: 13
    USART2->BRR |= (13 << 4); // Mantissa
    USART2->BRR |= 9; // Fraction
}	

void uart_init(void)
{
    /* set system clock to 100 Mhz */
    set_sysclk_to_100();

	uart_pin_setup();
	uart_enable();
}
