/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"
#include "mcu_request.h"
#include "mcu_mpu.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

// Marks end of bss section
extern char * __malloc_sbrk_start;

/* private variables */
static char *mem_start_brk;  /* points to first byte of heap */
static char *mem_brk;        /* points to last byte of heap */
//static char *mem_max_addr;   /* largest legal heap address */ 

/* 
 * mem_init - initialize the memory system model
 */
void mem_init(void)
{
	mem_request req;
	mem_start_brk = (char *)ALIGN((size_t)(&__malloc_sbrk_start));
    mem_brk = mem_start_brk;

	// Sbrk request: size=0 for reset, ptr set to heap start
	req = (mem_request){.request=SBRK, .size=0, .ptr=mem_brk};
	req_send(&req);
	proc_update();
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
	mem_reset_brk();
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
void mem_reset_brk()
{
	mem_request req;
    mem_brk = mem_start_brk;

	// Sbrk request: size=0 for reset, ptr set to heap start
	req = (mem_request){.request = SBRK, .size=0, .ptr=mem_brk};
	req_send(&req);
	proc_update();
}

/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
void *mem_sbrk(unsigned int incr) 
{
    char *old_brk = mem_brk;
	register size_t * stack_top asm("sp");
	mem_request req;

	// Special incr cases
	if (incr < 0) {
		char output_str[] = "Negative incr not supported";
		var_print(output_str);
		return (void *)-1;
	} else if (incr == 0) {
		return mem_brk;
	}

	// Check if there is enough memory
    if (((mem_brk + incr) > (char *)(stack_top))) {
		char output_str[] = "ERROR: mem_sbrk failed. Ran out of memory...\n";
		var_print(output_str);
		return (void *)-1;
    }
    mem_brk += incr;

	// Sbrk request: size=0 for reset, size=1 for sbrk move
	req = (mem_request){.request = SBRK, .size=incr, .ptr=0};
	req_send(&req);
	proc_update();
    return (void *)old_brk;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
