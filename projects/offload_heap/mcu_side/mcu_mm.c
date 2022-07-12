#include "mcu_mm.h"
#include "memlib.h"
#include "mcu_request.h"
#include <assert.h>

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

// Extend heap by words * WSIZE with alignment, return 1 on success 0 on fail
static int extend_heap(size_t words) {
	char * bp;
	size_t size;

	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; // Maintain double word alignment
	if ((long)(bp = mem_sbrk(size)) == -1) {
		return 0;
	} else {
		return 1;
	}
}

int mm_init(void)
{
	mem_req_setup();
	mem_init();
	extend_heap(4096);
    return 0;
}


void *mm_malloc(size_t size)
{
	size_t asize, extendsize;	
	mem_request req;	
	mem_request response;

	// Ignore 0 size
	if (size == 0) {
		return NULL;
	}

	// Send malloc request to server
	req = (mem_request){.request = MALLOC, .req_id=(++cur_id), .size = size, .ptr=NULL};
	req_send(&req);
	req_receive(&response);

	if (response.ptr) {
		return response.ptr;
	} else {
		// Need to extend heap
		// Add overhead and alignment to block size
		if (size <= DSIZE) {
			asize = 2*DSIZE;
		} else {
			asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE); // Add overhead and make rounding floor
		}
		extendsize = MAX(asize, CHUNKSIZE);

		if (extend_heap(extendsize/WSIZE)) {
			// Resend malloc request
			// Send malloc request to server
			req = (mem_request){.request = MALLOC, .req_id=(++cur_id), .size = size, .ptr=NULL};
			req_send(&req);
			req_receive(&response);
			
			assert(response.ptr); // Should have valid response now

			return(response.ptr);
		} else {
			// Not enough memory
			return NULL;
		}
	}
}

void mm_free(void *ptr)
{
	mem_request req = {.request=FREE, .req_id=(++cur_id), .size=0, .ptr=ptr};
	req_send(&req);
}

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
	size_t blk_size;
	size_t asize;
	size_t next_size;
	size_t next_alloc;
	mem_request req;
	mem_request response;

	// Special cases
	if (ptr == NULL) {
		newptr = mm_malloc(size);
		return newptr;
	}
	if (size == 0) {
		mm_free(ptr);
		return ptr;
	}

	// Send realloc request to server
	req = (mem_request){.request = REALLOC, .req_id=(++cur_id), .size = asize, .ptr=ptr};
	req_send(&req);
	req_receive(&response);

	// Extend heap and redo request if needed
	if (response.size) {
		if (extend_heap(response.size)) {
			req = (mem_request){.request = REALLOC, .req_id=(++cur_id), .size = asize, .ptr=ptr};
			req_send(&req);
			req_receive(&response);
		} else {
			// Not enough memory
			return NULL;
		}
	}

	if (response.ptr == oldptr) {
		// Address stays the same
		return response.ptr;
	} else {
		// Need to copy to new location
		memcpy(response.ptr, oldptr, size);
		mm_free(oldptr);
		return response.ptr;
	}
}
