#include "mcu.h"

void mem_init(void); // Initialize sbrk functions
void mem_deinit(void); // Reset sbrk to heap start
void *mem_sbrk(unsigned int incr); // Increment sbrk by incr bytes, returns old sbrk location
void mem_reset_brk(void); // Reset sbrk to heap start
void *mem_heap_lo(void); // Returns heap start
void *mem_heap_hi(void); // Returns heap end
size_t mem_heapsize(void); // Return heap size
size_t mem_pagesize(void); // Return page size
