Project files in projects/heap

run.sh in this directory will run tests on all trace files and display utilization and throughput.

Code overview:
mm.c: Original implemetation of CSAPP malloc lab, provide malloc related functions.
mcu.c: Provides loop and var_print function for gdb output.
mcu_timer.c: Provides timing functions and check for stack overflow.
mcu_mdriver.c: Read trace files loaded into teststring.h and runs the same test as the orignial malloc lab.
mcu_mlib.c: Provides sbrk related functions.

Helper scripts:
rep_to_hdr.py: Converts a .rep trace file to teststring.h.
tracechecker.py: Checks if trace files are valid.
extract_output.py: Extract the program output string from gdb_out.txt when gdb is ran with -x gdbtrace.txt.
trace_gen.py: Randomly generate trace files for testing.
