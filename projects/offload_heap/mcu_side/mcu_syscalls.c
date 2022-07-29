#include "mcu_syscalls.h"
#include "mcu_mm.h"
#include "mcu_mpu.h"
#include "mcu_timer.h"

void SVC_Handler(void) {
	__asm (
			".global SVC_Handler_Main\n" "TST lr, $4\n"
			"ITE EQ\n"
			"MRSEQ r0, MSP\n"
			"MRSNE r0, PSP\n" // Check to use msp or psp
			"B SVC_Handler_Main\n" // Go to the C handler function
	);
}

void SVC_Handler_Main(unsigned int * svc_args) {
	// Stack frame contents: r0-r3, LR, PC, and xPSR
	// Correspond with svc_args[0 to 7]
	// svc_args array contains arguments at start and return values at end, in order

	uint32_t svc_number = ((char *)svc_args[6])[-2];
	switch(svc_number) {
		case 0: // mm_init
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk); // Enable privileged mode
			mm_init();
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk); // Disable privileged mode
			break;
		case 1: // mm_malloc
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			svc_args[0] = (uint32_t)mm_malloc(svc_args[0]);
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
			break;
		case 2: // mm_free
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			mm_free((void *)(svc_args[0]));
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
			break;
		case 3: // mm_realloc
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			svc_args[0] = (uint32_t)mm_realloc((void *)svc_args[0], svc_args[1]);
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
			break;
		case 4: // mm_finish
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			mm_finish();
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
			break;
		case 5: // get_time
			__set_CONTROL(__get_CONTROL() & ~CONTROL_nPRIV_Msk);
			svc_args[0] = get_time();
			__set_CONTROL(__get_CONTROL() | CONTROL_nPRIV_Msk);
			break;
		default:
			break;
	}
}

// Initialize malloc library
void sys_mm_init(void) {
	asm volatile ("svc #0");
}

// Malloc size bytes of memory
void * sys_malloc(size_t size) {
	asm volatile ("svc #1");
	register uint32_t * ret_val asm("r0");
	return (void *)ret_val;
}

// Free memory region at pointer
void sys_free(void * ptr) {
	asm volatile ("svc #2");
}

// Reallocate ptr to a size byte region and return the new pointer
void * sys_realloc(void * ptr, size_t size) {
	asm volatile ("svc #3");
	register uint32_t * ret_val asm("r0");
	return (void *) ret_val;
}

// End communication session with server
void sys_mm_finish(void) {
	asm volatile ("svc #4");
}

// Return current time in ms
size_t sys_get_time(void) {
	asm volatile ("svc #5");
	register uint32_t * ret_val asm("r0");
	return (size_t) ret_val;
}
