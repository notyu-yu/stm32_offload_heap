#include "mcu.h"
#include "mcu_mm.h"
#include "memlib.h"
#include "mcu_timer.h"
#include "mcu_request.h"

void sys_mm_init(void);
void sys_timer_init(void);
void * sys_malloc(size_t size);
void sys_free(void * ptr);
void * sys_realloc(void * ptr, size_t size);
void sys_mm_finish(void);
