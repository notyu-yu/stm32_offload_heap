#include "../shared_side/shared_config.h"

#include <stdint.h>

typedef struct {
	uint32_t request;
	size_t req_id;
	size_t size;
	void * ptr;
} mem_request;
