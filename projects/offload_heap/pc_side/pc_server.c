#include "pc_request.h"
#include "pc_mm.h"

int main(int argc, char ** argv) {
	mem_request * req_in = malloc(sizeof(mem_request));
	mem_request * req_out = malloc(sizeof(mem_request));
	void * ptr;

	mm_init();

	while(1) {
		req_receive(req_in);
		switch (req_in -> request) {
			case MALLOC:
				ptr = mm_malloc(req_in->size);
				// Return request
				req_out->request = MALLOC;
				req_out->req_id = req_in->req_id;
				req_out->size = 0;
				req_out->ptr = ptr;
				req_send(req_out);
				break;
			case FREE:
				mm_free(req_in->ptr);
				break;
			case REALLOC:
				ptr = mm_realloc(req_in->ptr, req_in->size);
				// Return request
				req_out->request = REALLOC;
				req_out->req_id = req_in->req_id;
				req_out->size = 0;
				req_out->ptr = ptr;
				req_send(req_out);
				break;
			case SBRK:
				if (req_in->size) {
					// Set sbrk
					mem_set_brk(req_in->ptr);
				} else {
					// Reset sbrk
					mem_reset_brk();
				}
				break;
			default:
				printf("Invalid request type: %d.\n", req_in->request);
		}
	}
	return 0;
}
