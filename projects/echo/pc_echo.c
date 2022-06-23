#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "config.h"

// Set up serial device
static void serial_setup(int fd) {
	struct termios serial_settings;
	tcgetattr(fd, &serial_settings);

	// Set buad rate
	cfsetispeed(&serial_settings, BAUDRATE);
	cfsetospeed(&serial_settings, BAUDRATE);

	serial_settings.c_cflag &= ~CRTSCTS; // Hardware based flow control off
	serial_settings.c_cflag |= CREAD | CLOCAL; // Turn on receiver
	serial_settings.c_iflag &= ~(IXON | IXOFF | IXANY); // Software based flow control off, no parity marking
	serial_settings.c_lflag &= ~(ICANON | ECHO | ISIG); // Set operation mode, canonical, enable input echo and receiving signals
	serial_settings.c_oflag &= ~(ONLCR | OFILL); // Don't turn /n to /n/r, don't send fill characters
	// Read for 0.5 seconds at max
	// serial_settings.c_cc[VTIME] = 5;
	tcflush(fd, TCIOFLUSH); // Clear IO buffer
	tcsetattr(fd, TCSANOW, &serial_settings); // Apply settings
}

int main(int argc, char ** argv) {
	int fd;
	char send_buffer[BUFFERSIZE] = {0};
	char line_buffer[BUFFERSIZE] = {0};
	char receive_buffer[BUFFERSIZE*2] = {0};
	char echo_buffer[BUFFERSIZE*2] = {0};
	size_t msg_size = 0;
	size_t bytes_read = 0;
	size_t chunk_read = 0;

	// Open serial device
	fd = open(SERIALDEV, O_RDWR | O_NOCTTY);
	assert(fd>=0);
	serial_setup(fd);

	// Read message from stdin until buffer limit reached
	while(fgets(line_buffer, BUFFERSIZE, stdin)) {
		if (strlen(send_buffer) + strlen(line_buffer) < BUFFERSIZE - PADDINGSIZE) {
			strcat(send_buffer, line_buffer);
		} else {
			break;
		}
	}
	msg_size = strlen(send_buffer);

	// Send message to UART usb device and print return message
	if (VERBOSE) {
		printf("Sending message: %s\n", send_buffer);
		printf("Message size: %zu bytes\n", msg_size);
	}

	// Send message size
	assert(write(fd, &msg_size, sizeof(size_t)) == sizeof(size_t));
	tcdrain(fd);
	tcflush(fd, TCIOFLUSH); // Clear IO buffer

	// Send message content
	assert(write(fd, send_buffer, msg_size) == msg_size);
	tcdrain(fd);
	tcflush(fd, TCIOFLUSH); // Clear IO buffer

	// Read message in 64 bit chunks and print
	while (bytes_read < msg_size) {
		chunk_read = read(fd, receive_buffer, msg_size-bytes_read);
		bytes_read += chunk_read;
		strncat(echo_buffer, receive_buffer, chunk_read);
		memset(receive_buffer, 0, BUFFERSIZE*2);
	}

	puts(echo_buffer);
	
	if (VERBOSE) {
		printf("Read %zu bytes\n", bytes_read);
	}

	// Final checks
	if (strlen(echo_buffer) != strlen(send_buffer)) {
		printf("WARNING: echoed message length (%zu)  different from orignal message length (%zu).\n", strlen(echo_buffer), strlen(send_buffer));
	}
	if(strncmp(send_buffer, echo_buffer, BUFFERSIZE)) {
		printf("WARNING: echoed message different from message sent\n");
	}
	tcflush(fd, TCIOFLUSH); // Clear IO buffer

	return 0;
}
