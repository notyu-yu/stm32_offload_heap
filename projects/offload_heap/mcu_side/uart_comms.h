#include "../shared_side/shared_config.h"

#include <stdint.h>

// MCU to PC request struct
typedef struct {
	uint32_t request : 2; // Only 4 types
	size_t size : 30; // Support up to 1GB request
	void * ptr;
} mem_request;
