# $Id$

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/led1.hex

led1.out: led1.c
	avr-gcc -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o led1.out led1.c

$(OUTDIR)/led1.hex: led1.out
	avr-objcopy -O ihex led1.out led1.hex

flash: $(OUTDIR)/led1.hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:led1.hex:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg led1.out

clean:
	@rm -rf *.out *.hex
