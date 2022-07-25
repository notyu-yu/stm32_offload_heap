/*
 * mdriver.c - CS:APP Malloc Lab Driver
 * 
 * Uses a collection of trace files to tests a malloc/free/realloc
 * implementation in mm.c.
 *
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <time.h>
#include <stdarg.h>

#include "config.h"
#include "teststring.h"
#include "mcu_syscalls.h"

/**********************
 * Constants and macros
 **********************/

/* Misc */
#define MAXLINE     1024 /* max string size */
#define HDRLINES       4 /* number of header lines in a trace file */
#define LINENUM(i) (i+5) /* cnvt trace request nums to linenums (origin 1) */
#define KB 1024

/* Returns true if p is ALIGNMENT-byte aligned */
#define IS_ALIGNED(p)  ((((unsigned int)(p)) % ALIGNMENT) == 0)

/****************************** 
 * The key compound data types 
 *****************************/

/* Records the extent of each block's payload */
typedef struct range_t {
    char *lo;              /* low payload address */
    char *hi;              /* high payload address */
    struct range_t *next;  /* next list element */
} range_t;

/* Characterizes a single trace operation (allocator request) */
typedef struct {
    enum {ALLOC, FREE, REALLOC} type; /* type of request */
    int index;                        /* index for free() to use later */
    int size;                         /* byte size of alloc/realloc request */
} traceop_t;

/* Holds the information for one trace file*/
typedef struct {
    int sugg_heapsize;   /* suggested heap size (unused) */
    int num_ids;         /* number of alloc/realloc ids */
    int num_ops;         /* number of distinct requests */
    int weight;          /* weight for this trace (unused) */
    traceop_t *ops;      /* array of requests */
    char **blocks;       /* array of ptrs returned by malloc/realloc... */
    size_t *block_sizes; /* ... and a corresponding array of payload sizes */
} trace_t;

/* 
 * Holds the params to the xxx_speed functions, which are timed by fcyc. 
 * This struct is necessary because fcyc accepts only a pointer array
 * as input.
 */
typedef struct {
    trace_t *trace;  
    range_t *ranges;
} speed_t;

/* Summarizes the important stats for some malloc function on some trace */
typedef struct {
    /* defined for both libc malloc and student malloc package (mm.c) */
    double ops;      /* number of ops (malloc/free/realloc) in the trace */
    int valid;       /* was the trace processed correctly by the allocator? */
    double secs;     /* number of secs needed to run the trace */

    /* defined only for the student malloc package */
    double util;     /* space utilization for this trace (always 0 for libc) */

    /* Note: secs and util are only defined if valid is true */
} stats_t; 

/********************
 * Global variables
 *******************/
int verbose = 0;        /* global flag for verbose output */
static int errors = 0;  /* number of errs found when running student malloc */

/* The filenames of the default tracefiles */
/*
static char *default_tracefiles[] = {  
    DEFAULT_TRACEFILES, NULL
};
*/

/********************* 
 * Function prototypes 
 *********************/

/* these functions manipulate range lists */
static int add_range(range_t **ranges, char *lo, int size, 
		     int tracenum, int opnum);
static void remove_range(range_t **ranges, char *lo);
static void clear_ranges(range_t **ranges);

/* These functions read, allocate, and free storage for traces */
static trace_t *read_trace(void);
static void free_trace(trace_t *trace);

/* Routines for evaluating correctnes, space utilization, and speed 
   of the student's malloc package in mm.c */
static int eval_mm_valid(trace_t *trace, int tracenum, range_t **ranges);
static double eval_mm_util(trace_t *trace, int tracenum, range_t **ranges);
static void eval_mm_speed(void *ptr);

/* Various helper routines */
static void printresults(int n, stats_t *stats);
static void unix_error(char *msg);
static void malloc_error(int tracenum, int opnum, char *msg);
static void app_error(char *msg);

// Test file string
static char tracestr[] = TESTSTRING;

// Test for stack overflow
static void stack_test(void) {
	char buffer_array[KB/8] = {0};
	stack_test();
}

// Test for stack overflow
static void heap_test(void) {
	while(1) {
		if(!sys_malloc(10*KB)){
			sys_mm_finish();
			loop();
		}
	}
}

// Memory used by test script
static int test_mem_use = 0;

// Message buffer
char msg[KB] = {0};

/**************
 * Main routine
 **************/
int main(void)
{
    int i=0;
    int num_tracefiles = 0;    /* the number of traces in that array */
    trace_t *trace = NULL;     /* stores a single trace file in memory */
    range_t *ranges = NULL;    /* keeps track of block extents for one trace */
    stats_t mm_stats;  /* mm (i.e. student) stats for each trace */
    //speed_t speed_params;      /* input parameters to the xx_speed routines */ 
	
	// Test start and end time
	size_t start_time, end_time;

    int autograder = 0;  /* If set, emit summary info for autograder (-g) */

    /* temporaries used to compute the performance index */
    double util, avg_mm_util, p1, p2, avg_mm_throughput, ops, secs;
    int numcorrect;

	int p1_int;
	int p2_int;
	int perfindex_int;

    /* Initialize the simulated memory system in memlib.c */
	sys_mm_init();
	start_time = sys_get_time();

    /* Evaluate student's mm malloc package using the K-best scheme */
	trace = read_trace();
	mm_stats.ops = trace->num_ops;
	if (verbose > 1) {
	    sprintf(msg, "Checking sys_malloc for correctness, ");
		var_print(msg);
	}
	mm_stats.valid = eval_mm_valid(trace, i, &ranges);
	if (mm_stats.valid) {
	    if (verbose > 1) {
			sprintf(msg, "efficiency, ");
			var_print(msg);
		}
	    mm_stats.util = eval_mm_util(trace, i, &ranges);
	    if (verbose > 1) {
			sprintf(msg, "and performance.\n");
			var_print(msg);
		}
	    //mm_stats.secs = fsecs(eval_mm_speed, &speed_params);
		end_time = sys_get_time();
		mm_stats.secs = (end_time-start_time)/1000.0f;
	}
	free_trace(trace);

    /* Display the mm results in a compact table */
    if (verbose) {
		sprintf(msg, "\nResults for mm malloc:\n");
		var_print(msg);
		printresults(num_tracefiles, &mm_stats);
		sprintf(msg, "\n");
		var_print(msg);
    }

    /* 
     * Accumulate the aggregate statistics for the student's mm package 
     */
    secs = 0;
    ops = 0;
    util = 0;
    numcorrect = 0;
	secs = mm_stats.secs;
	ops = mm_stats.ops;
	util = mm_stats.util;
	if (mm_stats.valid)
	    numcorrect++;
    avg_mm_util = util;

    /* 
     * Compute and print the performance index 
     */
    if (errors == 0) {
	avg_mm_throughput = ops/secs;

	p1 = avg_mm_util;
	p2 = avg_mm_throughput;

	p1_int = p1*100;
	p2_int = (int)p2;

	sprintf(msg, "Utilization: %d%%. Throughput: %d\n",
	       p1_int, 
	       p2_int);
	
	var_print(msg);
    }
    else { /* There were errors */
	sprintf(msg, "Terminated with %d errors\n", errors);
	var_print(msg);
    }

    if (autograder) {
	sprintf(msg, "correct:%d\n", numcorrect);
	var_print(msg);
	sprintf(msg, "perfidx:%d\n", perfindex_int);
	var_print(msg);
    }

	sys_mm_finish();

	loop();
}


/*****************************************************************
 * The following routines manipulate the range list, which keeps 
 * track of the extent of every allocated block payload. We use the 
 * range list to detect any overlapping allocated blocks.
 ****************************************************************/

/*
 * add_range - As directed by request opnum in trace tracenum,
 *     we've just called the student's sys_malloc to allocate a block of 
 *     size bytes at addr lo. After checking the block for correctness,
 *     we create a range struct for this block and add it to the range list. 
 */
static int add_range(range_t **ranges, char *lo, int size, 
		     int tracenum, int opnum)
{
    char *hi = lo + size - 1;
    range_t *p;

    assert(size > 0);

    /* Payload addresses must be ALIGNMENT-byte aligned */
    if (!IS_ALIGNED(lo)) {
	sprintf(msg, "Payload address (%p) not aligned to %d bytes", 
		lo, ALIGNMENT);
        malloc_error(tracenum, opnum, msg);
        return 0;
    }

    /* The payload must lie within the extent of the heap */
    if ((lo < (char *)mem_heap_lo()) || (lo > (char *)mem_heap_hi()) || 
	(hi < (char *)mem_heap_lo()) || (hi > (char *)mem_heap_hi())) {
	sprintf(msg, "Payload (%p:%p) lies outside heap (%p:%p)",
		lo, hi, mem_heap_lo(), mem_heap_hi());
	malloc_error(tracenum, opnum, msg);
        return 0;
    }

    /* The payload must not overlap any other payloads */
    for (p = *ranges;  p != NULL;  p = p->next) {
        if ((lo >= p->lo && lo <= p-> hi) ||
            (hi >= p->lo && hi <= p->hi)) {
	    sprintf(msg, "Payload (%p:%p) overlaps another payload (%p:%p)\n",
		    lo, hi, p->lo, p->hi);
	    malloc_error(tracenum, opnum, msg);
	    return 0;
        }
    }

    /* 
     * Everything looks OK, so remember the extent of this block 
     * by creating a range struct and adding it the range list.
     */
    if ((p = (range_t *)sys_malloc(sizeof(range_t))) == NULL)
		unix_error("malloc error in add_range");
	test_mem_use += sizeof(range_t);
    p->next = *ranges;
    p->lo = lo;
    p->hi = hi;
    *ranges = p;
    return 1;
}

/* 
 * remove_range - Free the range record of block whose payload starts at lo 
 */
static void remove_range(range_t **ranges, char *lo)
{
    range_t *p;
    range_t **prevpp = ranges;

    for (p = *ranges;  p != NULL; p = p->next) {
        if (p->lo == lo) {
	    *prevpp = p->next;
            sys_free(p);
            break;
        }
        prevpp = &(p->next);
    }
}

/*
 * clear_ranges - free all of the range records for a trace 
 */
static void clear_ranges(range_t **ranges)
{
    range_t *p;
    range_t *pnext;

    for (p = *ranges;  p != NULL;  p = pnext) {
        pnext = p->next;
        sys_free(p);
    }
    *ranges = NULL;
}

/**********************************************
 * The following routines manipulate tracefiles
 *********************************************/

/*
 * read_trace - read a trace file and store it in memory
 */
static trace_t *read_trace()
{
    trace_t *trace;
    char type[MAXLINE];
    char path[MAXLINE];
    unsigned index, size;
    unsigned max_index = 0;
    unsigned op_index;
	char * scanptr = tracestr;
	int bytes_scanned = 0;

    /* Allocate the trace record */
    if ((trace = (trace_t *) sys_malloc(sizeof(trace_t))) == NULL)
		unix_error("malloc 1 failed in read_trance");
	test_mem_use += sizeof(trace_t);
	
    /* Read the trace file header */
    //if ((tracefile = fmemopen(tracestr, strlen(tracestr), "r")) == NULL) {
	//sprintf(msg, "Could not open %s in read_trace", path);
	//unix_error(msg);
    //}
    sscanf(scanptr, "%d%n", &(trace->sugg_heapsize), &bytes_scanned); /* not used */
	scanptr += bytes_scanned;
    sscanf(scanptr, "%d%n", &(trace->num_ids), &bytes_scanned);     
	scanptr += bytes_scanned;
    sscanf(scanptr, "%d%n", &(trace->num_ops), &bytes_scanned);     
	scanptr += bytes_scanned;
    sscanf(scanptr, "%d%n", &(trace->weight), &bytes_scanned);        /* not used */
	scanptr += bytes_scanned;
    
    /* We'll store each request line in the trace in this array */
    if ((trace->ops = 
	 (traceop_t *)sys_malloc(trace->num_ops * sizeof(traceop_t))) == NULL)
		unix_error("malloc 2 failed in read_trace");
	test_mem_use += sizeof(traceop_t);

    /* We'll keep an array of pointers to the allocated blocks here... */
    if ((trace->blocks = 
	 (char **)sys_malloc(trace->num_ids * sizeof(char *))) == NULL)
		unix_error("malloc 3 failed in read_trace");
	test_mem_use += trace->num_ids * sizeof(char *);

    /* ... along with the corresponding byte sizes of each block */
    if ((trace->block_sizes = 
	 (size_t *)sys_malloc(trace->num_ids * sizeof(size_t))) == NULL)
		unix_error("malloc 4 failed in read_trace");
	test_mem_use += trace->num_ids * sizeof(size_t);
    
    /* read every request line in the trace file */
    index = 0;
    op_index = 0;
    while (sscanf(scanptr, "%s%n", type, &bytes_scanned) != EOF) {
		scanptr += bytes_scanned;
		switch(type[0]) {
		case 'a':
			sscanf(scanptr, "%u %u%n", &index, &size, &bytes_scanned);
			scanptr += bytes_scanned;
			trace->ops[op_index].type = ALLOC;
			trace->ops[op_index].index = index;
			trace->ops[op_index].size = size;
			max_index = (index > max_index) ? index : max_index;
			break;
		case 'r':
			sscanf(scanptr, "%u %u%n", &index, &size, &bytes_scanned);
			scanptr += bytes_scanned;
			trace->ops[op_index].type = REALLOC;
			trace->ops[op_index].index = index;
			trace->ops[op_index].size = size;
			max_index = (index > max_index) ? index : max_index;
			break;
		case 'f':
			sscanf(scanptr, "%u%n", &index, &bytes_scanned);
			scanptr += bytes_scanned;
			trace->ops[op_index].type = FREE;
			trace->ops[op_index].index = index;
			break;
		case 's':
			stack_test();
			break;
		case 'h':
			heap_test();
			break;
		default:
			sprintf(msg, "Bogus type character (%c) in string (%s) in tracefile %s\n", 
			   type[0], scanptr-bytes_scanned, path);
			var_print(msg);
			loop();
		}
		op_index++;
    }
	// Removed to allow shortened trace files
    //assert(max_index == trace->num_ids - 1);
    //assert(trace->num_ops == op_index);
    
    return trace;
}

/*
 * free_trace - Free the trace record and the three arrays it points
 *              to, all of which were allocated in read_trace().
 */
void free_trace(trace_t *trace)
{
    sys_free(trace->ops);         /* free the three arrays... */
    sys_free(trace->blocks);      
    sys_free(trace->block_sizes);
    sys_free(trace);              /* and the trace record itself... */
}

/**********************************************************************
 * The following functions evaluate the correctness, space utilization,
 * and throughput of the libc and mm malloc packages.
 **********************************************************************/

/*
 * eval_mm_valid - Check the mm malloc package for correctness
 */
static int eval_mm_valid(trace_t *trace, int tracenum, range_t **ranges) 
{
    int i, j;
    int index;
    int size;
    int oldsize;
    char *newp;
    char *oldp;
    char *p;
    
    /* Reset the heap and free any records in the range list */
    clear_ranges(ranges);

    /* Call the mm package's init function */
	/*
    if (mm_init() < 0) {
	malloc_error(tracenum, 0, "mm_init failed.");
	return 0;
    }
	*/

    /* Interpret each operation in the trace in order */
    for (i = 0;  i < trace->num_ops;  i++) {
	index = trace->ops[i].index;
	size = trace->ops[i].size;

        switch (trace->ops[i].type) {

        case ALLOC: /* sys_malloc */

	    /* Call the student's malloc */
	    if ((p = sys_malloc(size)) == NULL) {
		malloc_error(tracenum, i, "sys_malloc failed.");
		return 0;
	    }
	    
	    /* 
	     * Test the range of the new block for correctness and add it 
	     * to the range list if OK. The block must be  be aligned properly,
	     * and must not overlap any currently allocated block. 
	     */ 
	    if (add_range(ranges, p, size, tracenum, i) == 0)
		return 0;
	    
	    /* ADDED: cgw
	     * fill range with low byte of index.  This will be used later
	     * if we realloc the block and wish to make sure that the old
	     * data was copied to the new block
	     */
	    memset(p, index & 0xFF, size);

	    /* Remember region */
	    trace->blocks[index] = p;
	    trace->block_sizes[index] = size;
	    break;

        case REALLOC: /* sys_realloc */
	    
	    /* Call the student's realloc */
	    oldp = trace->blocks[index];
	    if ((newp = sys_realloc(oldp, size)) == NULL) {
		malloc_error(tracenum, i, "sys_realloc failed.");
		return 0;
	    }
	    
	    /* Remove the old region from the range list */
	    remove_range(ranges, oldp);
	    
	    /* Check new block for correctness and add it to range list */
	    if (add_range(ranges, newp, size, tracenum, i) == 0)
		return 0;
	    
	    /* ADDED: cgw
	     * Make sure that the new block contains the data from the old 
	     * block and then fill in the new block with the low order byte
	     * of the new index
	     */
	    oldsize = trace->block_sizes[index];
	    if (size < oldsize) oldsize = size;
	    for (j = 0; j < oldsize; j++) {
	      if (newp[j] != (index & 0xFF)) {
		malloc_error(tracenum, i, "sys_realloc did not preserve the "
			     "data from old block");
		return 0;
	      }
	    }
	    memset(newp, index & 0xFF, size);

	    /* Remember region */
	    trace->blocks[index] = newp;
	    trace->block_sizes[index] = size;
	    break;

        case FREE: /* sys_free */
	    
	    /* Remove region from list and call student's free function */
	    p = trace->blocks[index];
	    remove_range(ranges, p);
	    sys_free(p);
	    break;

	default:
		sprintf(msg, "Type %d", trace->ops[i].type);
		var_print(msg);
	    app_error("Nonexistent request type in eval_mm_valid");
        }

    }

    /* As far as we know, this is a valid malloc package */
    return 1;
}

/* 
 * eval_mm_util - Evaluate the space utilization of the student's package
 *   The idea is to remember the high water mark "hwm" of the heap for 
 *   an optimal allocator, i.e., no gaps and no internal fragmentation.
 *   Utilization is the ratio hwm/heapsize, where heapsize is the 
 *   size of the heap in bytes after running the student's malloc 
 *   package on the trace. Note that our implementation of mem_sbrk() 
 *   doesn't allow the students to decrement the brk pointer, so brk
 *   is always the high water mark of the heap. 
 *   
 */
static double eval_mm_util(trace_t *trace, int tracenum, range_t **ranges)
{   
    int i;
    int index;
    int size, newsize, oldsize;
    int max_total_size = 0;
    int total_size = 0;
    char *p;
    char *newp, *oldp;

    /* initialize the heap and the mm malloc package */
    //mem_reset_brk();
    //if (mm_init() < 0)
	//app_error("mm_init failed in eval_mm_util");

    for (i = 0;  i < trace->num_ops;  i++) {
        switch (trace->ops[i].type) {

        case ALLOC: /* mm_alloc */
	    index = trace->ops[i].index;
	    size = trace->ops[i].size;

	    if ((p = sys_malloc(size)) == NULL) 
			app_error("sys_malloc failed in eval_mm_util");
	    
	    /* Remember region and size */
	    trace->blocks[index] = p;
	    trace->block_sizes[index] = size;
	    
	    /* Keep track of current total size
	     * of all allocated blocks */
	    total_size += size;
	    
	    /* Update statistics */
	    max_total_size = (total_size > max_total_size) ?
		total_size : max_total_size;
	    break;

	case REALLOC: /* sys_realloc */
	    index = trace->ops[i].index;
	    newsize = trace->ops[i].size;
	    oldsize = trace->block_sizes[index];

	    oldp = trace->blocks[index];
	    if ((newp = sys_realloc(oldp,newsize)) == NULL)
		app_error("sys_realloc failed in eval_mm_util");

	    /* Remember region and size */
	    trace->blocks[index] = newp;
	    trace->block_sizes[index] = newsize;
	    
	    /* Keep track of current total size
	     * of all allocated blocks */
	    total_size += (newsize - oldsize);
	    
	    /* Update statistics */
	    max_total_size = (total_size > max_total_size) ?
		total_size : max_total_size;
	    break;

        case FREE: /* sys_free */
	    index = trace->ops[i].index;
	    size = trace->block_sizes[index];
	    p = trace->blocks[index];
	    
	    sys_free(p);
	    
	    /* Keep track of current total size
	     * of all allocated blocks */
	    total_size -= size;
	    
	    break;

	default:
	    app_error("Nonexistent request type in eval_mm_util");

        }
    }

	max_total_size += test_mem_use;

    return ((double)max_total_size  / (double)mem_heapsize());
}


/*
 * eval_mm_speed - This is the function that is used by fcyc()
 *    to measure the running time of the mm malloc package.
 */
inline static void eval_mm_speed(void *ptr)
{
    int i, index, size, newsize;
    char *p, *newp, *oldp, *block;
    trace_t *trace = ((speed_t *)ptr)->trace;

    /* Reset the heap and initialize the mm package */
    //mem_reset_brk();
    //if (mm_init() < 0) 
	//app_error("mm_init failed in eval_mm_speed");

    /* Interpret each trace request */
    for (i = 0;  i < trace->num_ops;  i++)
        switch (trace->ops[i].type) {

        case ALLOC: /* sys_malloc */
            index = trace->ops[i].index;
            size = trace->ops[i].size;
            if ((p = sys_malloc(size)) == NULL)
		app_error("sys_malloc error in eval_mm_speed");
            trace->blocks[index] = p;
            break;

	case REALLOC: /* sys_realloc */
	    index = trace->ops[i].index;
            newsize = trace->ops[i].size;
	    oldp = trace->blocks[index];
            if ((newp = sys_realloc(oldp,newsize)) == NULL)
		app_error("sys_realloc error in eval_mm_speed");
            trace->blocks[index] = newp;
            break;

        case FREE: /* sys_free */
            index = trace->ops[i].index;
            block = trace->blocks[index];
            sys_free(block);
            break;

	default:
	    app_error("Nonexistent request type in eval_mm_valid");
        }
}

/*************************************
 * Some miscellaneous helper routines
 ************************************/

/*
 * printresults - prints a performance summary for some malloc package
 */
static void printresults(int n, stats_t *stats) 
{
    int i;
    double secs = 0;
    double ops = 0;
    double util = 0;

    /* Print the individual results for each trace */
    sprintf(msg, "%5s%7s %5s%8s%10s%6s\n", 
	   "trace", " valid", "util", "ops", "secs", "Kops");
	var_print(msg);
    for (i=0; i < n; i++) {
	if (stats[i].valid) {
	    sprintf(msg, "%2d%10s%5.0f%%%8.0f%10.6f%6.0f\n", 
		   i,
		   "yes",
		   stats[i].util*100.0,
		   stats[i].ops,
		   stats[i].secs,
		   (stats[i].ops/1e3)/stats[i].secs);
		var_print(msg);
	    secs += stats[i].secs;
	    ops += stats[i].ops;
	    util += stats[i].util;
	}
	else {
	    sprintf(msg,"%2d%10s%6s%8s%10s%6s\n", 
		   i,
		   "no",
		   "-",
		   "-",
		   "-",
		   "-");
		var_print(msg);
	}
    }

    /* Print the aggregate results for the set of traces */
    if (errors == 0) {
	sprintf(msg, "%12s%5.0f%%%8.0f%10.6f%6.0f\n", 
	       "Total       ",
	       (util/n)*100.0,
	       ops, 
	       secs,
	       (ops/1e3)/secs);
	var_print(msg);
    }
    else {
	sprintf(msg, "%12s%6s%8s%10s%6s\n", 
	       "Total       ",
	       "-", 
	       "-", 
	       "-", 
	       "-");
	var_print(msg);
    }

}

/* 
 * app_error - Report an arbitrary application error
 */
void app_error(char * err_msg) 
{
    sprintf(msg, "%s\n", err_msg);
	var_print(msg);
	loop();
}

/* 
 * unix_error - Report a Unix-style error
 */
void unix_error(char * err_msg) 
{
    sprintf(msg, "%s: %s\n", err_msg, strerror(errno));
	var_print(msg);
	loop();
}

/*
 * malloc_error - Report an error returned by the sys_malloc package
 */
void malloc_error(int tracenum, int opnum, char *err_msg)
{
    errors++;
    sprintf(msg, "ERROR [trace %d, line %d]: %s\n", tracenum, LINENUM(opnum), err_msg);
	var_print(msg);
	loop();
}
