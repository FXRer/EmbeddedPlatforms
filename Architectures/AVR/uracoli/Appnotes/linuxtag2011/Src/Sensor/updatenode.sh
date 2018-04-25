BOARD=muse231
DUDEOPTS="-P usb -p m88 -c avrisp2 -B 10 -F"

python wibo_$BOARD.py -f bin/sensorapp_$BOARD.hex $* |\
     avrdude $DUDEOPTS -U flash:w:-:i

python wibo_$BOARD.py -O 0 $*  |\
     avrdude $DUDEOPTS -U eeprom:w:-:i
