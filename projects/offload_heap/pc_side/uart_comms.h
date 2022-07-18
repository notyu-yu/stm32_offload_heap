#include "../shared_side/shared_config.h"

#include <stdint.h>

typedef struct {
	uint32_t request : 2;
	uint32_t size : 30;
	uint32_t ptr;
} mem_request;
