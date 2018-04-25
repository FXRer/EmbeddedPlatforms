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
#include <string.h>
#include <avr/io.h>
#include "game.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
volatile uint8_t LedState[NB_FIELDS] = {0,0,0, 0,0,0, 0,0,0};

/* === prototypes ========================================================== */

/* === functions =========================================================== */
void xxo_leds_init(void)
{
    memset((void*)LedState, 0, NB_FIELDS);
}

void xxo_leds_display(uint8_t row)
{
    static uint8_t color_state = 0;

    static uint8_t pat_row[6] = {_BV(3),
                                 _BV(4),
                                 _BV(5),
                                 _BV(4)+_BV(5),
                                 _BV(3)+_BV(5),
                                 _BV(3)+_BV(4)};

    uint8_t portb, i, *pled_state, rowval;

    rowval = pat_row[color_state + row];
    portb = rowval;
    pled_state = (uint8_t*)&LedState[3*row];

    for(i = 0; i < 3; i++)
    {
        if (color_state != 0)
        {
            portb |= (*pled_state == 2) ? _BV(i) : 0;
        }
        else
        {
            portb |= (*pled_state == 1) ? 0 : _BV(i);
        }
        pled_state ++;
    }

    PORTB = portb;
    DDRB = (DDRB & ~ROW_IO_MASK) | _BV(row+3) | 7;

    if (row == 2)
    {
        color_state = color_state ? 0 : 3;
    }
}

void xxo_flash_leds(uint8_t n, uint8_t color)
{

    for(uint8_t i = 0; i < n; i++)
    {
        memset((void*)LedState, color, NB_FIELDS);
        xxo_do_sleep(250);
        memset((void*)LedState, 0, NB_FIELDS);
        xxo_do_sleep(250);
    }
}

void xxo_flood_leds(uint8_t color)
{

    memset((void*)LedState, 0, NB_FIELDS);
    for(uint8_t i = 0; i < 9; i++)
    {
        LedState[i] = color;
        xxo_do_sleep(250);
    }
}

void xxo_set_leds(uint8_t idx, uint8_t color)
{
        LedState[idx] = color;
}

void xxo_highlight_winning_pattern(uint8_t n, winner_t *w)
{
    for(uint8_t i = 0; i < n; i++)
    {

        LedState[w->a] ^= w->col;
        LedState[w->b] ^= w->col;
        LedState[w->c] ^= w->col;
        xxo_do_sleep(500);
    }

}
