make -C Doc/ clean 
make -C Doc/ workshop.html
test -d Firmware || mkdir Firmware

for i in */*.mk; 
do 
    dn=$(dirname $i); 
    fn=$(basename $i); 
    
    echo === $dn $fn ===; 
    test -d $dn/Firmware || mkdir $dn/Firmware

    if [[ -e $dn/${fn%.mk}_ref.c ]] ;    
    then
        mv --verbose ${dn}/${fn%.mk}.c ${dn}/${fn%.mk}.c.orig
        cp --verbose $dn/${fn%.mk}_ref.c ${dn}/${fn%.mk}.c
        make -C $dn -f $fn OUTDIR=./Firmware clean all;
        mv --verbose ${dn}/${fn%.mk}.c.orig ${dn}/${fn%.mk}.c
        #touch ${dn}/*.out  ${dn}/*.hex
    else
        make -C $dn -f $fn OUTDIR=./Firmware clean all;
    fi
    echo
done

zip -r tic_tac_toe_reloaded-1.1.zip \
    ./01_HelloWorld/debug.sh \
    ./01_HelloWorld/hello.c \
    ./02_Leds/*.{c,mk} \
    ./03_Timer/*.{c,mk} \
    ./04_CapTouch/*.{c,h,mk} \
    ./05_ResTouch/*.{c,h,mk} \
    ./06_Funk/*.{c,h,mk} \
    ./07_Spiel/*.{c,h,mk} \
    ./08_SpielPowerSave/*.{c,h,mk} \
    ./99_Hwtest/*.{c,h} \
    ./99_Hwtest/Makefile \
    ./*/Firmware/*.hex \
    ./debug.cfg \
    ./setup-sudotools.sh \
    ./Doc/*.jpg \
    ./Doc/Img/*.png \
    ./Doc/Makefile \
    ./Doc/state.png \
    ./Doc/state.txt \
    ./Doc/*.png \
    ./Doc/workshop.html \
    ./Doc/workshop.txt
