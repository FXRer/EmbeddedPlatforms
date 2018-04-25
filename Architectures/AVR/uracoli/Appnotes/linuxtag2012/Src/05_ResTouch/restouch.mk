# $Id$

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/restouch.hex

restouch.out: restouch.c leds.c
	avr-gcc -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o restouch.out restouch.c leds.c

$(OUTDIR)/restouch.hex: restouch.out
	avr-objcopy -O ihex restouch.out restouch.hex

flash: $(OUTDIR)/restouch.hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:restouch.hex:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg restouch.out

clean:
	@rm -rf *.out *.hex
