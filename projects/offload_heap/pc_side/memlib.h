#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void mem_init(void); // Not implemented
void mem_deinit(void); // Reset brk value to heap start
void mem_reset_brk(uint32_t ptr); // Reset brk value to heap start
uint32_t mem_sbrk(int incr); // Increment brk by incr
void mem_set_brk(uint32_t ptr); // Set brk value to ptr
uint32_t mem_heap_lo(void); // Beginnig of heap
uint32_t mem_heap_hi(void); // End of heap
size_t mem_heapsize(void); // Returns heap size
size_t mem_pagesize(void); // Returns page size - not of the MCU
