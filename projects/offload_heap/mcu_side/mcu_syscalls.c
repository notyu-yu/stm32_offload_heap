#include "mcu_syscalls.h"
#include "mcu_mm.h"
#include "mcu_mpu.h"
#include "mcu_timer.h"

void SVC_Handler(void) {
	__asm (
			".global SVC_Handler_Main\n"
			"TST lr, $4\n"
			"ITE EQ\n"
			"MRSEQ r0, MSP\n"
			"MRSNE r0, PSP\n" // Check to use msp or psp
			"B SVC_Handler_Main\n" // Go to the C handler function
	);
}

void SVC_Handler_Main(unsigned int * svc_args) {
	// Stack frame contents: r0-r3, LR, PC, and xPSR
	// Correspond with svc_args[0 to 7]
	// First registers are arguments and return values

	unsigned int svc_number = ((char *)svc_args[6])[-2];
	switch(svc_number) {
		case 0: // Enable privileged mode
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			break;
		default:
			break;
	}
}

void priv_mode_on(void) {
	asm volatile ("svc #0");
}

void priv_mode_off(void) {
	__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
}

// Initialize malloc library
void sys_mm_init(void) {
	priv_mode_on();
	mm_init();
	priv_mode_off();
}

// Initialize system timer - not needed after mm_init
void sys_timer_init(void) {
	priv_mode_on();
	timer_init();
	priv_mode_off();
}

// Malloc size bytes of memory
void * sys_malloc(size_t size) {
	void * ptr;
	priv_mode_on();
	ptr = mm_malloc(size);
	priv_mode_off();
	return ptr;
}

// Free memory region at pointer
void sys_free(void * ptr) {
	priv_mode_on();
	mm_free(ptr);
	priv_mode_off();
}

// Reallocate ptr to a size byte region and return the new pointer
void * sys_realloc(void * ptr, size_t size) {
	void * newptr;
	priv_mode_on();
	newptr = mm_realloc(ptr, size);
	priv_mode_off();
	return newptr;
}

// End communication session with server
void sys_mm_finish(void) {
	priv_mode_on();
	mm_finish();
	priv_mode_off();
}

// Return current time in ms
size_t sys_get_time(void) {
	size_t t;
	priv_mode_on();
	t = get_time();
	priv_mode_off();
	return t;
}
