#include "pc_mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "summer_research",
    /* First member's full name */
    "Yuhang Cui",
    /* First member's email address */
    "yuhang.cui@yale.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

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

// Merge blk with its next block, free the extra block
static blk_elt * merge_next(blk_elt * blk) {
	blk->size += blk->next->size;
	blk->next->prev = blk;
	blk->next = blk->next->next;
	free(blk->next);
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
		merge_next(blk->prev);
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
	return first_fit(asize);
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
		new_blk->ptr = ((char *)blk->ptr) + asize;
		new_blk->size = free_size;
		new_blk->alloc = 0;
		blk->next->prev = new_blk;
		blk->next = new_blk;
	} else {
		// Allocate entire block
		blk->alloc = 1;
	}
}

// Shrink current block
static void shrink_blk(blk_elt * blk, size_t asize) {
	size_t original_size = blk->size;
	size_t free_size;
	void * free_p;
	blk_elt * new_blk;
	// Check if there is free block leftover 
	if (original_size > asize) {
		// Split block into allocated and free blocks
		free_size = original_size-asize;
		// Make new free block
		free_p = (char *)(blk->ptr) + asize;
		new_blk = malloc(sizeof(blk_elt));
		new_blk->next = blk->next;
		new_blk->prev = blk;
		new_blk->ptr = free_p;
		new_blk->size = free_size;
		new_blk->alloc = 0;
		blk->next->prev = new_blk;
		blk->next = new_blk;
		// Update original block
		blk->size = asize;

		coalesce(free_p);
	}
	// No change needed otherwise
}

// Extend current block
static void extend_blk(blk_elt * blk, size_t asize) {
	size_t combined_size = blk->size + blk->next->size;
	size_t free_size;
	void * free_p;
	// Check if there is free block leftover 
	if (combined_size > asize) {
		free_size = combined_size-asize;
		free_p = (char *)(blk->ptr) + asize;
		// Shrink next free block
		blk->next->size = free_size;
		// Update current block size
		blk->size = asize;

		coalesce(free_p);
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
	new_blk->ptr = ((char *)list_start->prev) + list_start->prev->size;
	new_blk->size = incr;
	new_blk->alloc = 0;

	// Insert it before starting block
	list_start->prev->next = new_blk;
	list_start->prev = new_blk;

	// Coalesce it
	coalesce(new_blk);
}

int mm_init(void)
{
	// Allocate starter block
	if (list_start) {
		mm_heap_reset();
	} else {
		list_start = malloc(sizeof(blk_elt));
		list_start->next = list_start->prev = list_start;
		list_start->ptr = NULL;
		list_start->size = 0;
		list_start->alloc = 1;
	}

	// TODO: MCU side should ask for 1 chunk starting space

    return 0;
}

void * mm_malloc(size_t size)
{
	size_t asize; // Adjusted block size
	size_t extendsize; // Amount to extend heap by
	blk_elt * blk;

	// Ignore 0 size
	if (size == 0) {
		return NULL;
	}

	// Add overhead and alignment to block size
	if (size <= DSIZE) {
		asize = 2*DSIZE;
	} else {
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE); // Add overhead and make rounding floor
	}

	// Search free block for fit
	if ((blk = find_fit(asize)) != NULL) {
		place(blk, asize);
		return blk;
	}

	// Need to extend heap: return Null and let MCU send sbrk request
	
	return blk;
}

void mm_free(void *ptr)
{
	blk_elt * current_blk = list_start->next;

	// Look through linked list for free block
	while (current_blk->size) {
		if (current_blk->ptr == ptr) {
			current_blk->alloc = 0;
			coalesce(current_blk);
			return;
		}
		current_blk = current_blk->next;
	}
}

void *mm_realloc(void * ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
	size_t blk_size;
	size_t asize;
	size_t next_size;
	size_t next_alloc;

	blk_elt * search_blk = list_start->next;

	// Special cases
	if (ptr == NULL) {
		newptr = mm_malloc(size);
		return newptr;
	}
	if (size == 0) {
		mm_free(ptr);
		return ptr;
	}

	// Search for block in linked list
	while (search_blk->size) {
		if (search_blk->ptr == ptr) {
			break;
		}
	}

	// Ptr not round in list
	if (search_blk->size == 0) {
		return NULL;
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
			extend_blk(oldptr, asize);
			return oldptr;
		} else {
			// Need to malloc new block
			newptr = mm_malloc(size);
			if (newptr == NULL)
			  return NULL;
			return newptr;
		}
	} else if (blk_size > asize) {
		// Need to shrink block
		shrink_blk(oldptr, asize);
		return oldptr;
	} else {
		// Do nothing
		return ptr;
	}
}
