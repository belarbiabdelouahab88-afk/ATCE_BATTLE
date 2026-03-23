EXEC=blink
MCU=atmega32u4
DFU=dfu-programmer
CC=avr-gcc
CFLAGS=-mmcu=$(MCU) -Os -Wall -I/home/user/uc/compilation/lufa/VirtualSerial_example/ -I/home/user/uc/compilation/lufa/VirtualSerial_lib/ \
	-I/home/user/uc/compilation/lufa/lufa-LUFA-140928/ -DF_USB=16000000UL -DF_CPU=16000000UL -Os -std=gnu99


all: $(EXEC).hex

$(EXEC).hex: $(EXEC).o
	$(CC) $(CFLAGS) -L/home/user/uc/compilation/lufa/VirtualSerial_lib/ -o $(EXEC).hex $(EXEC).o -L../lufa/VirtualSerial_lib/ -lVirtualSerial

$(EXEC).o: $(EXEC).c
	$(CC) $(CFLAGS) -c $(EXEC).c 

clean:
	rm *.o $(EXEC).hex
	
install: $(EXEC).hex
	$(DFU) $(MCU) erase
	$(DFU) $(MCU) flash $(EXEC).hex
	$(DFU) $(MCU) reset
