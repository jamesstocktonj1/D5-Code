avr-gcc -mmcu=atmega644p -DF_CPU=12000000 -Wall -Os main.c io.c lib/liblcd.a -o build/prog.elf
avr-objcopy -O ihex build/prog.elf build/prog.hex
avrdude -c usbasp -p m644p -U flash:w:build/prog.hex