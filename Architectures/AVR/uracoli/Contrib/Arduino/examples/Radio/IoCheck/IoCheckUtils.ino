/* $Id$ */
/**
 * I/O Check Utilities
 */
#include "HardwareRadio.h"

extern bool DO_ECHO;

// blocking read for line from serial
char * readline(char * buf, int maxsize)
{
    int idx = 0;
    char b;
    char * rv = NULL;
    buf[0] = 0;
    while(idx < maxsize)
    {
        if (Serial.available())
        {
            b = Serial.read();
            if(b == '\n' || b == '\r')
            {
                buf[idx++] = 0;
                rv = buf;
                break;
            }
            else
            {
                buf[idx++] = b;
            }
        }
    }
    return rv;
}

void execute_pin_command(char **argv, int argc)
{
    int pin_start, pin_end, pin_dir, pin_port, pin_pin, i;
    bool do_analog_read = false;
    if (argc < 3)
    {
        Serial.println("err: invalid number of arguments");
    }
    else
    {
        if(strcasecmp("all", argv[0]) == 0)
        {
            pin_start = 0;
            pin_end = NUM_DIGITAL_PINS;
        }
        else if (argv[0][0] == 'a' || argv[0][0] == 'A')
        {
            // looks stupid but there is no table for Ax pins,
            // just const decalrations.
            switch(argv[0][1])
            {
                #if NUM_ANALOG_INPUTS > 0
                case '0':
                    pin_start = pin_end = A0;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 1
                case '1':
                    pin_start = pin_end = A1;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 2
                case '2':
                    pin_start = pin_end = A2;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 3
                case '3':
                    pin_start = pin_end = A3;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 4
                case '4':
                    pin_start = pin_end = A4;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 5
                case '5':
                    pin_start = pin_end = A5;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 6
                case '6':
                    pin_start = pin_end = A6;
                    break;
                #endif
                #if NUM_ANALOG_INPUTS > 7
                case '7':
                    pin_start = pin_end = A7;
                    break;
                #endif
                default:
                    Serial.println("analog pin not supported");
                    return;
            }
            do_analog_read = true;
            pin_end ++;
        }
        else
        {
            pin_start = pin_end = atoi(&argv[0][1]);
            pin_end ++;
        }
        pin_dir = atoi(argv[1]);
        pin_port = atoi(argv[2]);
        #if 0
        Serial.print("start: ");
        Serial.print(pin_start);
        Serial.print("end: ");
        Serial.print(pin_end);
        Serial.print("dir: ");
        Serial.print(pin_dir);
        Serial.print("port: ");
        Serial.println(pin_port);
        #endif
        for(i = pin_start; i < pin_end; i++)
        {
            pinMode(i, pin_dir);
            digitalWrite(i, pin_port);
            delay(1);
            pin_pin = do_analog_read ? analogRead(i) : digitalRead(i);
            if (DO_ECHO)
            {
                Serial.print("d");
                Serial.print(i);
                Serial.print(" ");
            }
            Serial.println(pin_pin);
        }
    }
}

void execute_set_jtag(bool val)
{
    if( val )
    {
        MCUCR &= ~(1<<JTD);
        MCUCR &= ~(1<<JTD);
        Serial.println("jtag is now ON");
    }
    else
    {
        MCUCR |= (1<<JTD);
        MCUCR |= (1<<JTD);
        Serial.println("jtag is now OFF");
    }
}
void execute_set_sleep(void)
{
    Serial.println("entering sleep");
    delay(100);
    /* need to be implemented in HardwareRadio */
    radio_set_state(STATE_SLEEP);
    //TRXPR = 1 << SLPTR; // sent transceiver to sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // select power down mode
    sleep_enable();
    sleep_cpu(); // go to deep sleep
    sleep_disable(); // executed after wake-up
}

void execute_transmit(void)
{
    int i;
    Serial.print("tx: ");
    Radio.flush();
    uint8_t d[] = {1,2,3,4,5,6,7,8,9};
    for (i = 0; i < (sizeof(d) - 1); i++)
    {
        Serial.print(d[i], HEX);
        Serial.print(", ");
    }
    Serial.println(d[i], HEX);
    Radio.sendto(0xffff, d, sizeof(d));
}

void execute_receive(int tmo)
{
    int t1 = millis(), t2, i = 0, inwaiting = 0;
    Serial.print("rx: ");
    do
    {
        inwaiting = Radio.available();
        if(inwaiting)
        {
            for(i = 0; i < (inwaiting-1) ; i++)
            {
                Serial.print(Radio.read(), HEX);
                Serial.print(", ");
            }
            Serial.println(Radio.read(), HEX);
            break;
        }
        delay(20);
        t2 = millis();
    }
    while((t2 - t1) < tmo);

    if (i == 0)
    {
        Serial.println("error, no frame received");
    }
}
