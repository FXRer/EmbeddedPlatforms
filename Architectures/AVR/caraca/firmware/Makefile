#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1999 by Konrad Riedel and Claudio Lanconelli
#

#This may be: CANDONGLE, CANDONGLE2 CARACA_NODE1, CARACA_NODE2, CARACA_STAR2, CARACA_NODE_SKR
    CONFIG_VAR = CANDONGLE2

#You can run make with the parameter to override default CONFIG_VAR, but be aware
# that you need to run a 'make cleanobj' whenever you change the CONFIG_VAR
#make CONFIG_VAR=CANDONGLE
#make CONFIG_VAR=CARACA_STAR2
#
#If you want to build all targets run 'make all'

	LOG	= make.log
	ASM	= ava
	ASMOPT	= -v -L -l$(LOG) -I. -favr_noendianbug
	LDFLAGS = -v -l$(LOG) -j $(TRG).obj
	PRG	= uisp
	PRGOPT	= -dstk200 -dno-poll
#	PRGOPT  = /dev/avr --AtmelCAvr 38 
	OUTFMT	= motorola
#	OUTFMT	= intel

    ifeq ($(CONFIG_VAR),CARACA_NODE1)
# CARACA Node1
	CONF_H = config_node1.h
	OBJ = kernel.o eeprom.o canbus.o ir_rc5.o node.o node1_action.o
	TRG = node1
    else
    ifeq ($(CONFIG_VAR),CARACA_NODE2)
# CARACA Node2
	CONF_H = config_node2.h
	OBJ = kernel.o eeprom.o i2cbus.o ds1621.o canbus.o ir_rc5.o node.o node2_action.o
	TRG = node2
    else
    ifeq ($(CONFIG_VAR),CARACA_STAR2)
# CARACA Star2
	CONF_H = config_star2.h
	OBJ = kernel.o eeprom.o i2cbus.o canbus.o rtc.o star.o star2_action.o
	TRG = star2
    else
    ifeq ($(CONFIG_VAR),CANDONGLE)
# Dongle
	CONF_H = config_dongle.h
	OBJ = kernel.o eeprom.o canbus.o dongle.o
	TRG = dongle
    else
    ifeq ($(CONFIG_VAR),CARACA_NODE_SKR)
# CARACA Node_skr (2313)
	CONF_H = config_node_skr.h
	OBJ = kernel.o eeprom.o canbus.o node_skr.o  node_skr_action.o
	#OBJ = kernel.o eeprom.o i2cbus.o ds1621.o canbus.o ir_rc5.o node_skr.o
	TRG = node_skr
    else
    ifeq ($(CONFIG_VAR),CARACA_NODE2_SKR)
# CARACA Node2_skr (4433)
	CONF_H = config_node2_skr.h
	OBJ = node2_skr_action.o kernel.o eeprom.o i2cbus.o ds1621.o canbus.o ir_rc5.o node_skr.o
	TRG = node2_skr
    else
    ifeq ($(CONFIG_VAR),CANDONGLE2)
# Dongle2 (4433)
	CONF_H = config_dongle4433.h
	OBJ = kernel.o eeprom.o canbus.o dongle.o
	TRG = dongle2
    endif
    endif
    endif
    endif
    endif
    endif
    endif

#link: instructions to create output file from object files

$(TRG).rom: $(OBJ)
	$(ASM) --$(OUTFMT) --multiple-output-files -favr_noendianbug $(LIB) $(OBJ) $(LDFLAGS) -o $(TRG)
	mv $(TRG).flash $(TRG).rom
	mv $(TRG).eeprom $(TRG).eep

$(TRG).eep: $(OBJ)
	$(ASM) --$(OUTFMT) --multiple-output-files -favr_noendianbug $(LIB) $(OBJ) $(LDFLAGS) -o $(TRG)
	mv $(TRG).eeprom $(TRG).eep
	mv $(TRG).flash $(TRG).rom

$(TRG).out: $(OBJ)
	$(ASM) --$(OUTFMT) -favr_noendianbug $(LIB) $(OBJ) $(LDFLAGS) -o $@

.PHONY: clean cleanobj all upload erase reset

all:
	@ echo "*** Build CARACA Node2 ***"
	$(MAKE) cleanobj CONFIG_VAR=CARACA_NODE2
	$(MAKE) node2.rom CONFIG_VAR=CARACA_NODE2
	@ echo "*** Done ***"
	@ echo "*** Build CARACA Star2 ***"
	$(MAKE) cleanobj CONFIG_VAR=CARACA_STAR2
	$(MAKE) star2.rom CONFIG_VAR=CARACA_STAR2
	@ echo "*** Done ***"
	@ echo "*** Build CAN Dongle2 ***"
	$(MAKE) cleanobj CONFIG_VAR=CANDONGLE2
	$(MAKE) dongle2.rom CONFIG_VAR=CANDONGLE2
	@ echo "*** Done ***"

config.h: $(CONF_H)
	rm -f config.h
	cp $(CONF_H) config.h
	#ln -s $(CONF_H) config.h

cleanobj:
	rm -f *.o config.h

clean:
	rm -f *.o *.obj *~ *.log *.out *.flash *.eeprom *.rom *.eep *.lst config.h serial.h

%.o : %.s
	$(ASM) $< $(ASMOPT)

$(OBJ): config.h can_config.h macro.h vars.h kernel.e kernel.h canbus.e canbus.h eeprom.e serial.h
kernel.s: kernel.h
i2cbus.s: i2cbus.h
node.s: ds1621.e

serial.h:
	$(RM) serial.h
	echo '#define B_DATE "'`date +'%Y%m%d'`\" > serial.h
	echo -n 'Serial Number: '; \
        echo '#define SERIAL '`read SER; echo $$SER` >> serial.h
	#"


# Hardware and Clean functions

upload: $(TRG).out
	$(PRG) $(PRGOPT) --erase --upload $(TRG).out

erase:
	$(PRG) $(PRGOPT) --erase
	
reset:
	$(PRG) $(PRGOPT)
