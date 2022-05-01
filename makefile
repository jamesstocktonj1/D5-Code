# AVR Makefile
# James Stockton


# make file commands
#	all 	- compiles whole program
#	iotest	- test dac pins
#	disasm 	- disassembles elf file
#	size 	- shows size of elf file
#	test	- test programmer connection
#	flash	- upload hex to target
#	fuse	- set fuses of target
#	clean	- deletes compiled files

# avr constants
MCU = atmega644p
CLK = 12000000

# compiler constants
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
OBJSIZE = avr-size

# compiler flags
CFLAGS = -Wall -Os
TARGET = main
TEST = iotest

# build and source folders
BUILD = build
SOURCE = src
INCLUDES = $(wildcard $(SOURCE)/*.c)
LIB = lib
LIBRARY = $(wildcard $(LIB)/*.c)

#programmer constant
AVRDUDE = avrdude 
PROGRAMMER = usbasp

#fuses
LF = 0xff
HF = 0x9c
EF = 0xff


all: $(BUILD)/$(TARGET).hex

testio: $(BUILD)/$(TEST).hex

disasm: $(BUILD)/$(TARGET).elf
	$(OBJDUMP) -d $(BUILD)/$(TARGET).elf

size: $(BUILD)/$(TARGET).elf
	$(OBJSIZE) -C $(BUILD)/$(TARGET).elf

test:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU)

flash: all
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U flash:w:$(BUILD)/$(TARGET).hex

fuse:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U lfuse:w:$(LF):m -U hfuse:w:$(HF):m -U efuse:w:$(EF):m

clean:
	rm -f $(BUILD)/$(TARGET).elf
	rm -f $(BUILD)/$(TARGET).hex
	rm -f $(BUILD)/$(TEST).elf
	rm -f $(BUILD)/$(TEST).hex


$(BUILD)/$(TARGET).elf: $(TARGET).c $(INCLUDES) $(LIBRARY)
	$(CC) -DF_CPU=$(CLK) -mmcu=$(MCU) $(CFLAGS) $(TARGET).c $(INCLUDES) $(LIBRARY) -o $(BUILD)/$(TARGET).elf

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex -O ihex


$(BUILD)/$(TEST).elf: $(TEST).c $(INCLUDES)
	$(CC) -DF_CPU=$(CLK) -mmcu=$(MCU) $(CFLAGS) $(TEST).c $(INCLUDES) -o $(BUILD)/$(TEST).elf

$(BUILD)/$(TEST).hex: $(BUILD)/$(TEST).elf
	$(OBJCOPY) $(BUILD)/$(TEST).elf $(BUILD)/$(TEST).hex -O ihex