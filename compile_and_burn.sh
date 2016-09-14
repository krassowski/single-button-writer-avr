#!/bin/bash
if avr-gcc -mmcu=atmega8535 -Os -o $1.o $1.c $2 -std=c99 -Wall
then
    avr-objcopy --verbose -j .text -j .data -O ihex  $1.o  $1.hex
    avrdude -c usbasp -p m8535 -P /dev/programator -e
    avrdude -c usbasp -p m8535 -P /dev/programator -U flash:w:$1.hex
    rm $1.o $1.hex
else
    echo "Compilation failed"
fi
