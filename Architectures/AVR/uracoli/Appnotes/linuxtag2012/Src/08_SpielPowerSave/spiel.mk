# $Id$
BOARD=res
#BOARD=cap

# allow defining avrdude/avarice tools by environment variables
# i.e. AVARICE='sudo avarice' before running makefiles
AVRDUDE?=avrdude
AVARICE?=avarice
OUTDIR=./

all: $(OUTDIR)/spiel_res.hex $(OUTDIR)/spiel_cap.hex

spiel_res.out: spiel.c funk.c leds.c restouch.c misc.c
	avr-gcc -Wall -I../uracoli/inc/ -Dxxo -O2 -DF_CPU=1000000UL -g \
	-funsigned-char -funsigned-bitfields -std=gnu99 \
	-mmcu=atmega128rfa1 -o $@ spiel.c funk.c leds.c restouch.c misc.c \
	-L../uracoli/lib -lradio_xxo

spiel_cap.out: spiel.c funk.c leds.c captouch.c misc.c
	avr-gcc -Wall -I../uracoli/inc/ -Dxxo -O2 -DF_CPU=1000000UL -g \
	-funsigned-char -funsigned-bitfields -std=gnu99 \
	-mmcu=atmega128rfa1 -o $@ spiel.c funk.c leds.c captouch.c misc.c \
	-L../uracoli/lib -lradio_xxo

$(OUTDIR)/spiel%.hex: spiel%.out
	avr-objcopy -O ihex $< $@

flash: $(OUTDIR)/spiel_$(BOARD).hex
	$(AVRDUDE) -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:$<:i

debug:
	sleep 2
	$(AVARICE) -C -I -P atmega128rfa1 -2g --detach :4242
	avr-gdb -x ../debug.cfg spiel_$(BOARD).out

clean:
	@rm -rf *.o *.out *.hex
