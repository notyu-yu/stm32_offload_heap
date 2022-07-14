#define BAUDRATE 115200
#define BUFFERSIZE 2048
#define SERIALDEV "/dev/ttyUSB0"
#define USE_DMA 1

#ifndef _STRING_H
	#include <string.h>
#endif

#define MALLOC 0
#define FREE 1
#define REALLOC 2
#define SBRK 3

#include <stdint.h>

typedef struct {
	uint32_t request;
	uint32_t req_id;
	uint32_t size;
	uint32_t ptr;
} mem_request;
