#include "mcu_request.h"

#define READSIZE(buffer) *(size_t *)buffer

// Returns pointer to msg without padding
static char * msg_offset(char * msg) {
	for (size_t i=0; i<BUFFERSIZE; i++) {
		if (msg[i]) {
			return msg + i;
		}
	}
	return msg;
}

// Send size bytes at data pointer, using method defined by USE_DMA macro
static void send(void * data, size_t size) {
	if (USE_DMA) {
		uart_tx_start(data, size);
		uart_tx_wait();
	} else {
		uart_send(data, size);
	}
}

// Receive size bytes at buffer pointer, using method defined by USE_DMA macro
static void receive(void * buffer, size_t size) {
	if (USE_DMA) {
		uart_rx_start(buffer, size);
		uart_rx_wait();
	} else {
		uart_receive(buffer, size);
	}
}

// Initialize request communication
void mem_req_setup(void) {
	uart_init();
	led_init();
	uart_dma_init();
}

// Send request
void req_send(mem_request * buffer) {
	send(buffer, sizeof(mem_request));
}

// Wait for response
void req_receive(mem_request * buffer) {
	receive(buffer, sizeof(mem_request));
}
