#include "../shared_code/uart_comms.h"

void req_receive(mem_request * buffer); // Wait and receive request from mcu
void req_send(mem_request * burrer); // Send request to mcu
