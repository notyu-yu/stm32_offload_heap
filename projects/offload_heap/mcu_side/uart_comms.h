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

typedef struct {
	char request;
	size_t req_id;
	size_t size;
	void * ptr;
} mem_request;
