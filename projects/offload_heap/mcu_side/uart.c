/*
 * Based on uart.c example by Furkan Cayci
 */

#include "uart.h"

char msg_buffer[BUFFERSIZE] = {0};

// Send content of pointer through uart
void uart_send(void * data, size_t size) {
	for (size_t i=0; i<size; i++){
		// Send character
		USART1->DR = ((char *)data)[i];
		// Wait for transmit complete
		while(!(USART1->SR & (1 << 6)));
	}
}

// Receive size bytes of content from uart and write it to buffer
void uart_receive(void * buffer, size_t size)  {
	// USART CR2 configure stop bit count, default 1
	for (size_t i=0; i < size; i++) {
		// Wait until RXNE bit is set
		while (!(USART1->SR & (0x1U << 5))){};
		// Receive character
		((char *)buffer)[i] = USART1->DR;
	}
}

// Setup GPIO B6 and B7 pins for UART
static void uart_pin_setup(void) {
    // Enable GPIOB clock, bit 0 on AHB1ENR
    RCC->AHB1ENR |= (1 << 1);

    // Set pin modes as alternate mode (pins 6 and 7)
    // USART1 TX and RX pins are PB6 and PB7 respectively
    GPIOB->MODER &= ~(0xFU << 12); // Reset bits
    GPIOB->MODER |=  (0xAU << 12); // Set to alternate function mode

    // Set pin modes as high speed
    GPIOB->OSPEEDR |= (0xFU << 12);

    // Choose AF7 for USART1 in Alternate Function registers
    GPIOB->AFR[0] |= (0x7 << 24);
    GPIOB->AFR[0] |= (0x7 << 28);
}

// Initialize UART 1
static void uart_enable(void) {
	// Enable clock: bit 4 on APB2ENR
    RCC->APB2ENR |= (1 << 4);

    // USART1 RX enable, RE bit 2
    USART1->CR1 |= (1 << 2);
    // USART1 TX enable, TE bit 3
    USART1->CR1 |= (1 << 3);

    // Enable usart1 - UE, bit 13
    USART1->CR1 |= (1 << 13);

	// fCK = APB2 speed, 100MHz
    // baud rate = fCK / (8 * (2 - OVER8) * USARTDIV)
	// For STM32F411: fCK = 100 Mhz (Sysclk/4), Baudrate = 4000000, OVER8 = 0
	// USARTDIV = fCK / baud / 8 * (2-OVER8)
	// USARTDIV = 100M / 4000000 / 16 = 1.5625
	// Fraction: 0.5624*16 = 9
	// Mantissa: 1
    USART1->BRR |= (1 << 4); // Mantissa
    USART1->BRR |= 9; // Fraction
}

void uart_init(void)
{
    /* set system clock to 100 Mhz */
    set_sysclk_to_100();

	uart_pin_setup();
	uart_enable();
}
