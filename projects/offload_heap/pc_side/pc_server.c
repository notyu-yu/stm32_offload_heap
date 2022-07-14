#include "pc_request.h"
#include "pc_mm.h"

int main(int argc, char ** argv) {
	mem_request * req_in = malloc(sizeof(mem_request));
	mem_request * req_out = malloc(sizeof(mem_request));
	uint32_t ptr;

	uart_setup();
	mm_init();

	while(1) {
		req_receive(req_in);
		switch (req_in -> request) {
			case MALLOC:
				printf("Malloc request of size %u received.\n", req_in->size);
				ptr = mm_malloc(req_in->size);
				// Return request
				req_out->request = MALLOC;
				req_out->req_id = req_in->req_id;
				req_out->size = 0;
				req_out->ptr = ptr;
				req_send(req_out);
				puts("Malloc request finished");
				break;
			case FREE:
				printf("Free request of pointer %xu received.\n", req_in->ptr);
				mm_free(req_in->ptr);
				break;
			case REALLOC:
				printf("Realloc request of pointer %xu and size %u received.\n", req_in->ptr, req_in->size);
				ptr = mm_realloc(req_in->ptr, req_in->size);
				// Return request
				req_out->request = REALLOC;
				req_out->req_id = req_in->req_id;
				req_out->size = 0;
				req_out->ptr = ptr;
				req_send(req_out);
				puts("Realloc request finished");
				break;
			case SBRK:
				if (req_in->size) {
					// Set sbrk
					printf("Calling sbrk with %u.\n", req_in->size);
					mem_sbrk(req_in->size);
				} else {
					// Reset sbrk
					puts("Sbrk reset");
					mem_reset_brk(req_in->ptr);
					mm_init();
				}
				break;
			default:
				printf("Invalid request type: %ud.\n", req_in->request);
		}
	}
	return 0;
}
