#include "mcu.h"

void mpu_init(void) {
	MPU->CTRL = 0;
}
