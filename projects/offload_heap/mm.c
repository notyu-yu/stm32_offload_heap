/* Malloc implementation with implicit free list and first-fit search.
 * heap-listp stores the block pointer to the prologue block. The list terminates with an epilogue block with size 0.
 * Header and footers are unsigned 32-bit integers, with the first 29 bits storing the size, and the last bit indicating if block is allocated (0 for free 1 for allocated)
 * The bp block pointer in macros refers to start of payload.
 * Size of a block includes size of header and footers.
 * |header|payload|footer|header...
 *        ^
 *       bp
 * |<-------size-------->|
 */

#include "mm.h"
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
#define PACK(size, alloc) ((size) | (alloc)) // Put size and alloc bit in one word

// Read and write to address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// Read size and alloc bit at p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// bp is address of payload
// Get header and footer address of block pointer bp
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Get address of next and previous block of bp
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// Pointer to prologue block
static void * heap_listp;

// Coalesce free blocks with adjacent free blocks, return pointer to coalesced free block
static void * coalesce(void * bp) {
	// Alloc bit of prev and next block
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	// Current block size
	size_t size = GET_SIZE(HDRP(bp));

	if (prev_alloc && next_alloc) {
		// Neither are free
		return bp;
	} else if (prev_alloc && !next_alloc) {
		// Coalesce with next block
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	} else if (!prev_alloc && next_alloc) {
		// Coalesce with previous block
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	} else {
		// Both blocks are free
		size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	return bp;
}

// Extend heap by 1 chunk
static void * extend_heap(size_t words) {
	char * bp;
	size_t size;

	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; // Maintain double word alignment
	if ((long)(bp = mem_sbrk(size)) == -1) {
		return NULL;
	}

	// Free block header and footer
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	// New epilogue header
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));

	// Coalesce previous block
	return coalesce(bp);
}

// Inline to avoid unused warnings
// First fit search for implicit free list, return pointer to payload section, NULL if no fit found
static inline void *first_fit(size_t asize) {
	void * cur_search = heap_listp;
	// Repeat until epilogue block is reached
	while ((GET_SIZE(HDRP(cur_search))!=0)) {
		if ((GET_ALLOC(HDRP(cur_search))==0) && (GET_SIZE(HDRP(cur_search)) >= asize)) {
			return cur_search;
		} else {
			cur_search = NEXT_BLKP(cur_search);
		}
	}
	return NULL;
}

// Best fit search for implicit free list, return pointer to payload, NULL if not found
static inline void *best_fit(size_t asize) {
	void * cur_search = heap_listp;
	size_t cur_size = 0;
	void * best_result = NULL;
	size_t best_size = (size_t)-1; // Default to max size
	// Repeat until epilogue block is reached
	while (GET_SIZE(HDRP(cur_search))!=0) {
		cur_size = GET_SIZE(HDRP(cur_search));
		if ((GET_ALLOC(HDRP(cur_search))==0) && (cur_size >= asize)) {
			// Update new best fit pointer and size
			if (GET_SIZE(HDRP(cur_search)) < best_size) {
				best_size = cur_size;
				best_result = cur_search;
			}
		}
		cur_search = NEXT_BLKP(cur_search);
	}
	return best_result;
}

// Place fit algorithm here
static void * find_fit(size_t asize) {
	return first_fit(asize);
}

// Put an asize allocated block at bp
static void place(void * bp, size_t asize) {
	size_t original_size = GET_SIZE(HDRP(bp));
	size_t free_size;
	void * free_p;
	// Check if there is free block leftover 
	if (original_size >= (asize + DSIZE)) {
		// Split block into allocated and free blocks
		free_size = original_size-asize;
		// Header and footer of free block
		free_p = (char *)(bp) + asize;
		PUT(HDRP(free_p), PACK(free_size, 0));
		PUT(FTRP(free_p), PACK(free_size, 0));
		// Header and footer of allocated block
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
	} else {
		// Allocate entire block
		PUT(HDRP(bp), PACK(original_size, 1));
		PUT(FTRP(bp), PACK(original_size, 1));
	}
}

// Shrink current block
static void shrink_blk(void * bp, size_t asize) {
	size_t original_size = GET_SIZE(HDRP(bp));
	size_t free_size;
	void * free_p;
	// Check if there is free block leftover 
	if (original_size > (asize + DSIZE)) {
		// Split block into allocated and free blocks
		free_size = original_size-asize;
		// Header and footer of free block
		free_p = (char *)(bp) + asize;
		PUT(HDRP(free_p), PACK(free_size, 0));
		PUT(FTRP(bp), PACK(free_size, 0));
		// Header and footer of allocated block
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		coalesce(free_p);
	}
	// No change needed otherwise
}

// Extend current block
static void extend_blk(void * bp, size_t asize) {
	size_t combined_size = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
	size_t free_size;
	void * free_p;
	// Check if there is free block leftover 
	if (combined_size > (asize + DSIZE)) {
		free_size = combined_size-asize;
		// Header and footer of remaining free block
		free_p = (char *)(bp) + asize;
		PUT(HDRP(free_p), PACK(free_size, 0));
		PUT(FTRP(free_p), PACK(free_size, 0));
		// Header and footer of new allocated block
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		coalesce(free_p);
	} else {
		// Use whole block
		PUT(HDRP(bp), PACK(combined_size, 1));
		PUT(FTRP(bp), PACK(combined_size, 1));
	}
}

int mm_init(void)
{
	// Allocate initial heap
	if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
		return -1;
	}
	PUT(heap_listp, 0); //Padding
	// Prologue header and footer
	PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + WSIZE*2, PACK(DSIZE, 1));
	// Epilogue header
	PUT(heap_listp + WSIZE*3, PACK(0, 1));
	heap_listp += 2*WSIZE;

	// Extend heap by a chunk
	if (extend_heap(CHUNKSIZE/WSIZE) == NULL){
		return -1;
	}

    return 0;
}

void *mm_malloc(size_t size)
{
	size_t asize; // Adjusted block size
	size_t extendsize; // Amount to extend heap by
	char * bp;

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
	if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return bp;
	}

	// Need to extend heap
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
		// Not enough memory
		return NULL;
	}
	place(bp, asize);
	return bp;
}

void mm_free(void *ptr)
{
	size_t size = GET_SIZE(HDRP(ptr));
	
	// Set header and footer to free
	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);
}

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
	size_t blk_size;
	size_t asize;
	size_t next_size;
	size_t next_alloc;

	// Special cases
	if (ptr == NULL) {
		newptr = mm_malloc(size);
		return newptr;
	}
	if (size == 0) {
		mm_free(ptr);
		return ptr;
	}

	blk_size = GET_SIZE(HDRP(oldptr));

	// Add overhead and alignment to block size
	if (size <= DSIZE) {
		asize = 2*DSIZE;
	} else {
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE); // Add overhead and make rounding floor
	}

	if (blk_size < asize) {
		next_size = GET_SIZE(HDRP(NEXT_BLKP(oldptr)));
		next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(oldptr)));
		if ((next_alloc == 0) && ((next_size + blk_size) >= asize)) {
			// Can combine with next free block
			extend_blk(oldptr, asize);
			return oldptr;
		} else {
			// Need to malloc new block
			newptr = mm_malloc(size);
			if (newptr == NULL)
			  return NULL;
			memcpy(newptr, oldptr, size);
			mm_free(oldptr);
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
