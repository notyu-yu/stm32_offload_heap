#include "uart_dma.h"

void mem_req_setup(void); // Setup request communication
void req_send(mem_request * buffer); // Send request
void req_receive(mem_request * buffer); // Wait for request response

extern size_t cur_id;
