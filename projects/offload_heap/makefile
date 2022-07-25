TARGET = mcu_mdriver
SRCS = mcu_side/mcu_mdriver.c mcu_side/mcu_mlib.c mcu_side/mcu_mm.c mcu_side/mcu_timer.c mcu_side/mcu.c mcu_side/mcu_request.c mcu_side/uart.c mcu_side/uart_dma.c mcu_side/mcu_syscalls.c mcu_side/mcu_mpu.c mcu_side/mcu_init.c

LINKER_SCRIPT = ../../flash/STM32F411VEHX_FLASH.ld

# Generate debug info
DEBUG = 1

# Choose processor
CDEFS  = -DSTM32F411xE
# Enable FPU
#CDEFS += -D__VFP_FP__

include ../armf4.mk
