/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "pc_request.h"
#include "pc_mm.h"

#define ALIGNMENT 4
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* private variables */
static uint32_t mem_start_brk;  /* points to first byte of heap */
static uint32_t mem_brk;        /* points to last byte of heap */
//static char *mem_max_addr;   /* largest legal heap address */ 

/* 
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
	mem_reset_brk(mem_start_brk);
}

/*
 * mem_reset_brk - reset starting brk to ptr
 */
void mem_reset_brk(uint32_t ptr)
{
    mem_brk = mem_start_brk = ptr;
}

/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
// Only changes local variable, all sbrk requests need to be initiated by the MCU
uint32_t mem_sbrk(int incr) 
{
    uint32_t old_brk = mem_brk;
    mem_brk += incr;

	mm_sbrk(incr);

    return old_brk;
}

// Set sbrk to a specific pointer
void mem_set_brk(uint32_t ptr) {
	mem_brk = ptr;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
uint32_t mem_heap_lo()
{
    return mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
uint32_t mem_heap_hi()
{
    return (mem_brk - 1);
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
