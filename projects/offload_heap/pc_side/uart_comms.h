#include "../shared_side/shared_config.h"

#include <stdint.h>

typedef struct {
	uint32_t request;
	uint32_t req_id;
	uint32_t size;
	uint32_t ptr;
} mem_request;
