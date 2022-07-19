#include "dict.h"
#include "memlib.h"
#include "../shared_side/shared_config.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) // Heap request chunk

#define MAX(x,y) ((x) > (y) ? (x) : (y))

// Pointer to first block
static blk_elt * list_start = NULL;

// Look through linked list for block pointer, return 0 when not found
static inline blk_elt * linear_blk_search(uint32_t ptr) {
	blk_elt * search_blk = list_start->next;
	while (search_blk->size) {
		if (search_blk->ptr == ptr) {
			return search_blk;
		}
		search_blk = search_blk->next;
	}
	return 0;
}

// Use hash table to get pointer
static inline blk_elt * dict_blk_search(uint32_t ptr) {
	return dict_search(ptr);
}

// Search algorithm for pointer lookup
static blk_elt * blk_search(uint32_t ptr) {
	if (DICT_SEARCH) {
		return dict_blk_search(ptr);
	} else {
		return linear_blk_search(ptr);
	}
}

// Merge blk with its next block, free the extra block
static blk_elt * merge_next(blk_elt * blk) {
	blk_elt * temp = blk->next;
	blk->size += blk->next->size;
	blk->next->next->prev = blk;
	blk->next = blk->next->next;
	dict_delete(temp->ptr);
	free(temp);
	return blk;
}

// Coalesce free blocks with adjacent free blocks, return pointer to coalesced free block
static void coalesce(blk_elt * blk) {
	// Alloc bit of prev and next block
	size_t prev_alloc = blk->prev->alloc;
	size_t next_alloc = blk->next->alloc;
	// Current block size
	size_t size = blk->size;

	if (prev_alloc && next_alloc) {
		// Neither are free
		return;
	} else if (prev_alloc && !next_alloc) {
		// Coalesce with next block
		merge_next(blk);
	} else if (!prev_alloc && next_alloc) {
		// Coalesce with previous block
		merge_next(blk->prev);
	} else {
		// Both blocks are free
		merge_next(blk);
		merge_next(blk->prev);
	}
}

// Inline to avoid unused warnings
// First fit search for implicit free list, return pointer to payload section, NULL if no fit found
static inline blk_elt * first_fit(size_t asize) {
	blk_elt * cur_search = list_start->next;
	// Repeat until epilogue block is reached
	while (cur_search->size) {
		if ((cur_search->alloc == 0) && (cur_search->size >= asize)) {
			return cur_search;
		} else {
			cur_search = cur_search->next;
		}
	}
	return NULL;
}

// Best fit search for implicit free list, return pointer to payload, NULL if not found
static inline blk_elt * best_fit(size_t asize) {
	blk_elt * cur_search = list_start->next;
	size_t cur_size = 0;
	blk_elt * best_result = NULL;
	size_t best_size = (size_t)-1; // Default to max size
	// Repeat until epilogue block is reached
	while (cur_search->size) {
		cur_size = cur_search->size;
		if ((cur_search->alloc == 0) && (cur_size >= asize)) {
			// Update new best fit pointer and size
			if (cur_size < best_size) {
				best_size = cur_size;
				best_result = cur_search;
			}
		}
		cur_search = cur_search->next;
	}
	return best_result;
}

// Place fit algorithm here
static blk_elt * find_fit(size_t asize) {
	if (BEST_FIT) {
		return best_fit(asize);
	} else {
		return first_fit(asize);
	}
}

// Put an asize allocated block at free block blk
static void place(blk_elt * blk, size_t asize) {
	size_t original_size = blk->size;
	size_t free_size;
	blk_elt * new_blk;
	// Check if there is free block leftover 
	if (original_size > asize) {
		// Split block into allocated and free blocks
		free_size = original_size-asize;
		// Allocate original block
		blk->alloc = 1;
		blk->size = asize;
		// Make new free block
		new_blk = malloc(sizeof(blk_elt));
		new_blk->next = blk->next;
		new_blk->prev = blk;
		new_blk->ptr = blk->ptr + asize;
		new_blk->size = free_size;
		new_blk->alloc = 0;
		blk->next->prev = new_blk;
		blk->next = new_blk;
		dict_insert(new_blk->ptr, new_blk);
	} else {
		// Allocate entire block
		blk->alloc = 1;
	}
}

// Shrink current block
static void shrink_blk(blk_elt * blk, size_t asize) {
	size_t original_size = blk->size;
	size_t free_size;
	uint32_t free_p;
	blk_elt * new_blk;
	// Check if there is free block leftover 
	if (original_size > asize) {
		// Split block into allocated and free blocks
		free_size = original_size-asize;
		// Make new free block
		free_p = blk->ptr + asize;
		new_blk = malloc(sizeof(blk_elt));
		new_blk->next = blk->next;
		new_blk->prev = blk;
		new_blk->ptr = free_p;
		new_blk->size = free_size;
		new_blk->alloc = 0;
		dict_insert(new_blk->ptr, new_blk);
		// Update adjacent blocks
		blk->next->prev = new_blk;
		blk->next = new_blk;
		// Update original block
		blk->size = asize;

		coalesce(new_blk);
	}
	// No change needed otherwise
}

// Extend current block
static void extend_blk(blk_elt * blk, size_t asize) {
	size_t combined_size = blk->size + blk->next->size;
	size_t free_size;
	uint32_t free_p;
	// Check if there is free block leftover 
	if (combined_size > asize) {
		free_size = combined_size-asize;
		free_p = blk->ptr + asize;
		// Shrink next free block
		blk->next->size = free_size;
		dict_delete(blk->next->ptr);
		blk->next->ptr = free_p;
		dict_insert(blk->next->ptr, blk->next);
		// Update current block size
		blk->size = asize;
	} else {
		merge_next(blk);
	}
}

// Clear heap info list
void mm_heap_reset(void) {
	blk_elt * cur_blk = list_start->next;
	blk_elt * temp_blk;
	while (cur_blk->size) {
		temp_blk = cur_blk;
		cur_blk = cur_blk->next;
		free(temp_blk);
	}
	list_start->next = list_start->prev = list_start;
}

// Insert free block to linked list
void mm_sbrk(int incr) {
	blk_elt * new_blk;
	// Make new block
	new_blk = malloc(sizeof(blk_elt));
	new_blk->next = list_start;
	new_blk->prev = list_start->prev;
	new_blk->ptr = list_start->prev->ptr + list_start->prev->size;
	new_blk->size = incr;
	new_blk->alloc = 0;
	dict_insert(new_blk->ptr, new_blk);

	// Insert it before starting block
	list_start->prev->next = new_blk;
	list_start->prev = new_blk;

	// Coalesce it
	coalesce(new_blk);
}

int mm_init(uint32_t ptr)
{
	dict_create();

	// Allocate starter block
	if (list_start) {
		mm_heap_reset();
	} else {
		list_start = malloc(sizeof(blk_elt));
		list_start->next = list_start->prev = list_start;
		list_start->ptr = ptr;
		list_start->size = 0;
		list_start->alloc = 1;
	}

    return 0;
}

uint32_t mm_malloc(size_t size)
{
	size_t asize; // Adjusted block size
	size_t extendsize; // Amount to extend heap by
	blk_elt * blk;

	// Ignore 0 size
	if (size == 0) {
		return 0;
	}

	// Add overhead and alignment to block size
	if (size <= DSIZE) {
		asize = DSIZE;
	} else {
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE); // Add overhead and make rounding floor
	}

	// Search free block for fit
	if ((blk = find_fit(asize)) != NULL) {
		place(blk, asize);
		return blk->ptr;
	}

	// Need to extend heap: return Null and let MCU send sbrk request
	
	return 0;
}

void mm_free(uint32_t ptr)
{
	blk_elt * freed_blk = blk_search(ptr);

	// Look through linked list for free block
	if (freed_blk) {
		freed_blk->alloc = 0;
		coalesce(freed_blk);
	} else {
		puts("Pointer for free not found");
	}
}

uint32_t mm_realloc(uint32_t ptr, size_t size)
{
    uint32_t oldptr = ptr;
    uint32_t newptr;
	size_t blk_size;
	size_t asize;
	size_t next_size;
	size_t next_alloc;

	// Search for block in linked list
	blk_elt * search_blk = blk_search(ptr);

	// Ptr not found in list
	if (search_blk->size == 0) {
		puts("Realloc ptr not found");
		return 0;
	}

	blk_size = search_blk->size;

	// Add alignment to block size
	if (size < DSIZE) {
		asize = DSIZE;
	} else {
		asize = DSIZE * ((size + (DSIZE-1))/DSIZE); // Make rounding floor
	}

	if (blk_size < asize) {
		next_size = search_blk->next->size;
		next_alloc = search_blk->next->alloc;
		if ((next_alloc == 0) && ((next_size + blk_size) >= asize)) {
			// Can combine with next free block
			extend_blk(search_blk, asize);
			return oldptr;
		} else {
			// Need to malloc new block, return 0
			return 0;
		}
	} else if (blk_size > asize) {
		// Need to shrink block
		shrink_blk(search_blk, asize);
		return oldptr;
	} else {
		// Do nothing
		return oldptr;
	}
}

// Print all block list elements
void list_print(void) {
	if (!list_start) {
		puts("list not initialized");
		return;
	}
	blk_elt * cur_blk = list_start;
	blk_elt * prev = list_start;
	printf("The start block: %u alloc, %zu size, %08x ptr\n", cur_blk->alloc, cur_blk->size, cur_blk->ptr); 
	cur_blk = list_start->next;
	for (size_t i=1; cur_blk->size; i++) {
		printf("The %zu th block: %u alloc, %zu size, %08x ptr\n", i, cur_blk->alloc, cur_blk->size, cur_blk->ptr); 
		prev = cur_blk;
		cur_blk = cur_blk->next;
		// Check linked list consistency - Reinclude assert.h
		/*
		assert(cur_blk->prev == prev);
		assert(cur_blk->prev->next == cur_blk);
		if (!dict_search(cur_blk->ptr)) {
			printf("ptr %08x not in hash\n", cur_blk->ptr);
			assert(dict_search(cur_blk->ptr));
		}
		*/
	}
}
