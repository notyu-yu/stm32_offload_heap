#define BAUDRATE B4000000 // The USARTDIV bits for the MCU needs to be manually calculated and set in mcu_side/uart.c
#define BUFFERSIZE 2048 // Max message size, should not exceed 4095 due to termios restrictions
#define SERIALDEV "/dev/ttyUSB0" // UART device name
#define USE_DMA 1 // Whether or not to use DMA for UART
#define VERBOSE 0 // Whether or not to print debug message in pc_server
#define DICT_SEARCH 1 // 0 to use linear pointer search, 1 to use hashtable

// Free block search options
#define FIRST_FIT 0
#define BEST_FIT 1
#define SEG_FIT 2

#define SEARCH_OPT SEG_FIT // Input option macro here

#ifndef _STRING_H
	#include <string.h>
#endif

// Request types
#define MALLOC 0
#define FREE 1
#define REALLOC 2
#define SBRK 3
