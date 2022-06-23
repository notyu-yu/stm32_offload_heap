echo "=================================================="
echo "activate openocd"
trap "kill 0" EXIT
openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg > /dev/null 2>/dev/null &
for fn in ./short_trace/*.rep; do
	echo "=================================================="
	echo $fn
	python ./rep_to_hdr.py $fn
	make > /dev/null 2> /dev/null
	arm-none-eabi-gdb -x ./gdbtrace.txt ./Debug/mcu_mdriver.elf > ./gdb_out.txt
	python ./extract_output.py
done
