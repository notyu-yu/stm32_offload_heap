#include "uart_dma.h"

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

int main(void) {
	char debug_msg[BUFFERSIZE] = {0};
	char * stripped_msg;
	size_t received_size=0;

	uart_init();
	led_init();
	uart_dma_init();

	while(1) {
		// Read received buffer size
		led_on(GREEN);
		receive(msg_buffer, sizeof(size_t));
		received_size = READSIZE(msg_buffer) + PADDINGSIZE;
		memset(msg_buffer, 0, sizeof(size_t));
		led_off(GREEN);

		// Send back buffer content
		if (received_size > BUFFERSIZE) {
			// Message too large
			led_on(ORANGE);
			sprintf(debug_msg, "Attempting to read %u bytes larger than buffer size %d.\n", (unsigned int)received_size, BUFFERSIZE);
			send(debug_msg, strlen(debug_msg));
			led_off(ORANGE);
		} else {
			// Read message
			led_on(RED);
			receive(msg_buffer, received_size);
			led_off(RED);

			// Echo message
			led_on(BLUE);
			stripped_msg = msg_offset(msg_buffer);
			send(stripped_msg, strlen(stripped_msg));
			memset(msg_buffer, 0, received_size);
			led_off(BLUE);
		}
	}
}
