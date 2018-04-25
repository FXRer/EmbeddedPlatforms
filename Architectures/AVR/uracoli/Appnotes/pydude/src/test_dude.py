# $Id$
#
# testing functions for pydude
import pydude
import readline, rlcompleter
readline.parse_and_bind("tab:complete")
import atexit, os


def do_exit(*argv,**argd):
    global ad
    ad.close()
    if os.name != 'posix':
        raw_input("Type Enter to Exit...")

atexit.register(do_exit)

adconf = "/mnt/net/axel/1/scratch/sw/Embedded/Savannah/uracoli-appnotes/pydude/avrdude-5.5/avrdude.conf"
pydude.read_config(adconf)



ad = pydude.AvrDude()
ad.set_programmer("jtag2")
ad.set_part("ATMEGA1281")
ad.open('usb')



sig = ad.part_check_signature()

#==============================
# invert first 8 eeprom bytes
# x = ad.part_read("ee",0,8)
# y = map(lambda s: ~s,x)
# ad.part_write("ee",0,y)
#==============================


