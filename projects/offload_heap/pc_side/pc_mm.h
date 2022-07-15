#include "memlib.h"

extern int mm_init (uint32_t);
extern uint32_t mm_malloc (size_t size);
extern void mm_free (uint32_t ptr);
extern uint32_t mm_realloc(uint32_t ptr, size_t size);
extern void mm_sbrk(int incr);
extern void mm_heap_reset();
extern void list_print(void);


/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char *teamname; /* ID1+ID2 or ID1 */
    char *name1;    /* full name of first member */
    char *id1;      /* login ID of first member */
    char *name2;    /* full name of second member (if any) */
    char *id2;      /* login ID of second member */
} team_t;

extern team_t team;

struct blk_struct {
	struct blk_struct * next;
	struct blk_struct * prev;
	uint32_t ptr;
	size_t size;
	char alloc;
};

typedef struct blk_struct blk_elt;
