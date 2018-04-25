function error {
    err=$1
    shift
    echo "==========================================="
    echo "Error[$err] $*"
    echo "==========================================="

    exit $err
}

avr-gcc -g -mmcu=atmega128rfa1 hello.c ||\
     error 1 "avr-gcc failed"
avr-objcopy -O ihex a.out a.hex ||\
     error 2 "avr-objcopy failed"
avrdude -P usb -p atmega128rfa1 -c dragon_jtag -U fl:w:a.hex:i ||\
     error 3 "avrdude failed"
#warten bis USB wieder frei ist.
sleep 2
avarice -I -P atmega128rfa1 -2g --detach :4242 ||\
     error 4 "avarice failed"
avr-gdb -x ../debug.cfg a.out


