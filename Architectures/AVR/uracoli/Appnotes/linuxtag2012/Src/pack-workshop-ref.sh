#! /bin/bash
set -e

make -C Doc/ clean
make -C Doc/ workshop.html workshop.pdf

for i in */*.mk;
do

    dn=$(dirname $i);
    fn=$(basename $i);

    echo === $dn $fn ===;
    if [[ -e $dn/${fn%.mk}_ref.c ]] ;
    then
        mv --verbose ${dn}/${fn%.mk}.c ${dn}/${fn%.mk}.c.orig
        cp --verbose $dn/${fn%.mk}_ref.c ${dn}/${fn%.mk}.c
        make -C $dn -f $fn clean all;
        mv --verbose ${dn}/${fn%.mk}.c.orig ${dn}/${fn%.mk}.c
        touch ${dn}/*.out  ${dn}/*.hex
    else
        make -C $dn -f $fn clean all;
    fi
    echo
done

zip -r workshop_ref.zip \
    ./01_HelloWorld/debug.sh \
    ./01_HelloWorld/hello.c \
    ./02_Leds/leds.hex \
    ./02_Leds/led1.c \
    ./02_Leds/led1.mk \
    ./02_Leds/leds.c \
    ./02_Leds/leds.hex \
    ./02_Leds/leds.out \
    ./02_Leds/leds.mk \
    ./02_Leds/leds_ref.c \
    ./03_Timer/timer.c \
    ./03_Timer/timer.mk \
    ./03_Timer/timer.out \
    ./03_Timer/timer_ref.c \
    ./03_Timer/timer.hex \
    ./04_CapTouch/captouch.c \
    ./04_CapTouch/captouch.h \
    ./04_CapTouch/captouch.mk \
    ./04_CapTouch/captouch.out \
    ./04_CapTouch/captouch_ref.c \
    ./04_CapTouch/leds.c \
    ./04_CapTouch/captouch.hex \
    ./05_ResTouch/leds.c \
    ./05_ResTouch/restouch.c \
    ./05_ResTouch/restouch.h \
    ./05_ResTouch/restouch.mk \
    ./05_ResTouch/restouch.out \
    ./05_ResTouch/restouch_ref.c \
    ./05_ResTouch/restouch.hex \
    ./06_Funk/captouch.c \
    ./06_Funk/funk.c \
    ./06_Funk/funk.h \
    ./06_Funk/funk.mk \
    ./06_Funk/leds.c \
    ./06_Funk/restouch.c \
    ./06_Funk/funk_cap.hex \
    ./06_Funk/funk_cap.out \
    ./06_Funk/funk_res.hex \
    ./06_Funk/funk_res.out \
    ./07_Spiel/captouch.c \
    ./07_Spiel/funk.c \
    ./07_Spiel/leds.c \
    ./07_Spiel/restouch.c \
    ./07_Spiel/spiel.c \
    ./07_Spiel/spiel.h \
    ./07_Spiel/spiel.mk \
    ./07_Spiel/spiel_cap.hex \
    ./07_Spiel/spiel_cap.out \
    ./07_Spiel/spiel_res.hex \
    ./07_Spiel/spiel_res.out \
    ./debug.cfg \
    ./setup-sudotools.sh \
    ./Doc/cap_sense.png \
    ./Doc/full_setup.jpg \
    ./Doc/Img/xxo_cap_led4green_nosleep.png \
    ./Doc/Img/xxo_cap_led4green.png \
    ./Doc/Img/xxo_cap_led4green_sleep.png \
    ./Doc/Img/xxo_cap_led4red_nosleep.png \
    ./Doc/Img/xxo_cap_led4red.png \
    ./Doc/Img/xxo_cap_led4red_sleep.png \
    ./Doc/Img/xxo_cap_leds_off_nosleep.png \
    ./Doc/Img/xxo_cap_leds_off.png \
    ./Doc/Img/xxo_cap_leds_off_sleep.png \
    ./Doc/Makefile \
    ./Doc/res_sense.png \
    ./Doc/state.png \
    ./Doc/state.txt \
    ./Doc/workshop__1.png \
    ./Doc/workshop__2.png \
    ./Doc/workshop__3.png \
    ./Doc/workshop__4.png \
    ./Doc/workshop__4.txt \
    ./Doc/workshop.html \
    ./Doc/workshop.pdf \
    ./Doc/workshop.txt

