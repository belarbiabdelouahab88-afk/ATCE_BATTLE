EXEC=led
DFU=dfu-programmer
CC=avr-gcc
CFLAGS=-mmcu=atmega32u4 -Os -Wall
MCU=atmega32u4

all:$(EXEC).hex

$(EXEC).hex: $(EXEC).out
	avr-objcopy -Oihex $(EXEC).out $(EXEC).hex
	
$(EXEC).out: $(EXEC).c
	$(CC) $(CFLAGS) -o $(EXEC).out $(EXEC).c

clean:
	rm $(EXEC).hex $(EXEC).out

install: $(EXEC).hex
	$(DFU) $(MCU) erase --force
	$(DFU) $(MCU) flash $(EXEC).hex
	$(DFU) $(MCU) reset
