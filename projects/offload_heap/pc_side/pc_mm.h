#include "memlib.h"

extern int mm_init (uint32_t); // Initialize data structures and peripherals
extern uint32_t mm_malloc (size_t size); // Allocate size byte region and return pointer, return NULL if sbrk needed
extern void mm_free (uint32_t ptr); // Free memory at ptr
extern uint32_t mm_realloc(uint32_t ptr, size_t size); // Allocate size byte region with data at ptr, returns NULL if malloc needed
extern void mm_sbrk(int incr); // Increment brk by incr and update relavent structures
extern void mm_heap_reset(); // Reset brk to heap start
extern void list_print(void); // Print memory block list and check for consistency

// Memory block information struct
struct blk_struct {
	struct blk_struct * next;
	struct blk_struct * prev;
	struct blk_struct * next_free;
	struct blk_struct * prev_free;
	uint32_t ptr;
	size_t size;
	char alloc;
};

typedef struct blk_struct blk_elt;
