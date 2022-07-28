#include "dict.h"
#include <assert.h>

#define START_COUNT 128

// Dictionary used
dict pointer_dict = {.count=0, .size=0, .table=NULL};

// Hash function, returns hash key mod table size
static uint32_t hash_func(uint32_t key) {
	// FNV1a hash function
	uint64_t prime = ((1ULL<<40)+(1<<8)+0xb3);
	uint64_t basis = (14695981039346656037ULL);

	for (size_t i=0; i<sizeof(key); i++) {
		basis ^= ((char *)&key)[i];
		basis *= prime;
	}

	return key % pointer_dict.size;
}

// Initialize dict
void dict_create(void) {
	pointer_dict.size = START_COUNT;
	pointer_dict.count = 0;
	pointer_dict.table = calloc(pointer_dict.size, sizeof(dict_elt));
}

// Insert entry with MCU pointer key and blk_elt * ptr
static void internal_dict_insert(dict_elt * table, uint32_t key, blk_elt * ptr) {
	uint32_t index = hash_func(key);
	dict_elt * entry;
	dict_elt * insert_point=&(table[index]);
	// Insert new entry to dict
	if (table[index].key) {
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
		table[index].key = key;
		table[index].ptr = ptr;
		table[index].next = NULL;
	}
}

// Double dict array size if count is too large
static void dict_double(void) {
	dict_elt * cur_entry;
	dict_elt * temp;
	dict_elt * old_table = pointer_dict.table;
	dict_elt * new_table = calloc(pointer_dict.size*2, sizeof(dict_elt));
	size_t old_size = pointer_dict.size;

	pointer_dict.size*=2;

	// Go through dict table
	for (size_t i=0; i<old_size; i++) {
		// Insert non-empty table entries to new dict
		if (pointer_dict.table[i].key) {
			internal_dict_insert(new_table, pointer_dict.table[i].key, pointer_dict.table[i].ptr);
		}
		cur_entry = pointer_dict.table[i].next;
		// Insert and free each parent before child
		while (cur_entry) {
			internal_dict_insert(new_table, cur_entry->key, cur_entry->ptr);
			temp = cur_entry;
			cur_entry = cur_entry->next;
			free(temp);
		}
	}
	pointer_dict.table = new_table;
	free(old_table);
}

// Insert entry with MCU pointer key and blk_elt * ptr
void dict_insert(uint32_t key, blk_elt * ptr) {
	internal_dict_insert(pointer_dict.table, key, ptr);
	pointer_dict.count++;
	if (pointer_dict.count > pointer_dict.size) {
		dict_double();
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
			pointer_dict.count--;
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
	for (size_t i=0; i<pointer_dict.size; i++) {
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
	pointer_dict.size=0;
}
