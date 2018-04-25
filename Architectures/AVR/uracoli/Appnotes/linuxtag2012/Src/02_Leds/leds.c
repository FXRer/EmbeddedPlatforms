#include <avr/io.h>
#include <util/delay.h>

#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define OFF (0)
#define RED (1)
#define GREEN (2)

#define DISPLAY_RED (0)
#define DISPLAY_GREEN (3)


static uint8_t LedState[9] = { RED,   GREEN, RED,
                               RED,   OFF,   GREEN,
                               GREEN, GREEN, RED};

int main(void)
{
    uint8_t display_state = DISPLAY_GREEN;
    uint8_t i, portb, row = 0;

    while(1)
    {

        for(i = 0; i < 3; i++)
        {
            /* berechne die Variable portb für die aktuelle Zeile (row)*/
        }

        DDRB = 0;
        PORTB = portb;
        DDRB = (DDRB & ~ROW_IO_MASK) | _BV(row+3) | 7;

        /*
         * schalte die Variable display_state weiter
         *   DISPLAY_GREEN, DISPLAY_RED, DISPLAY_GREEN, ....
         */

        row ++;
        if (row > 2)
        {
            row = 0;
        }
        _delay_ms(10);
    }

    return 0;
}
