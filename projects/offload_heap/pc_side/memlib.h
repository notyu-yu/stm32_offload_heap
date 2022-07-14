#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void mem_init(void);               
void mem_deinit(void);
void mem_reset_brk(uint32_t ptr); 
uint32_t mem_sbrk(int incr);
void mem_set_brk(uint32_t ptr);
uint32_t mem_heap_lo(void);
uint32_t mem_heap_hi(void);
size_t mem_heapsize(void);
size_t mem_pagesize(void);
