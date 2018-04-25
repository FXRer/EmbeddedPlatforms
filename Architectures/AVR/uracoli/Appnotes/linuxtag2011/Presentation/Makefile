export PATH := /home/mavo/nobackup/asciidoc/bin:$(PATH)

FILES = slides.txt \
		slides_intro.txt \
		slides_radio.txt \
		slides_sensor.txt \
		slides_server.txt \
		slides_outro.txt

all: html
pdf: slides.pdf
html: slides.html

slides.html: $(FILES)
	asciidoc --attribute linkcss --backend slidy slides.txt

slides.pdf: $(FILES)
	a2x -f pdf slides.txt

clean:
	-rm -rf slides.html slides.pdf
