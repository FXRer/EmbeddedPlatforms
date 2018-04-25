:author: Daniel Thiele, Axel Wachtler
:lang: de
:toc: 1
= Software-Paket zum Artikel in EPJ14, S.22 =

Dieses Paket enthält die Software zum Artikel _Funkübertragung mit IEEE 802.15.4
Transceivern - nicht nur für Profis_ im Embedded Projects Journal Nr. 14, S. 22.
Das Heft steht als PDF zum
https://journal.embedded-projects.net/index.php?module=archiv&action=pdf&id=16[Download]
bereit.


== Schritt 1: Software Installation ==

Folgende Pakete werden benötigt:

[horizontal]
uracoli-epj00-1.0.zip::
    der Qelltext des Temperaturloggers.
AVR-Toolchain::
    avr-gcc und avr-binutils mit Unterstützung für den Controller ATmega128RFA1.
Atmel-Studio oder avrdude::
    zum Flashen der Firmware
Python 2.x + pyserial::
    für den Webserver.


__Anders als im Artikel beschrieben, ist für das Demoprojekt nur das Paket
+uracoli-epj00-1.0.zip+ erforderlich. Es enthält alle benötigten Quellen.__

Zunächst wird das Paket entpackt, dabei entsteht das Verzeichnis +uracoli-epj00-1.0+.

--------------------------------------------------------------------------------
$ wget http://download.savannah.nongnu.org/releases/uracoli/uracoli-epj00-1.0.zip
$ unzip uracoli-epj00-1.0.zip
--------------------------------------------------------------------------------

Nach dem Entpacken befindet sich die folgende Verzeichnisstruktur auf der
Festplatte.

--------------------------------------------------------------------------------
uracoli-epj00-1.0
|-- inc .................. µracoli Include-Dateien
|   `-- boards
|-- src .................. Quellen + Makefile der Library
|   |-- libioutil
|   `-- libradio
|-- tlogger .............. Quellen + Makefile des Temperaturloggers
|                          und die Script webserver.py und nodeaddr.py
`-- wuart ................ Quellen + Makefile des Wireless UART
--------------------------------------------------------------------------------

== Schritt 2: Firmware des Gateway-Knotens ==

Auf dem Gateway-Knoten wird die Wireless-UART-Firmware installiert. Die Firmware
wird mit den folgenden Kommandos compiliert und es ensteht das File
+bin/wuart_radiofaro.hex+:

--------------------------------------------------------------------------------
$ make -C src/ radiofaro
$ make -C wuart/ radiofaro
--------------------------------------------------------------------------------

Um nun die Parameter der Funkschnittstelle dauerhaft im Flash des Controllers
zu speichern, wird das Script +nodeaddr.py+ verwendet, mit dem der
Konfigurationsdatensatz am Ende des Flash-Speichers erzeugt wird. Das
modifizierte HEX-File wird danach auf den Controller geflasht.

--------------------------------------------------------------------------------
$ python tlogger/nodeaddr.py -B radiofaro \
                             -p 0x1234 -a 0 \
                             -c 17 \
                             -f bin/wuart_radiofaro.hex \
                             -o w.hex

$ avrdude -P usb -p m128rfa1 -c jtag2 -U w.hex
--------------------------------------------------------------------------------

Als Gateway-Knoten kann auch jedes andere der von µracoli unterstützte Board
verwendet werden. Ein Liste der Boards erhält man mit

--------------------------------------------------------------------------------
$ make -C wuart/ list
--------------------------------------------------------------------------------

Soll die WUART-Firmware z.B. für den RZUSB-Stick von Atmel überstetzt werden,
sind folgende Kommandos erforderlich:

--------------------------------------------------------------------------------
$ make -C src/ rzusb
$ make -C wuart/ rzusb
$ python tlogger/nodeaddr.py -B rzusb \
                             -p 0x1234 -a 0 \
                             -c 17 \
                             -f bin/wuart_rzusb.hex \
                             -o w.hex
$ avrdude -P usb -p usb1287 -c jtag2 -U w.hex
--------------------------------------------------------------------------------


== Schritt 3: Firmware des Sensor-Knotens ==

Die Firmware für die Sensorknoten wird analog wie die Gateway-Firmware erzeugt.
Dabei ist zu beachten, dass die mit dem Parameter +-a+ angegebene Adresse für
jeden Knoten vor dem Flashen zu ändern ist.

--------------------------------------------------------------------------------
$ make -C src radiofaro
$ make -C tlogger radiofaro

# Flash node #1
$ python tlogger/nodeaddr.py -B radiofaro \
                                  -p 0x1234 \
                                  -a 1 \
                                  -c 17 \
                                  -f bin/tlogger_radiofaro.hex
                                  -o t.hex

$ avrdude -P usb -p m128rfa1 -c jtag2 -U t.hex

# Flash node #2
$ python tlogger/nodeaddr.py -B radiofaro \
                                  -p 0x1234 \
                                  -a 2 \
                                  ....
$ avrdude -P usb -p m128rfa1 -c jtag2 -U t.hex

# Flash node ......
--------------------------------------------------------------------------------

Ähnlich wie schon zuvor bei der der WUART-Firmware kann der Temperaturlogger
auch auf anderen Boards mit ATmega128RFA1 installiert werden. Die
unterstützten Boards werden mit dem folgendem Befehl angezeigt:

--------------------------------------------------------------------------------
$ make -C tlogger list
--------------------------------------------------------------------------------

Die Erzeugung der Firmware für das +xxo+ Board sieht dann wie folgt aus:

--------------------------------------------------------------------------------
$ make -C src xxo
$ make -C tlogger xxo
$ python tlogger/nodeaddr.py -B xxo \
                                  -p 0x1234 \
                                  -a 1 \
                                  -c 17 \
                                  -f bin/tlogger_xxo.hex
                                  -o t.hex
$ avrdude -P usb -p m128rfa1 -c jtag2 -U t.hex
$ ...
--------------------------------------------------------------------------------

== Schritt 4: Test der Funkkomunikation ==

Der Gateway-Knoten wird an den PC angeschlossen und der virtuelle COM-Port
in einem Terminal-Programm geöffnet. Die Portparameter sind 38400 8N1, ohne
Handshake. Danach sollte im Terminal aller 1s die folgenden Ausschriften zu
lesen sein:

----------------------------
Wuart 0.3 chan=17 radio 83.04 cfg F
ADDR=0002, T=19.9, V=2.63
ADDR=0002, T=20.3, V=2.63
ADDR=0002, T=20.1, V=2.63
ADDR=0002, T=20.1, V=2.63
ADDR=0002, T=19.9, V=2.63
ADDR=0002, T=19.9, V=2.63
----------------------------

== Schritt 5: Test des Webservers ==

Das Python-Script +tlogger/webserver.py+ implementiert einen Webserver, der
die Sensordaten-Daten von der seriellen Schnittstelle des Gateway-Knotens
liest und als Webseite auf dem entsprechenden HTTP Port ausgibt. Man kann die
Webseite unter Angabe der URL http://localhost:8081 im Web-Browser anzeigen.

Das Script lädt seine Parameter aus der Config-Datei +webserver.cfg+, die an
die lokalen Gegebenheiten anzupassen ist.

--------------------------------------------------------------------------------
[settings]
http_port = 8080                          <1>
http_port_max = 8086

serial_port = /dev/ttyUSB0                <2>
baudrate = 38400

html_header = ...                         <3>
html_footer = ...
html_error_404 = ...

table_header = ...
table_row_std = ...
table_row_tmo = ...
table_row_err = ...
table_footer = ...

[labels]                                  <4>
0x0001 = Labor
0x0002 = Server-Raum
--------------------------------------------------------------------------------
<1> Die Parameter +http_port+ und +http_port_max+ geben an, welchen Port der
    Webserver verwendet. Sollte der Port 8080 bereits durch einen anderen
    Dienst verwendet sein, wird versucht den Webserver auf Port 8081 zu starten,
    u.s.w.
<2> Der Parameter +serial_port+ gibt den Namen der seriellen Schnittstelle des
    Gateway-Knotens an (unter Windows +COMx+). Mit dem  +baudrate+ wird die
    Übertragungsgeschwindigkeit eingestellt.
<3> Die Parameter +html_header+, +html_footer+, +html_error_404+, +table_header+,
    +table_row_{std|tmo|err}+, +table_footer+ beinhalten HTML-Templates, die der
    Webserver zum Erzeugen der Seite verwendet.
<4> Im Abschnitt +labels+ der Config-Datei werden den Adressen der Sensorknoten
    symbolische Namen zugeordnet.


Der Start erfolgt einfach mit dem Befehl +python webserver.py+:

--------------------------------------------------------------------------------
$python webserver.py
reading config file webserver.cfg
processing config data
open serial port /dev/ttyUSB0:38400
 - failed to open port 8080
HTTP server running on port 8081
--------------------------------------------------------------------------------

Alternativ kann auch ein anderer Dateiname für das Config-File verwendet werden.
Das Python-Script wird dann mit
--------------------------------------------------------------------------------
python webserver.py -C myfile.cfg
--------------------------------------------------------------------------------
aufgerufen.

== Zusammenfassung ==

Mit dem vorliegenden Software-Paket kann eine Umgebung zur webgestützten Anzeige
von Sensordaten aufgebaut werden. Die Verwendung des ATmega128RFA1 als drahtloses
Funkthermometer ist dabei nur eines von vielen möglichen Anwendungsbeispielen.

Wenn es Fragen zum Paket gibt, können diese im Thread zum Artikel unter
http://www.mikrocontroller.net/topic/epj-14-seite-22 gestellt werden.

Viel Spass beim Experimentieren wünscht das µracoli-Team.
