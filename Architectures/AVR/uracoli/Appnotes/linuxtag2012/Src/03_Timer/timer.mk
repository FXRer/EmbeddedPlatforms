# $Id$

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/timer.hex

timer.out: timer.c
	avr-gcc -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o timer.out timer.c

$(OUTDIR)/timer.hex: timer.out
	avr-objcopy -O ihex timer.out $(OUTDIR)/timer.hex

flash: $(OUTDIR)/timer.hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:timer.hex:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg timer.out

clean:
	@rm -rf *.out *.hex
