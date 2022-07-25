Project files in projects/offload_heap
Designed for STM32F411 MCU

Usage:
1) Connect the MCU with UART cable (PA2 is TX, PA3 is RX).
2) Run make and burn the program (Debug/mcu_mdriver.elf).
3) Press the Reset button, when the blue LED turns on, run the pc_server executable.
4) The orange LED should turn on if the trace test successfully finished.

Programming with the MCU malloc library:
1) Include "mcu_syscalls.h" in the "mcu_side" directory.
2) Call sys_mm_init() to initialize the library.
3) The malloc functions in syscalls.h can now be used like their standard counterparts.
4) When the program finishes, run sys_mm_finish() to gracefully end communication with pc_server.

LED Indicators:
Blue: Run pc_server to continue program.
Red: Start signal error, usually due to UART setup problem.
Orange: Program successfully finished.
Green: Communicating with Linux server.

MCU side code:
mcu_mdriver.c: Runs the test script written in config.h.
mcu_mlib.c: Provides sbrk related functions.
mcu_mm.c: Provides malloc related functions.
mcu_request.c: Provides malloc communication related functions.
mcu_timer.c: Provides timer functions.
mcu.c: Provides debugging functions.
mcu_init.c: Provides interrupt and led functions.
mcu_syscalls.c: Provides syscalls for user programs.
mcu_mpu.c: Provides MPU functions.
uart.c: Provides UART communication functions.
uart_dma.c: Provides UART communication functions using DMA.

PC side code:
pc_mlib.c: Provides sbrk related functions.
pc_mm.c: Provides malloc related functions.
pc_request.c: Provides malloc request communication functions.
pc_server.c: Continuously monitors and handles malloc request from UART.
dict.c: Provides hash table functions;

Shared config file: shared_side/shared_config.h

Helper scripts:
rep_to_hdr.py: Converts a .rep trace file to teststring.h.
tracechecker.py: Checks if trace files are valid.
extract_output.py: Extract the program output string from gdb_out.txt when gdb is ran with -x gdbtrace.txt.
trace_gen.py: Randomly generate trace files for testing.
run.sh: Runs test scripts in the short_trace directory, the pc_server binary needs to be manually restarted for every test with sudo privilege.

Communications Implementation:
Communication between MCU and the Linux server is done with the mem_request struct defined in uart_comms.h.
request: Request type, defined in shared_config.h.
req_id: Unique id assigned to each request from MCU.
size: Size related to the request.
ptr: Pointer related to the request.

MCU to Linux request format:
___________________________________________________________________________________________
request  |Malloc       |Free           |Realloc             |Sbrk
___________________________________________________________________________________________
size     |Malloc size  |0              |Realloc size        |0 on initialization, 
                                                            |sbrk increment otherwise
___________________________________________________________________________________________
ptr      |Null         |Pointer freed  |Pointer realloc'ed  |Heap start on initialization,
                                                            |0 otherwise
___________________________________________________________________________________________

End Signal: SBRK request with 0 size and ptr.

Linux to MCU response format:
_______________________________________________________
request  |Malloc              |Realloc
_______________________________________________________
ptr      |Malloc'ed pointer   |Realloc'ed pointer
         |Null when sbrk needed ->
_______________________________________________________
Start Signal: Request with every field being 1.

Linux side Heap information data structure:
Doubly linked list/deque using blk_struct structure.
prev & next: Maintains linked list.
ptr: Block start pointer.
size: Block size.
alloc: 1 when allocated, 0 when free.
Start of list stored in list_start variable with 0 size and 1 alloc.
