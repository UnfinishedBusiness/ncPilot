avrdude -Cavrdude.conf -v -V -patmega328p -carduino -P$1 -b115200 -D -Uflash:w:firmware.hex:i
