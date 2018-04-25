/* RadioFaro wirless/serial bridge. */
#include "HardwareRadio.h"

/* data captured from UART */
char serial_inbyte = 0;

/* data received from Radio */
char serial_outbyte = 0;

/* next time the radio buffer will flushed */
unsigned long tx_time;

void setup() {
    Radio.begin();
    Serial.begin(57600);
    tx_time = 0;
    Serial.println("RadioUart.ino");
}

void loop() {

    if ((Serial.available() > 0))
    {
        serial_inbyte = Serial.read();
        Radio.print(serial_inbyte);
    }

    if (millis() > tx_time){
        tx_time = millis() + 25;
        Radio.flush();
    }

    if (Radio.available() > 0)
    {
        serial_outbyte = Radio.read();
        Serial.print(serial_outbyte);
    }
}
