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

void dict_create(void); // Initialize the dict
void dict_insert(uint32_t key, blk_elt * ptr); // Insert entry with MCU pointer key and blk_elt pointer ptr
blk_elt * dict_search(uint32_t key); // Search for blk_elt pointer given MCU pointer key
void dict_delete(uint32_t key); // Delete entry with key from dict
void dict_destroy(void); // Free memory used by dict

// Only one dictonary needed
extern dict pointer_dict;
