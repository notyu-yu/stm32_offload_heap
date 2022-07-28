#include "uart_comms.h"

void uart_setup(void); // Setup uart device communications
void req_receive(mem_request * buffer); // Wait and receive request from mcu
void req_send(uint32_t * buffer); // Send request to mcu
