
PICS=cap_sense.png res_sense.png full_setup.jpg current.png

all: slides.pdf workshop.html

cap_sense.png: ../../Paper/cap_sense.pdf
	convert -resize 400 $< $@

res_sense.png: ../../Paper/res_sense.pdf
	convert -resize 400 $< $@

full_setup.jpg: ../../Poster/full_setup.jpg
	convert -resize 400  $< $@

current.png:
	montage \
	    -tile 2x3 -geometry 320x240 \
	    -label "Alle LEDs aus, MCU in spin lock" \
	    Img/xxo_cap_leds_off_nosleep.png \
	    -label "Alle LEDs aus, MCU in sleep" \
	    Img/xxo_cap_leds_off_sleep.png \
	    -label "LED 4 grün, MCU in spin lock" \
	    Img/xxo_cap_led4green_nosleep.png \
	    -label "LED 4 grün, MCU in sleep" \
	    Img/xxo_cap_led4green_sleep.png \
	    -label "LED 4 rot, MCU in spin lock" \
	    Img/xxo_cap_led4red_nosleep.png \
	    -label "LED 4 rot, MCU in sleep" \
	    Img/xxo_cap_led4red_sleep.png \
	    current.png

current2.png:
	montage \
	    -tile 2x2 -geometry 320x240 \
	    -label "Deep sleep" \
	    Img/xxo_spiel_deep_sleep.png \
	    -label "Deep sleep (Detail)" \
	    Img/xxo_spiel_deep_sleep_detail.png \
	    -label "Wiedereinschalten" \
	    Img/xxo_spiel_deep_sleep_wakeup.png \
	    current2.png

%.html: %.txt $(PICS)
	asciidoc -a lang=de $<

%.pdf: %.txt $(PICS)
	a2x -L --asciidoc-opts='-a lang=de' -fpdf $<

slides.pdf: slides.tex
	pdflatex slides.tex

clean:
	-rm workshop.pdf workshop.html slides.pdf
