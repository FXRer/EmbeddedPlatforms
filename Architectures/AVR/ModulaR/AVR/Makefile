APP		= ModulaR
DEVICE	= attiny85
OPTS	=

LKRSCPT		= module.lkr

OPT			= -Os #-O0 -g -ggdb3
CC_FLAGS	=  -Wall -Wextra $(OPT) -mmcu=$(DEVICE) -I/usr/lib/avr/include $(OPTS) -Wno-strict-aliasing -ffunction-sections -fdata-sections -fomit-frame-pointer -funsigned-bitfields -fpack-struct -fshort-enums -finline-limit=3 -ffreestanding  -fno-tree-scev-cprop -Wno-pragmas
LD_FLAGS	=  -Wall -Wextra $(OPT) -mmcu=$(DEVICE) -Wl,--gc-sections -lm -lc -Wl,-T $(LKRSCPT)
CC			= avr-gcc
LD			= avr-gcc
EXTRA		= avr-size -Ax $(APP) && avr-objcopy -j .text -j .data -O ihex $(APP) $(APP).hex

OBJS		= crt.o main.o usbdrvasm.o osccal.o usbdrv.o speck.o key.o

$(APP).hex: $(APP).elf
	avr-objcopy -j .text -j .data -j .rodata -O ihex $(APP).elf $(APP).hex

$(APP).elf: $(OBJS) $(LKRSCPT)
	$(LD) $(LD_FLAGS) -o $(APP).elf $(OBJS)
	avr-size -Ax $(APP).elf

AVR: $(APP).hex
	avrdude -p $(DEVICE) -c avrisp2 -P usb -U flash:w:$(APP).hex:i

%.o : %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

%.o : %.S
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(APP).elf $(APP).hex

fuses:
	avrdude -p $(DEVICE) -c avrisp2 -P usb -U efuse:w:0b11111110:m -U hfuse:w:0b11011101:m -U lfuse:w:0b11100001:m -F -B 250
	#BOD @ 2.5-2.9V & no HW bootloader
	#no JTAG & OCD, SPI_prog on, WDT off, defaults
	#no clock div, defaults, crystal clock

