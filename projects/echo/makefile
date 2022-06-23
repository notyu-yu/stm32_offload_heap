TARGET = mcu_echo
SRCS = mcu_echo.c uart_dma.c uart.c

LINKER_SCRIPT = ../../flash/STM32F411VEHX_FLASH.ld

# Generate debug info
DEBUG = 1

# Choose processor
CDEFS  = -DSTM32F411xE
# Enable FPU
#CDEFS += -D__VFP_FP__

include ../armf4.mk
