#define BAUDRATE 115200
#define BUFFERSIZE 2048
#define SERIALDEV "/dev/ttyUSB0"
#define USE_DMA 0

#ifndef _STRING_H
	#include <string.h>
#endif

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
