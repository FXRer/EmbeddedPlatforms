rm -rf install/uracoli-tst-arduino.zip build/arduino
scons parduino version=tst &&\
unzip -o install/uracoli-tst-arduino.zip -d ${HOME}/.arduino/sketchbook/
echo Done installing install/uracoli-tst-arduino.zip in ${HOME}/.arduino/sketchbook/
