/* Copyright (c) 2012 Axel Wachtler, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */

/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "game.h"

/* === macros ============================================================== */
#define KEY_NONE (255)
#define SCAN_LOWER (2)
#define SCAN_UPPER (6)
#define SCAN_THRS_RELEASE (SCAN_LOWER + 1)
#define SCAN_THRS_PRESS   (SCAN_UPPER - 1)

/* === types =============================================================== */

/* === globals ============================================================= */
static uint8_t PadScans[9] = { 0 };
static uint8_t PadState[9] = { 0 };
static volatile uint8_t KeyEvent;

/* === prototypes ========================================================== */

/* === functions =========================================================== */


void xxo_scan_pads(uint8_t row)
{
    uint8_t portb, *pscan, *pstate, i, pind, keyidx;

    KeyEvent = KEY_NONE;
    portb = PORTB;
    PORTB = 0;
    DDRB  = ROW_IO_MASK;
    DDRD = COL_PADS;
    _delay_us(20);
    DDRD = 0x0;
    switch(row)
    {
        /* because of */
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
    pscan = &PadScans[keyidx];

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
            KeyEvent = keyidx;
        }

        pscan ++;
        pstate ++;
        keyidx ++;
    }
}

uint8_t xxo_get_key(void)
{
uint8_t ret;
    cli();
    ret = KeyEvent;
    KeyEvent = KEY_NONE;
    sei();
    return ret;
}
