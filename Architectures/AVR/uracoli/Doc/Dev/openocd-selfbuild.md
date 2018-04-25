# Openocd self compiling on Ubuntu {#pgOpenocd_selfbuild}

## Links

- https://github.com/RIOT-OS/RIOT/wiki/OpenOCD
- http://jjmilburn.github.io/2014/09/18/Atmel-SAMD20-EclipseCDT/
- http://karibe.co.ke/2013/08/setting-up-linux-opensource-build-and-debug-tools-for-freescale-freedom-board-frdm-kl25z/

## Commands

    dmesg
    sudo apt-get install build-essential autoconf automake libtool libusb-dev libusb-1.0-0-dev libhidapi-dev
    sudo apt-get install build-essential autoconf automake libtool libusb-dev libusb-1.0-0-dev
    sudo apt-get install libudev-dev
    cd ..
    git clone git://github.com/signal11/hidapi.git
    mv hidapi hidapi-git-anon
    cd hidapi-git-anon/
    ./bootstrap
    ./configure
    make
    sudo make  install
    wget http://sourceforge.net/projects/openocd/files/openocd/0.9.0/openocd-0.9.0.zip
    unzip  openocd-0.9.0.zip
    cd openocd-0.9.0/
    ./configure --prefix=/opt/openocd-0.9.0
    make
    sudo make install
    /opt/openocd-0.9.0/bin/openocd
    sudo ln -s /usr/local/lib/libhidapi-hidraw.so.0 /usr/lib/libhidapi-hidraw.so.0
    /opt/openocd-0.9.0/bin/openocd
    sudo cp contrib/99-openocd.rules /etc/udev/rules.d/99-openocd.rules
    sudo udevadm control --reload-rules
    /opt/openocd-0.9.0/bin/openocd -f /./opt/openocd-0.9.0/share/openocd/scripts/board/atmel_samr21_xplained_pro.cfg
