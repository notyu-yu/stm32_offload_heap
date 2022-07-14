#include "uart_comms.h"

void uart_setup(void);
void req_receive(mem_request * buffer); // Wait and receive request from mcu
void req_send(mem_request * buffer); // Send request to mcu
