#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define OFF (0)
#define GREEN (1)
#define RED (2)

#define DISPLAY_RED (0)
#define DISPLAY_GREEN (3)

static uint8_t LedState[9] = { RED,   GREEN, RED,
                               RED,   OFF,   GREEN,
                               GREEN, GREEN, RED};

static void io_init(void)
{
    /*
     * Timer 0 with prescaler 8, and overflow interrupts enabled.
     * This is the basic "housekeeping" interrupt which manages the
     * LED multiplexing as well as the input pad scanning.
     *
     * At F_CPU = 1 MHz, the timer rolls over at 488 Hz, or
     * approximately each 2 ms.
     *
     * In the resistive touchpad version, the pads are being scanned
     * some time later after the respective row output has been
     * activated; the actual amount of time is determined by the
     * OCR0A value.
     */
    TCCR0B = _BV(CS01);
    TIMSK0 = _BV(TOIE0);
    OCR0A = 220;
    sei();
}

static void display_leds(uint8_t row)
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

ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;

    display_leds(row);

    row ++;
    if (row > 2)
    {
        row = 0;
    }
}

int main(void)
{

    io_init();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while(1)
    {
        sleep_mode();
    }

    return 0;
}
