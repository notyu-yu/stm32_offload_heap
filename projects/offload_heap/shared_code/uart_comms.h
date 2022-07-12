#define BAUDRATE 115200
#define BUFFERSIZE 2048
#define SERIALDEV "/dev/ttyUSB0"
#define USE_DMA 1

#ifndef _STRING_H
	#include <string.h>
#endif

typedef enum {
	MALLOC,
	FREE,
	REALLOC,
	SBRK
} req_type;

typedef struct {
	req_type request;
	size_t req_id;
	size_t size;
	void * ptr;
} mem_request;
