echo "=================================================="
echo "activate openocd"
trap "kill 0" EXIT
openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg > /dev/null 2>/dev/null &
echo "=================================================="
echo "make program"
make
echo "=================================================="
echo "burn it"
arm-none-eabi-gdb -x ./gdbrc.txt ./Debug/mcu_echo.elf
