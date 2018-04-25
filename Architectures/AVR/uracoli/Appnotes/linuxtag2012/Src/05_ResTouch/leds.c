#include <avr/io.h>
#include <avr/interrupt.h>
#include "restouch.h"
extern uint8_t LedState[9];


void io_init(void)
{
    /*
     * Timer 0 with prescaler 8, and overflow interrupts enabled.
     * At F_CPU = 1 MHz, the timer rolls over at 488 Hz, or
     * approximately each 2 ms.
     */
    TCCR0B = _BV(CS01);
    TIMSK0 = _BV(TOIE0);
    OCR0A = 220;
    sei();
}

void display_leds(uint8_t row)
{
    static uint8_t display_state = DISPLAY_GREEN;
    static uint8_t pat_row[6] = {
                                 _BV(3), /* rot */
                                 _BV(4),
                                 _BV(5),
                                 _BV(4)+_BV(5), /* gruen */
                                 _BV(3)+_BV(5),
                                 _BV(3)+_BV(4)
                                 };

    uint8_t i, rowval, portb, *pled;

    rowval = pat_row[display_state + row];
    portb = rowval;
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
    DDRB = (DDRB & ~ROW_IO_MASK) | _BV(row+3) | 7;

    if (row == 2)
    {
        display_state = display_state ? DISPLAY_RED : DISPLAY_GREEN;
    }
}

