#include "mcu.h"
#include "uart.h"
#include "mcu_mm.h"
#include "mcu_init.h"
#define MAXLINE 1024

static char output_str[MAXLINE*2];
size_t output_offset=0;
void * sp_reset = (void *)0x20005000;

void loop() {
	led_on(ORANGE);
	while(1) {}
}

// Append printed output to output_str
void var_print(char * str) {
	if (output_offset + strlen(str) <= MAXLINE*2) {
		strcat(output_str, str);
	} else {
		while(1){}
	}
}

