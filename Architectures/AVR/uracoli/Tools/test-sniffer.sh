rm -rf test/uracoli-sniffer* install/uracoli-sniffer*
scons psniffer version=tst
unzip -d test install/uracoli-sniffer-tst.zip
