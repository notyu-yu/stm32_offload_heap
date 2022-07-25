#include "pc_request.h"
#include "dict.h"
#include <assert.h>

// Send start signal of 1 in every field of mem_request
static void start_signal(void) {
	uint32_t req = 1;
	req_send(&req);
}

int main(int argc, char ** argv) {
	mem_request * req_in = malloc(sizeof(mem_request));
	mem_request * req_out = malloc(sizeof(mem_request));
	uint32_t ptr;

	uart_setup();
	start_signal();

	// Receive sbrk initialization request
	req_receive(req_in);
	assert(req_in->request == SBRK && req_in->ptr);
	// Reset sbrk
	if (VERBOSE) {
		puts("Sbrk reset");
	}
	mem_reset_brk(req_in->ptr);
	mm_init(req_in->ptr);

	while(1) {
		req_receive(req_in);
		if (VERBOSE) {
			printf("Request type: %u\n", req_in->request);
			printf("Request size: %u\n", req_in->size);
			printf("Request ptr: %08x\n", req_in->ptr);
			list_print();
		}
		switch (req_in -> request) {
			case MALLOC:
				if (VERBOSE) {
					printf("Malloc request of size %u received.\n", req_in->size);
				}
				ptr = mm_malloc(req_in->size);
				// Return request
				req_send(&ptr);
				if (VERBOSE) {
					printf("Malloc request finished: %08x\n", ptr);
				}
				break;
			case FREE:
				if (VERBOSE) {
					printf("Free request of pointer 0x%08x received.\n", req_in->ptr);
				}
				mm_free(req_in->ptr);
				break;
			case REALLOC:
				if (VERBOSE) {
					printf("Realloc request of pointer 0x%08x and size %u received.\n", req_in->ptr, req_in->size);
				}
				ptr = mm_realloc(req_in->ptr, req_in->size);
				// Return request
				req_send(&ptr);
				if (VERBOSE) {
					printf("Realloc request finished: %08x\n", ptr);
				}
				break;
			case SBRK:
				if (req_in->size) {
					// Set sbrk
					if (VERBOSE) {
						printf("Calling sbrk with %u.\n", req_in->size);
					}
					mem_sbrk(req_in->size);
				} else {
					if (req_in->ptr) {
						// Reset sbrk
						if (VERBOSE) {
							puts("Sbrk reset");
						}
						mem_reset_brk(req_in->ptr);
						mm_init(req_in->ptr);
					} else {
						// End signal
						mm_init(0);
						dict_destroy();
						puts("Session ended");
						return 0;
					}
				}
				break;
			default:
				printf("Invalid request type: %u.\n", req_in->request);
		}
	}
	return 0;
}
