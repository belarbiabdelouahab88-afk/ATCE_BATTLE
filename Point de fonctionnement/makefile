MCU=atmega32u4
DFU=dfu-programmer
EXEC=point_de_fonct
CC=avr-gcc
CFLAGS=-mmcu=atmega32u4 -Os -Wall -I../compilation/lufa/lufa-LUFA-140928 -I../compilation/lufa/VirtualSerial_example -L../compilation/lufa/VirtualSerial_lib  -std=gnu99

all:$(EXEC).hex

$(EXEC).hex: $(EXEC).out
	avr-objcopy -Oihex $(EXEC).out $(EXEC).hex
	
$(EXEC).out: $(EXEC).c
	$(CC) $(CFLAGS) -o $(EXEC).out $(EXEC).c -lVirtualSerial

clean:
	rm $(EXEC).hex $(EXEC).out

install: $(EXEC).hex
	$(DFU) $(MCU) erase --force
	$(DFU) $(MCU) flash $(EXEC).hex
	$(DFU) $(MCU) reset
