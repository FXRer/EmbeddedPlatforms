/* $Id$ */
/**
 * I/O Check sketch
 */
#include "HardwareRadio.h"

bool DO_ECHO = false;

void setup()
{

    Serial.begin(57600);
    // flush input
    while(Serial.available())
    {
        Serial.read();
    }
    Radio.begin();
    Serial.println("Sketch IoCheck\n\r"
                   "Type help for a list of commands");
}

void loop()
{
    static uint8_t val = 0;
    static char line[32], *pcmd;

    if (DO_ECHO)
    {
        Serial.print("\n\rIoCheck>");
    }
    /* check serial IO */
    pcmd = readline(line, sizeof(line));

    if(pcmd != NULL)
    {
        if (DO_ECHO)
        {
            Serial.println(pcmd);
        }
        process_command(pcmd);
    }
}

// =============================================================
// parse command and dispatch to execution function
void process_command(char *pcmd)
{
    char *argv[8], *p;
    int argc = 0, i;
    bool newarg;

    p = pcmd;
    argv[argc++] = p;
    newarg = false;

    while(*p != 0)
    {
        if (*p == ' ')
        {
            *p = 0;
            newarg = true;
        }
        else
        {
            if (newarg == true && argc < 8)
            {
                argv[argc++] = p;
                newarg = false;
            }
        }
        p++;
    }
    #if 0
    // dump result of line parsing
    for (i=0; i<argc; i++)
    {
        Serial.print(" - ");
        Serial.print(i);
        Serial.print(":");
        Serial.println(argv[i]);
    }
    #endif

    if (strncasecmp("pin", argv[0], 3) == 0)
    {
        execute_pin_command(argv + 1, argc - 1);
    }
    else if (strncasecmp("help", argv[0], 4) == 0)
    {
        execute_print_help();
    }
    else if (strncasecmp("echo", argv[0], 4) == 0)
    {
        DO_ECHO = atoi(argv[1]) ? true : false;
        Serial.print("echo is now ");
        Serial.println(DO_ECHO ? "ON": "OFF");
    }
    else if (strncasecmp("jtag", argv[0], 4) == 0)
    {
        execute_set_jtag(atoi(argv[1]) ? true : false);
    }
    else if (strncasecmp("sleep", argv[0], 4) == 0)
    {
        execute_set_sleep();
    }
    else if (strncasecmp("tx", argv[0], 2) == 0)
    {
        execute_transmit();
    }
    else if (strncasecmp("rx", argv[0], 2) == 0)
    {
        execute_receive(atoi(argv[1]));
    }
}

void execute_print_help(void)
{
    Serial.print("\n\r"
        " pin <id> <dir> <val> : set pin (id is e.g. 'd0' or 'a0', dir: 0=input, 1=output)\n\r"
        " echo <bool>          : switch echo ON/OFF\n\r"
        " jtag <bool>          : switch jtag ON/OFF\n\r"
        " sleep                : set MCU and TRX to sleep\n\r"
        " tx                   : transmit frame\n\r"
        " rx <tmo>             : receive frame (tmo to wait for frame in ms)\n\r"
        " help                 : print help\n\r");
}
