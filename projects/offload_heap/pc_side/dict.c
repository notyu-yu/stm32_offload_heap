#include "dict.h"
#include <assert.h>

#define START_COUNT 256

dict pointer_dict = {.count=0, .table=NULL};

// Hash function used
static uint32_t hash_func(uint32_t key) {
	return key % pointer_dict.count;
}

// Initialize dict
void dict_create(void) {
	pointer_dict.count = START_COUNT;
	pointer_dict.table = calloc(pointer_dict.count, sizeof(dict_elt));
}

// Insert entry with MCU pointer key and blk_elt * ptr
void dict_insert(uint32_t key, blk_elt * ptr) {
	uint32_t index = hash_func(key);
	dict_elt * entry;
	dict_elt * insert_point=&(pointer_dict.table[index]);
	// Insert new entry to dict
	if (pointer_dict.table[index].key) {
		// Key used, make new dict entry
		entry = malloc(sizeof(dict_elt));
		entry->key = key;
		entry->ptr = ptr;
		entry->next = NULL;
		// Append to end of chain
		while (insert_point->next) {
			assert(insert_point);
			insert_point = insert_point->next;
		}
		insert_point->next = entry;
	} else {
		// Key unused
		pointer_dict.table[index].key = key;
		pointer_dict.table[index].ptr = ptr;
		pointer_dict.table[index].next = NULL;
	}
}

// Search for MCU pointer key from dict, return NULL if not found
blk_elt * dict_search(uint32_t key) {
	uint32_t index  = hash_func(key);
	dict_elt * cur_entry = &(pointer_dict.table[index]);
	while (cur_entry) {
		if (cur_entry->key == key) {
			return cur_entry->ptr;
		}
		cur_entry = cur_entry->next;
	}
	return NULL;
}

// Delete entry from dict 
void dict_delete(uint32_t key) {
	uint32_t index  = hash_func(key);
	dict_elt * cur_entry = &(pointer_dict.table[index]);
	dict_elt * prev_entry = NULL;
	dict_elt * temp;
	while (cur_entry) {
		if (cur_entry->key == key) {
			if (prev_entry) {
				// Delete current block
				prev_entry->next = cur_entry->next;
				free(cur_entry);
			} else {
				if (cur_entry->next) {
					// Make next element the start
					temp = cur_entry->next;
					cur_entry->key = temp->key;
					cur_entry->ptr = temp->ptr;
					cur_entry->next = temp->next;
					free(temp);
				} else {
					// Reset table entry
					cur_entry->key = 0;
					cur_entry->ptr = NULL;
					cur_entry->next = NULL;
				}
			}
			break;
		}
		prev_entry = cur_entry;
		cur_entry = cur_entry->next;
	}
}

// Destroy dict and free all entries
void dict_destroy(void) {
	dict_elt * cur_entry;
	dict_elt * temp;
	for (size_t i=0; i<pointer_dict.count; i++) {
		cur_entry = pointer_dict.table[i].next;
		// Free parent before each child
		while (cur_entry) {
			temp = cur_entry;
			cur_entry = cur_entry->next;
			free(temp);
		}
	}
	free(pointer_dict.table);
	pointer_dict.count=0;
}
