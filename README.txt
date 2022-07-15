Project files in projects/offload_heap

Usage:
After running make and connecting the UART cable (PA2 is TX, PA3 is RX), run the "pc_server" binary with sudo on the linux machine, and burn the program ("Debug/mcu_mdriver" for this project) onto the MCU. Resetting the MCU to run the script will use the linux server for memory management.

Using the MCU malloc library:
Include the mcu_mm.h, memlib.h, and mcu_timer.h scripts. Call mm_init() and timer_init() to setup the malloc library and stack overflow checking. The functions in mcu_mm.h and memlib.h should function as their standard counterparts.

MCU side code:
mcu_mdriver.c: Runs the test script written in config.h.
mcu_mlib.c: Provides sbrk related functions.
mcu_mm.c: Provides malloc related functions.
mcu_request.c: Provides malloc communication related functions.
mcu_timer.c: Provides timer and stack overflow check.
mcu.c: Provides debugging functions.
uart.c: Provides UART communication functions.
uart_dma.c: Provides UART communication functions using DMA.

PC side code:
pc_mlib.c: Provides sbrk related functions.
pc_mm.c: Provides malloc related functions.
pc_request.c: Provides malloc request communication functions.
pc_server.c: Continunously monitors and handles malloc request from UART.

Helper scripts:
rep_to_hdr.py: Converts a .rep trace file to teststring.h.
tracechecker.py: Checks if trace files are valid.
extract_output.py: Extract the program output string from gdb_out.txt when gdb is ran with -x gdbtrace.txt.
trace_gen.py: Randomly generate trace files for testing.
run.sh: Runs test scripts in the short_trace directory, the pc_server binary needs to be manually restarted for every test with sudo privelige.
