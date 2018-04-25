#include <avr/io.h>
#include <util/delay.h>

#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define OFF (0)
#define RED (1)
#define GREEN (2)

#define DISPLAY_RED   (0)
#define DISPLAY_GREEN (3)


static uint8_t LedState[9] = { RED,   GREEN, RED,
                               RED,   OFF,   GREEN,
                               GREEN, GREEN, RED};


int main(void)
{
    uint8_t display_state = DISPLAY_GREEN;
    uint8_t i, portb, *pled, row = 0;

    static uint8_t portb_row[6] = {_BV(3), /* rot */
                                 _BV(4),
                                 _BV(5),
                                 _BV(4)+_BV(5), /* gruen */
                                 _BV(3)+_BV(5),
                                 _BV(3)+_BV(4)};
    while(1)
    {
        portb = portb_row[display_state + row];
        pled = &LedState[3*row];

        for(i = 0; i < 3; i++)
        {
            if (display_state == DISPLAY_RED)
            {
                portb |= (*pled == RED) ? 0 : _BV(i);
            }
            else
            {
                portb |= (*pled == GREEN) ? _BV(i) : 0;
            }
            pled ++;
        }

        DDRB = 0;
        PORTB = portb;
        DDRB = (DDRB & ~ROW_IO_MASK) | _BV(row+3) | COL_LEDS;

        if (row == 2)
        {
            display_state = display_state ? DISPLAY_RED : DISPLAY_GREEN;
        }

        row ++;
        if (row > 2)
        {
            row = 0;
        }

        _delay_ms(10);
    }

    return 0;
}
