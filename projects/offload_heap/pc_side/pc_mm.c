#include "dict.h"
#include "memlib.h"
#include "../shared_side/shared_config.h"
#include <assert.h>

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) // Heap request chunk

#define MAX(x,y) ((x) > (y) ? (x) : (y))

// Number of size classes
#define SIZE_CLASSES 12

/* 
 * Class table: first 8 classes are 1-8 words.
 * Later classe sizes are 2*prev_class_size.
 * Each class include blocks greater than its size but smaller than
 * next class size.
 * Starting block in array has size 0.
 */
static blk_elt class_table[SIZE_CLASSES] = {0};

// Pointer to first block
static blk_elt * list_start = NULL;

// Returns the index of size in class table
size_t class_index(uint32_t size) {
	if (SEG_FIT) {
		size_t dwords = size/DSIZE;	
		size_t index = 7;
		if (dwords == 0) {
			// First 8 classes - index is dwords-1
			return 1;
		} else if (dwords <= 8) {
			return dwords-1;
		} else {
			// Later classes - Size doubling each time
			dwords >>= 3; // Divide by 8
			while (dwords && (index < SIZE_CLASSES-1)) {
				dwords >>= 1;
				index ++;
			}
			return index;
		}
	}
	return 0;
}

// Remove a free block from its class list
void free_blk_remove(blk_elt * blk) {
	if (SEG_FIT) {
		blk->prev_free->next_free = blk->next_free;
		blk->next_free->prev_free = blk->prev_free;
		// Might help with debugging
		blk->next_free = NULL;
		blk->prev_free = NULL;
	}
}

// Add free block to the beginning of the appropriate class list
void free_blk_add(blk_elt * blk) {
	if (SEG_FIT) {
		size_t index = class_index(blk->size);
		assert(!blk->alloc);
		// Set prev and next of blk
		blk->next_free = class_table[index].next_free;
		blk->prev_free = &(class_table[index]);
		// Set prev and next of adjacent blocks
		blk->prev_free->next_free = blk;
		blk->next_free->prev_free = blk;
	}
}

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
	// Update blk information
	blk->size += blk->next->size;
	blk->next->next->prev = blk;
	blk->next = blk->next->next;
	// Remove it from free list and dict
	free_blk_remove(temp);
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
	// Old class index
	size_t old_index;
	// Temporary buffer - stores remaining free block
	blk_elt * temp;

	if (prev_alloc && next_alloc) {
		// Neither are free
		return;
	} else if (prev_alloc && !next_alloc) {
		// Coalesce with next block
		temp = blk;
		old_index = class_index(temp->size);
		merge_next(blk);
		if (old_index != class_index(temp->size)) {
			free_blk_remove(temp);
			free_blk_add(temp);
		}
	} else if (!prev_alloc && next_alloc) {
		// Coalesce with previous block
		temp = blk->prev;
		old_index = class_index(temp->size);
		merge_next(blk->prev);
		if (old_index != class_index(temp->size)) {
			free_blk_remove(temp);
			free_blk_add(temp);
		}
	} else {
		// Both blocks are free
		temp = blk->prev;
		old_index = class_index(temp->size);
		merge_next(blk);
		merge_next(blk->prev);
		if (old_index != class_index(temp->size)) {
			free_blk_remove(temp);
			free_blk_add(temp);
		}
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

// Search for first block in size class that fits
static inline blk_elt * seg_fit(size_t asize) {
	size_t index = class_index(asize);
	blk_elt * cur_search;	
	// Loop through class sizes starting at index
	while (index < SIZE_CLASSES) {
		cur_search = class_table[index].next_free;
		// Look through all free blocks in current class
		while (cur_search->size) {
			if (cur_search->size >= asize) {
				return cur_search;
			}
			cur_search = cur_search->next_free;
		}
		index++;
	}
	return 0;
}

// Place fit algorithm here
static blk_elt * find_fit(size_t asize) {
	switch (SEARCH_OPT) {
		case FIRST_FIT:
			return first_fit(asize);
		case BEST_FIT:
			return best_fit(asize);
		case SEG_FIT:
			return seg_fit(asize);
		default:
			// Default to first fit
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
		free_blk_remove(blk);
		// Make new free block
		new_blk = malloc(sizeof(blk_elt));
		new_blk->next = blk->next;
		new_blk->prev = blk;
		new_blk->ptr = blk->ptr + asize;
		new_blk->size = free_size;
		new_blk->alloc = 0;
		// Update surrounding blocks
		blk->next->prev = new_blk;
		blk->next = new_blk;
		// Add to free list and dict
		free_blk_add(new_blk);
		dict_insert(new_blk->ptr, new_blk);
	} else {
		// Allocate entire block
		blk->alloc = 1;
		free_blk_remove(blk);
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
		// Add to free list and dict
		free_blk_add(new_blk);
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
	size_t old_index;
	// Check if there is free block leftover 
	if (combined_size > asize) {
		old_index = class_index(blk->next->size);
		free_size = combined_size-asize;
		free_p = blk->ptr + asize;
		// Shrink next free block
		blk->next->size = free_size;
		dict_delete(blk->next->ptr);
		blk->next->ptr = free_p;
		dict_insert(blk->next->ptr, blk->next);
		// Update current block size
		blk->size = asize;
		// Update block in class table if needed
		if (class_index(blk->next->size) != old_index) {
			free_blk_remove(blk->next);
			free_blk_add(blk->next);
		}
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
	// Add to free list and dict
	free_blk_add(new_blk);
	dict_insert(new_blk->ptr, new_blk);

	// Insert it before starting block
	list_start->prev->next = new_blk;
	list_start->prev = new_blk;

	// Coalesce it
	coalesce(new_blk);
}

// Initialize data structures
int mm_init(uint32_t ptr)
{
	dict_create();

	// Initialze class table
	for (int i=0; i<SIZE_CLASSES; i++) {
		class_table[i].prev = NULL;
		class_table[i].next = NULL;
		class_table[i].next_free = &(class_table[i]);
		class_table[i].prev_free = &(class_table[i]);
		class_table[i].ptr = 0;
		class_table[i].size = 0;
		class_table[i].alloc = 1;
	}

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

// Allocate region of size bytes and return pointer, return NULL if sbrk needed
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

// Free region at ptr
void mm_free(uint32_t ptr)
{
	blk_elt * freed_blk = blk_search(ptr);

	// Free block and coalesce it
	if (freed_blk) {
		freed_blk->alloc = 0;
		free_blk_add(freed_blk);
		coalesce(freed_blk);
	} else {
		puts("Pointer for free not found");
	}
}

// Allocate size byte region with data from ptr, returns NULL if malloc is needed
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

	// Start block information
	blk_elt * cur_blk = list_start;
	blk_elt * prev = list_start;
	printf("The start block: %u alloc, %zu size, %08x ptr\n", cur_blk->alloc, cur_blk->size, cur_blk->ptr); 
	cur_blk = list_start->next;

	// Loop through block list
	for (size_t i=1; cur_blk->size; i++) {
		printf("The %zu th block: %u alloc, %zu size, %08x ptr, next_f %p, prev_f %p\n", i, cur_blk->alloc, cur_blk->size, cur_blk->ptr, cur_blk->next_free, cur_blk->prev_free); 
		// Check linked list consistency - Reinclude assert.h
		assert(cur_blk->prev == prev);
		assert(cur_blk->prev->next == cur_blk);
		if (!dict_search(cur_blk->ptr)) {
			printf("ptr %08x not in hash\n", cur_blk->ptr);
			assert(dict_search(cur_blk->ptr));
		}
		prev = cur_blk;
		assert(cur_blk->prev->ptr + cur_blk->prev->size == cur_blk->ptr);
		cur_blk = cur_blk->next;
	}
}
