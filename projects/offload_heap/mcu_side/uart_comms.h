#include "../shared_side/shared_config.h"

#include <stdint.h>

typedef struct {
	uint32_t request : 2;
	size_t size : 30;
	void * ptr;
} mem_request;
