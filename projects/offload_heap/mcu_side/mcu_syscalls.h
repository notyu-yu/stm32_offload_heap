#ifndef __STRING
	#include <string.h>
#endif

void sys_mm_init(void);
void sys_timer_init(void);
void * sys_malloc(size_t size);
void sys_free(void * ptr);
void * sys_realloc(void * ptr, size_t size);
void sys_mm_finish(void);
size_t sys_get_time(void);
