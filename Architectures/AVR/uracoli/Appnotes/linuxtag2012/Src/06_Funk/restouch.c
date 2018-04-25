/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "funk.h"

/* === macros ============================================================== */
#define NSAMPLES (16) /* must match the inline asm below! */
#define NSAMPLE  (2)
#define KEY_NONE (255)
#define SCAN_LOWER (2)
#define SCAN_UPPER (6)
#define SCAN_THRS_RELEASE (SCAN_LOWER + 1)
#define SCAN_THRS_PRESS   (SCAN_UPPER - 1)

/* === globals ============================================================= */
static uint8_t PadState[9] = { 0 };

/* === functions =========================================================== */

uint8_t update_pads(uint8_t row)
{
    static uint8_t scans[9];

    uint8_t portb, *pscan, *pstate, i, pind, ret, keyidx;

    ret = KEY_NONE;
    portb = PORTB;
    PORTB = 0;
    DDRB  = ROW_IO_MASK;
    DDRD = COL_PADS;
    _delay_us(20);
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
    _delay_us(80);
    pind = PIND;
    PORTB = portb;
    keyidx = row*3;
    pscan = &scans[keyidx];

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
        }

        pscan ++;
        pstate ++;
        keyidx ++;
    }

    return ret;
}
