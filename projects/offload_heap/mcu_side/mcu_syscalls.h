#ifndef __STRING
	#include <string.h>
#endif

void sys_mm_init(void); // Initialize malloc library
void * sys_malloc(size_t size); // Allocate size bytes of memory and returns pointer
void sys_free(void * ptr); // Free memory region at ptr
void * sys_realloc(void * ptr, size_t size); // Allocate size byte region with content of ptr, returns new pointer
void sys_mm_finish(void); // End communication with PC
size_t sys_get_time(void); // Get current time in ms
