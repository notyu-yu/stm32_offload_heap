TARGET = mcu_mdriver
SRCS = mcu_mdriver.c mcu_mlib.c mm.c mcu_timer.c mcu.c

LINKER_SCRIPT = ../../flash/STM32F411VEHX_FLASH.ld

# Generate debug info
DEBUG = 1

# Choose processor
CDEFS  = -DSTM32F411xE
# Enable FPU
#CDEFS += -D__VFP_FP__

include ../armf4.mk
