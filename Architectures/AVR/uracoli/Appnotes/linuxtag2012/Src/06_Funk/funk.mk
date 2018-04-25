# $Id$
BOARD=res
#BOARD=cap

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/funk_res.hex $(OUTDIR)/funk_cap.hex

funk_res.out: funk.c leds.c restouch.c
	avr-gcc -I../uracoli/inc/ -Dxxo -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o $@ funk.c leds.c restouch.c -L../uracoli/lib -lradio_xxo

funk_cap.out: funk.c leds.c captouch.c
	avr-gcc -I../uracoli/inc/ -Dxxo -O2 -DF_CPU=1000000UL -g -mmcu=atmega128rfa1 -o $@ funk.c leds.c captouch.c -L../uracoli/lib -lradio_xxo

$(OUTDIR)/funk%.hex: funk%.out
	avr-objcopy -O ihex $< $@

flash: $(OUTDIR)/funk_$(BOARD).hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:$<:i

debug:
	sleep 2
	$(AVARICE) -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg funk_$(BOARD).out

clean:
	@rm -rf *.o *.out *.hex
