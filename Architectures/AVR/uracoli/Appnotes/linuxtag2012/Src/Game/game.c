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
/**
 * @file
 * @brief ....
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include "game.h"
/* === macros ============================================================== */
#if defined(CAP)
# define TYPE "CAP"
#elif defined(RES)
# define TYPE "RES"
#endif

/* === types =============================================================== */
typedef enum
{
    IDLE,
    WAIT_MY_KEY,
    WAIT_PEER_KEY
} game_state_t;

/* === globals ============================================================= */
static volatile int16_t SleepCounter;
/* === prototypes ========================================================== */
extern volatile uint8_t LedState[9];

/* === functions =========================================================== */
ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;

    if(SleepCounter > 0)
    {
        SleepCounter --;
    }

    xxo_scan_pads(row);
    xxo_leds_display(row);

    row ++;
    if (row > 2)
    {
        row = 0;
    }
}

void xxo_do_sleep(int16_t sleep_time)
{
    SleepCounter = (sleep_time>>1);
    while (SleepCounter > 0)
    {
        AVR_DO_SLEEP();
    }
}




static void io_init(void)
{
    /*scans
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

    xxo_leds_init();
    xxo_radio_init();
    sei();

    DDRB  = COL_LEDS | ROW_IO_MASK;
    PORTB = COL_LEDS | ROW_IO_MASK;
    PORTB = _BV(PB0);
    DDRD  = 0;
    PORTD = 0;

}

bool verify_pattern(uint8_t a, uint8_t b, uint8_t c, uint8_t col, winner_t *w)
{
bool ret = false;
    if ((LedState[(a)] == (col)) && (LedState[(b)] == (col)) && (LedState[(c)] == (col)))
    {
        w->a = a;
        w->b = b;
        w->c = c;
        w->col = col;
        ret = true;
    }
    return ret;
}

winner_t * check_winner(winner_t *pwinner)
{
winner_t *ret;
uint8_t color, i;
#define CHECK_WINNER(a,b,c,col, pw) verify_pattern(a, b, c, col, pw)

    ret = NULL;
    for (color = 1; color < 3; color ++)
    {
        if (CHECK_WINNER(0, 1, 2, color, pwinner))
        {
            XXO_PRINTF("win 012 col=%d\n\r", color);
            break;
        }
        else if (CHECK_WINNER(3, 4, 5, color, pwinner))
        {
            XXO_PRINTF("win 345 col=%d\n\r", color);
            break;
        }

        else if (CHECK_WINNER(6, 7, 8, color, pwinner))
        {
            XXO_PRINTF("win 678 col=%d\n\r", color);
            break;
        }
        else if (CHECK_WINNER(0, 3, 6, color, pwinner))
        {
            XXO_PRINTF("win 036 col=%d\n\r", color);
            break;
        }
        else if (CHECK_WINNER(1, 4, 7, color, pwinner))
        {
            XXO_PRINTF("win 147 col=%d\n\r", color);
            break;
        }
        else if (CHECK_WINNER(2, 5, 8, color, pwinner))
        {
            XXO_PRINTF("win 258 col=%d\n\r", color);
            break;
        }
        else if (CHECK_WINNER(0, 4, 8, color, pwinner))
        {
            XXO_PRINTF("win 048 col=%d\n\r", color);
            break;
        }

        else if (CHECK_WINNER(2, 4, 6, color, pwinner))
        {
            XXO_PRINTF("win 246 col=%d\n\r", color);
            break;
        }
    }

    if (color == 3)
    {
        for(i = 0; i < 9; i++)
        {
            if (LedState[i] == 0)
            {
                break;
            }
        }
        if (i == 9)
        {
            pwinner->col = 3;
            ret = pwinner;
        }
    }
    else
    {
        ret = pwinner;
    }

    return ret;
}

int main(void)
{

    game_state_t state = IDLE;
    winner_t winner;
    uint8_t key, revent, mycolor = 0;
    io_init();

    XXO_PRINT(TYPE" Tic-Tac-Toe booted\n\r");
    state = IDLE;

    while(1)
    {
        switch(state)
        {
            case IDLE:
                key = xxo_get_key();
                revent = xxo_radio_get_event();
                if(key != KEY_NONE)
                {
                    LedState[4] ^= 3;
                    XXO_PRINT("request\n\r");
                }
                if (revent == REQUEST_GAME)
                {
                    XXO_PRINT("start\n\r");
                    mycolor = 2;
                    xxo_flash_leds(4, mycolor);
                    state = WAIT_MY_KEY;
                }
                if (revent == START_GAME)
                {
                    mycolor = 1;
                    xxo_flash_leds(4, mycolor);
                    state = WAIT_PEER_KEY;
                }
                break;

            case WAIT_MY_KEY:
                key = xxo_get_key();
                if (key != KEY_NONE)
                {
                    if (LedState[key] == 0)
                    {
                        LedState[key] = mycolor;
                        XXO_PRINTF("key: %d\n\r", key);
                        state = WAIT_PEER_KEY;
                    }
                }
                break;

            case WAIT_PEER_KEY:
                revent = xxo_radio_get_event();
                if (revent != KEY_NONE)
                {
                    LedState[revent] = mycolor ^ 3;
                    state = WAIT_MY_KEY;
                }
                break;

        }

        if(key != KEY_NONE)
        {
            XXO_PRINTF(TYPE" state %d c=%d key=%d\n\r", state, mycolor, key);
        }

        if(revent != KEY_NONE)
        {
            XXO_PRINTF(TYPE" state %d c=%d revent=%d\n\r", state, mycolor, revent);
        }
        key = KEY_NONE;
        revent = KEY_NONE;

        if ( check_winner(&winner) )
        {
            XXO_PRINTF(TYPE" winner %d\n\r", winner.col);
            if (winner.col != 0)
            {
                xxo_highlight_winning_pattern(10, &winner);
                xxo_flood_leds(winner.col);
                xxo_set_leds(4, mycolor);
            }
            else
            {
                xxo_flash_leds(2,mycolor^3);
                xxo_flash_leds(2,mycolor);
            }
            xxo_do_sleep(3000);
            cli();
            xxo_leds_init();
            sei();
            mycolor = 0;
            state = IDLE;
        }

        AVR_DO_SLEEP();
    }
}
