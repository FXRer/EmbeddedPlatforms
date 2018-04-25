# $Id$
# Package Builder Script
REV=1.1
URACOLI_BASE=$(readlink -f ../../../uracoli)
PKG=uracoli-sensorlogger-${REV}

function do_exit
{
    ret=$1
    shift
    echo $ret, $*
    exit $ret
}

# build the needed scons stuff.
scons -C ${URACOLI_BASE} \
        install/src \
        install/wuart \
        install/inc \
        muse231 \
        || do_exit 10 "scons failed"

test -d "${PKG}" && rm -rf "${PKG}"
test -f "${PKG}.zip" && rm -rf "${PKG}.zip"

# create directories
for d in ${PKG}/sensor ${PKG}/pc_apps;
do
    test -d $d || mkdir -p $d
    test -d $d || do_exit 20 "Failed to create $d"
done

# copy local sources
sed  's/^URACOLIDIR.*/URACOLIDIR=../g;s/-pedantic//g'  \
        Sensor/Makefile  > ${PKG}/sensor/Makefile \
        || do_exit 30 "Failed to convert Sensor/Makefile"
cp -rfv Sensor/*.[hc] ${PKG}/sensor \
        || do_exit 31 "Failed to copy Sensor/*.[hc]"
cp -rfv PcApp/sensorlogger.py \
        PcApp/sensorserver.py \
        PcApp/tableserver.py \
        PcApp/utils.py \
        PcApp/sensor.cfg \
        ${PKG}/pc_apps \
        || do_exit 32 "Failed to copy PcApp/*.py"
cp -rfv Sensor/web*.py ${PKG}/pc_apps  \
        || do_exit 33 "Failed to copy Sensor/*.py"
cp -rfv ${URACOLI_BASE}/install/bin/wibo_muse231.py \
        ${PKG}/pc_apps \
        || do_exit 34 "Failed to copy ${URACOLI_BASE}/install/bin/wibo_muse231.py"

# copy uracoli source code
cp -rfv ${URACOLI_BASE}/LICENSE ${PKG}/ \
        || do_exit 40 "Failed to copy LICENSE"
cp -rfv ${URACOLI_BASE}/install/src ${PKG}/uracoli-src \
        || do_exit 41 "Failed to copy uracoli-src"
cp -rfv ${URACOLI_BASE}/install/inc ${PKG}/inc \
        || do_exit 42 "Failed to copy inc"
cp -rfv ${URACOLI_BASE}/install/wuart ${PKG}/wuart \
        || do_exit 43 "Failed to copy wuart"

# build libraries, apps and doc
make -C ${PKG}/uracoli-src tiny230 muse231 radiofaro psk230 stb230 \
        || do_exit 50 "Failed to build libs"
make -C ${PKG}/sensor BINDIR=../bin tiny230 muse231 \
        || do_exit 51 "Failed to build sensor"
make -C ${PKG}/wuart BINDIR=../bin radiofaro psk230 stb230 \
        || do_exit 52 "Failed to build wuart"
asciidoc -o ${PKG}/index.html README.ger \
        || do_exit 53 "Failed to build index.html"

# clean tempfiles
rm -rf ${PKG}/wuart/obj \
        || do_exit 61 "Failed to remove wuart/obj"
rm -rf ${PKG}/sensor/obj \
        || do_exit 62 "Failed to remove sensor/obj"
rm -rf ${PKG}/uracoli-src/obj \
        || do_exit 63 "Failed to remove uracoli-src/obj"
rm -rf ${PKG}/bin/*.elf \
        || do_exit 64 "Failed to remove bin/*.elf"
avr-strip -g ${PKG}/lib/*.a \
        || do_exit 65 "Failed to strip libs"

# build zipfile
zip  -r ${PKG}.zip ${PKG} \
        || do_exit 70 "Failed to create ${PKG}.zip"
