/* $Id$ */
/**
 * Use of the radio functions
 */

#include <stdio.h>
#include "HardwareRadio.h"

#define REG(name, reg) do{uint8_t _b_ = reg; Serial.print(name);Serial.print(" : "); Serial.println(_b_,HEX);}while(0)

unsigned long tx_time;
int cnt = 0;
void setup() {

    /* operating on channel 17, not receiving when idle. */
    Radio.begin(17, STATE_OFF);
    Serial.begin(57600);
    Serial.println("RadioIo");
}

void loop() {
int a0, a1;
char buf[64];

    if (millis() > tx_time)
    {
       a0 = analogRead(0);
       a1 = analogRead(1);
       snprintf(buf, 64, " a0: %d, a1: %d\r\n", a0, a1);

       // sent wireless
       Radio.write("cnt: ");
       Radio.print(cnt);
       Radio.write(buf);
       Radio.flush();

       // local echo of results
       Serial.print("cnt:");
       Serial.print(cnt);
       Serial.print(buf);

       tx_time = millis() + 500;
       cnt ++;
    }

}


