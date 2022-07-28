#include "mcu.h"

extern int mm_init (void); // Initialize malloc library
extern void mm_finish(void); // End communication session

// Standard malloc functions
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);
