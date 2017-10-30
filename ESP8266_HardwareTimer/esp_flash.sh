#!/bin/sh

echo Initiating the firmware flash process
make clean
make
cd firmware
esptool.py --port /dev/ttyUSB0 write_flash -fm dio -fs 4MB -ff 40m 0x00000 0x00000.bin 0x40000 0x40000.bin
cd ..

