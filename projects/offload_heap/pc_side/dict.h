#include "pc_mm.h"

struct dict_elt_struct {
	uint32_t key;
	blk_elt * ptr;
	struct dict_elt_struct * next;
};
typedef struct dict_elt_struct dict_elt;

struct dict_struct {
	size_t size;
	size_t count;
	dict_elt * table;
};
typedef struct dict_struct dict;

void dict_create(void);
void dict_insert(uint32_t key, blk_elt * ptr);
blk_elt * dict_search(uint32_t key);
void dict_delete(uint32_t key);
void dict_destroy(void);

// Only one dictonary needed
extern dict pointer_dict;
