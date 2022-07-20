#include "memlib.h"

extern int mm_init (uint32_t);
extern uint32_t mm_malloc (size_t size);
extern void mm_free (uint32_t ptr);
extern uint32_t mm_realloc(uint32_t ptr, size_t size);
extern void mm_sbrk(int incr);
extern void mm_heap_reset();
extern void list_print(void);

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
