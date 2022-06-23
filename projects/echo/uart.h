#include "../../include/stm32f411xe.h"
#include "../../include/system_stm32f4xx.h"

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Message buffer
extern char msg_buffer[BUFFERSIZE];

// UART Functions
void uart_init(void); // Initialize uart registers
void uart_send(void * data, size_t size); // Send size bytes of data starting at data pointer
void uart_receive(void * buffer, size_t size); // Receive size bytes of data and write to buffer
void uart_wait_receive(void * buffer, size_t size); // Same as receive but stall until message is sent

typedef enum {
	GREEN=12,
	ORANGE,
	RED,
	BLUE
} led;

// LED functions
void led_init(void); // Initialize LED GPIO pins
void led_on(led l); // Turn on LED
void led_off(led l); // Turn off LED
void led_toggle(led l); // Toggle LED state
