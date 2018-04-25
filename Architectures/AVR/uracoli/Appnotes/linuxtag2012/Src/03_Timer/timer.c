#include <avr/io.h>
#include <avr/interrupt.h>

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
    /*
     * Aufgabe 1:
     *  Setze einen Breakpoint in dieser Funktion
     *  Wird sie aufgerufen ?
     *
     * Aufgabe 2:
     *  Implementiere diese Funktion, nach dem
     *  Beispiel ../02_Leds/leds_ref.c
     */
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
    while(1)
    {
    }

    return 0;
}
