# $Id$

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/leds.hex

leds.out: leds.c
	avr-gcc -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o leds.out leds.c

$(OUTDIR)/leds.hex: leds.out
	avr-objcopy -O ihex leds.out $(OUTDIR)/leds.hex

flash: $(OUTDIR)/leds.hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:leds.hex:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg leds.out

clean:
	@rm -rf *.out *.hex
