# $Id$

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/captouch.hex

captouch.out: captouch.c leds.c
	avr-gcc -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o captouch.out captouch.c leds.c

$(OUTDIR)/captouch.hex: captouch.out
	avr-objcopy -O ihex captouch.out captouch.hex

flash: $(OUTDIR)/captouch.hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:captouch.hex:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg captouch.out

clean:
	@rm -rf *.out *.hex
