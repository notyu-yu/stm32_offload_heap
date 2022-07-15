#include "../shared_side/shared_config.h"

#include <stdint.h>

#define MALLOC 0
#define FREE 1
#define REALLOC 2
#define SBRK 3

typedef struct {
	uint32_t request;
	size_t req_id;
	size_t size;
	void * ptr;
} mem_request;
