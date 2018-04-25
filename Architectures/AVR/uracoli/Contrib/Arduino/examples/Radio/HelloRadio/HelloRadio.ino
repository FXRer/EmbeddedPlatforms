/* $Id$ */
/**
 * Use of the radio functions
 */
#include "HardwareRadio.h"

#define REG(name, reg) do{uint8_t _b_ = reg; Serial.print(name);Serial.print(" : "); Serial.println(_b_,HEX);}while(0)

unsigned long tx_time;
int cnt = 0;
void setup() {

    /* operating on channel 17, not receiving when idle. */
    Radio.begin(17, STATE_OFF);
    Serial.begin(57600);
    Serial.println("HelloRadio V$Release$");
}

void loop() {

    if (millis() > tx_time){
        tx_time = millis() + 500;
        // write string
        Radio.write("cnt: ");
        // write integer
        Radio.print(cnt);
        // write byte
        Radio.write('\n');
        Radio.flush();
        Serial.println(cnt);
        cnt ++;
    }

}


