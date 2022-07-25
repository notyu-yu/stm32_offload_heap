#include "mcu_request.h"
#include "mcu_init.h"

#define READSIZE(buffer) *(size_t *)buffer

static char tx_buffer[16] = {0};

// Send size bytes at data pointer, using method defined by USE_DMA macro
static void send(void * data, size_t size) {
	if (USE_DMA) {
		uart_tx_wait();
		memcpy(tx_buffer, data, size);
		uart_tx_start(tx_buffer, size);
	} else {
		uart_send(data, size);
	}
}

// Receive size bytes at buffer pointer, using method defined by USE_DMA macro
static void receive(void * buffer, size_t size) {
	if (USE_DMA) {
		uart_tx_wait();
		uart_rx_start(buffer, size);
		uart_rx_wait();
	} else {
		uart_receive(buffer, size);
	}
}

// Initialize request communication
void mem_req_setup(void) {
	mcu_init();
	uart_init();
	uart_dma_init();
}

// Send request
void req_send(mem_request * buffer) {
	led_on(GREEN);
	send(buffer, sizeof(mem_request));
	led_off(GREEN);
}

// Wait for response
void req_receive(void ** buffer) {
	led_on(GREEN);
	receive(buffer, sizeof(void *));
	led_off(GREEN);
}
