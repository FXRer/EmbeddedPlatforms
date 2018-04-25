/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "restouch.h"

/* === macros ============================================================== */
#define NSAMPLES (16) /* must match the inline asm below! */
#define NSAMPLE  (2)
#define KEY_NONE (255)
#define SCAN_LOWER (2)
#define SCAN_UPPER (6)
#define SCAN_THRS_RELEASE (SCAN_LOWER + 1)
#define SCAN_THRS_PRESS   (SCAN_UPPER - 1)

#define DISCHARGE_TIME   (30) /* time in us, to uncharge C_in */
#define CHARGE_TIME      (60) /* time in us, to charge C_in */

/* === globals ============================================================= */
uint8_t LedState[9] = { RED,   GREEN, RED,
                        RED,   OFF,   GREEN,
                        GREEN, GREEN, RED};

static uint8_t PadState[9] = { '_', '_', '_',
                               '_', '_', '_',
                               '_', '_', '_'  };

/* === functions =========================================================== */


uint8_t update_pads(uint8_t row)
{
    static uint8_t scans[9] = { SCAN_LOWER, SCAN_LOWER, SCAN_LOWER,
                                SCAN_LOWER, SCAN_LOWER, SCAN_LOWER,
                                SCAN_LOWER, SCAN_LOWER, SCAN_LOWER };

    uint8_t portb, *pscan, *pstate, i, pind, ret, keyidx;

    ret = KEY_NONE;
    portb = PORTB;

    /* discharge C_in */
    PORTB = 0;
    DDRB  = ROW_IO_MASK;
    DDRD = COL_PADS;
    _delay_us(DISCHARGE_TIME);

    /* charge C_in */
    DDRD = 0x0;
    switch(row)
    {
        case 0:
            PORTB = _BV(PB3);
            break;
        case 1:
            PORTB = _BV(PB4);
            break;
        case 2:
            PORTB = _BV(PB5);
            break;
    }
    _delay_us(CHARGE_TIME);
    pind = PIND;
    PORTB = portb;
    keyidx = row*3;
    pscan = &scans[keyidx];

    /* accumulate charge values */
    pstate = &PadState[keyidx];
    for (i = 0; i < 3; i++)
    {
        *pscan += (pind & _BV(i+5)) ? +1 : -1;
        if (*pscan < SCAN_LOWER)
        {
            *pscan = SCAN_LOWER;
        }
        if (*pscan > SCAN_UPPER)
        {
            *pscan = SCAN_UPPER;
        }

        /* detect key events */
        if ((*pscan < SCAN_THRS_RELEASE) && (*pstate != '_'))
        {
            *pstate = '_';
        }


        if ((*pscan > SCAN_THRS_PRESS) && (*pstate != '#'))
        {
            *pstate = '#';
            ret = keyidx;
            __asm("nop");
        }

        pscan ++;
        pstate ++;
        keyidx ++;
    }

    return ret;
}

ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;
    uint8_t key;

    key = update_pads(row);

    if (key != KEY_NONE)
    {
        LedState[key] ++;
        if(LedState[key] > 2)
        {
            LedState[key] = 0;
        }
    }

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
