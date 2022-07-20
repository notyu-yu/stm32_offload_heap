#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <assert.h>

#include "pc_request.h"

static int fd; // Serial device file descriptor
static char receive_buffer[BUFFERSIZE*2] = {0};
static char chunk_buffer[BUFFERSIZE*2] = {0};

// Concatenate size bytes of data from src to dest starting at start'th index
static void data_cat(char * dest, char * src, size_t start, size_t size) {
	for (size_t i=0; i<size; i++) {
		dest[start+i] = src[i];
	}
}

// Set up serial device
static void serial_setup(int fd) {
	struct termios serial_settings;
	tcgetattr(fd, &serial_settings);

	// Set buad rate
	cfsetispeed(&serial_settings, BAUDRATE);
	cfsetospeed(&serial_settings, BAUDRATE);

	// Set raw mode (no special processing)
	cfmakeraw(&serial_settings);
	
	serial_settings.c_cflag &= ~CRTSCTS; // Hardware based flow control off
	serial_settings.c_cflag |= CREAD | CLOCAL; // Turn on receiver

	// Read for 0.5 seconds at max
	// serial_settings.c_cc[VTIME] = 5;
	tcflush(fd, TCIOFLUSH); // Clear IO buffer
	tcsetattr(fd, TCSANOW, &serial_settings); // Apply settings
}

// Setup UART file descriptor
void uart_setup(void) {
	fd = open(SERIALDEV, O_RDWR | O_NOCTTY);
	assert(fd >= 0); // Error when not ran with sudo
	serial_setup(fd);
	assert(fd>=0);
}

// Wait for size bytes of data to be read into buffer from UART
static void uart_read(size_t size, void * buffer) {
	size_t bytes_read = 0;
	size_t chunk_read = 0;
	if (VERBOSE) {
		puts("pc receive start");
	}
	while (bytes_read < size) {
		chunk_read = read(fd, chunk_buffer, size-bytes_read);
		data_cat(buffer, chunk_buffer, bytes_read, chunk_read);
		bytes_read += chunk_read;
		memset(chunk_buffer, 0, BUFFERSIZE*2);
	}
	if (VERBOSE) {
		puts("pc receive end");
	}
}

// Send size bytes of data from buffer through UART
static void uart_send(size_t size, void * buffer) {
	if (VERBOSE) {
		puts("pc send start");
	}
	assert(write(fd, buffer, size));
	if (VERBOSE) {
		puts("pc send end");
	}
}

// Wait to receive a request and write struct to buffer
void req_receive(mem_request * buffer) {
	uart_read(sizeof(mem_request), buffer);
}

// Send a response stored in buffer back to mcu
void req_send(uint32_t * buffer) {
	uart_send(sizeof(uint32_t), buffer);
}
