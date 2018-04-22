#------------------------------------------------------------------------
# Makefile for Profibus test board
# Jim Brady, Nov 2005
#
# Compiler:  Keil version 7
# Target:    Atmel AT89C52 CPU at 11.0592 MHz with 
#            external 64KB RAM and 64KB FLASH  
#------------------------------------------------------------------------

CFLAGS = LARGE OPTIMIZE(5,SPEED) ORDER NOPRINT

profibus.hex: profibus.abs
	OH51 profibus.abs

profibus.abs: main.obj spc3.obj util.obj intr.obj 
	BL51 main.obj, spc3.obj, util.obj, intr.obj to profibus.abs RAMSIZE(256)

main.obj: main.c main.h spc3.h util.h
	C51 main.c $(CFLAGS)

spc3.obj: spc3.c main.h spc3.h util.h
	C51 spc3.c $(CFLAGS)

util.obj: util.c main.h spc3.h util.h
	C51 util.c $(CFLAGS)

intr.obj: intr.c main.h spc3.h util.h
	C51 intr.c $(CFLAGS)

clean:
	del *.obj
	del *.bak
	del *.lst
	del *.abs



