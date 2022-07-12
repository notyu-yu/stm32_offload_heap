echo "=================================================="
echo "activate openocd"
trap "kill 0" EXIT
openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg > /dev/null 2>/dev/null &
echo "=================================================="
echo "short tests"
echo "short1.rep"
python ./helper_scripts/rep_to_hdr.py ./tracefiles/short1.rep
make > /dev/null 2> /dev/null
arm-none-eabi-gdb -x ./helper_scripts/gdbtrace.txt ./Debug/mcu_mdriver.elf > ./gdb_out.txt
python ./helper_scripts/extract_output.py
echo "=================================================="
echo "short2.rep"
python ./helper_scripts/rep_to_hdr.py ./tracefiles/short2.rep
make > /dev/null 2> /dev/null
arm-none-eabi-gdb -x ./helper_scripts/gdbtrace.txt ./Debug/mcu_mdriver.elf > ./gdb_out.txt
python ./helper_scripts/extract_output.py
echo "=================================================="
echo "coalescing-bal.rep"
python ./helper_scripts/rep_to_hdr.py ./short_trace/coalescing-bal.rep
make > /dev/null 2> /dev/null
arm-none-eabi-gdb -x ./helper_scripts/gdbtrace.txt ./Debug/mcu_mdriver.elf > ./gdb_out.txt
python ./helper_scripts/extract_output.py
