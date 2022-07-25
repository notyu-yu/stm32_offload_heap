#include "mcu_mpu.h"

void mpu_init(void) {
	uint32_t const mpu_cfg_rbar[4] = {
		// Flash - region 0
		0x08000000,
		// SRAM - Region 1
		0x20000000,
		// GPIOD _ Region 2
		GPIOD_BASE,
		// RCC - Region 3
		RCC_BASE,
	};

	uint32_t const mpu_cfg_rasr[4] = {
		// Flash
		(MPU_DEFS_RASR_SIZE_512KB | MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk),
		// SRAM
		(MPU_DEFS_RASR_SIZE_128KB | MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk),
		// GPIOD
		(MPU_DEFS_RASR_SIZE_1KB | MPU_DEFS_SHARED_DEVICE | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk),
		// RCC
		(MPU_DEFS_RASR_SIZE_1KB | MPU_DEFS_SHARED_DEVICE | MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk)
	};

	if (MPU->TYPE == 0) {return;} // Do nothing if MPU don't exist
	__DMB(); // Finish outstanding transfers
	
	MPU->CTRL = 0; // Disable first
	
	for (size_t i=0; i<4; i++) {
		MPU->RNR = i; // Select region	
		MPU->RBAR = mpu_cfg_rbar[i]; // Write base address register
		MPU->RASR = mpu_cfg_rasr[i]; // Region attribute and size register
	}

	for (size_t i=4; i<8; i++) {
		// Disable unused regions
		MPU->RNR = i; // Select region	
		MPU->RBAR = 0; // Base address
		MPU->RASR = 0; // Region attribute and size register
	}

	MPU->CTRL |= 1<<2; // Enable privileged background region

	MPU->CTRL = MPU_CTRL_ENABLE_Msk; // Enable MPU
	
	__DSB(); // Memory barrier for subsequence data & instruction
	__ISB(); // Transfers using updated MPU settings
}
