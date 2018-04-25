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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "game.h"
/* === macros ============================================================== */
#define NSAMPLES 16             /* must match the inline asm below! */
#define NSAMPLE 2

/* === types =============================================================== */

/* === globals ============================================================= */
static uint8_t samples_port_g[NSAMPLES];
static uint8_t samples_port_d[NSAMPLES];
static uint8_t PadScans[9] = { 0 };
static uint8_t PadState[9] = { 0 };
static volatile uint8_t KeyEvent;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/*
 * Sample port D's input pins for their attached capacitance.
 *
 * First, the port is set to output, at low level, to discharge the
 * capacitor attached to the pin.  Then, the port is turned into an
 * input, and the input pullups are applied.  This causes the input
 * capacitor to be slowly charged, while the digital input register is
 * continuously sampled 16 times.  As the timing of this part is
 * crucial, inline assembly is used to quickly (and in equal time
 * steps) sample the input data into registers, from where they can be
 * stored into their final destination later on (by the compiler).
 */
static void sample_port_d(void)
{
    PORTD = 0;
    DDRD = 0xe0;
    __asm ("nop");
    DDRD = 0;
    PORTD = 0xe0;
    __asm ("in %[s0], %[pind]" "\n\t"
           "in %[s1], %[pind]" "\n\t"
           "in %[s2], %[pind]" "\n\t"
           "in %[s3], %[pind]" "\n\t"
           "in %[s4], %[pind]" "\n\t"
           "in %[s5], %[pind]" "\n\t"
           "in %[s6], %[pind]" "\n\t"
           "in %[s7], %[pind]" "\n\t"
           "in %[s8], %[pind]" "\n\t"
           "in %[s9], %[pind]" "\n\t"
           "in %[s10], %[pind]" "\n\t"
           "in %[s11], %[pind]" "\n\t"
           "in %[s12], %[pind]" "\n\t"
           "in %[s13], %[pind]" "\n\t"
           "in %[s14], %[pind]" "\n\t"
           "in %[s15], %[pind]"
           :
           /* output operands */
           [s0] "=r" (samples_port_d[0]),
           [s1] "=r" (samples_port_d[1]),
           [s2] "=r" (samples_port_d[2]),
           [s3] "=r" (samples_port_d[3]),
           [s4] "=r" (samples_port_d[4]),
           [s5] "=r" (samples_port_d[5]),
           [s6] "=r" (samples_port_d[6]),
           [s7] "=r" (samples_port_d[7]),
           [s8] "=r" (samples_port_d[8]),
           [s9] "=r" (samples_port_d[9]),
           [s10] "=r" (samples_port_d[10]),
           [s11] "=r" (samples_port_d[11]),
           [s12] "=r" (samples_port_d[12]),
           [s13] "=r" (samples_port_d[13]),
           [s14] "=r" (samples_port_d[14]),
           [s15] "=r" (samples_port_d[15])
           :
           /* input operands */
           [pind] "I" (_SFR_IO_ADDR(PIND)));
}

/*
 * Same as for port D above, but only portpin G0 is used as an input,
 * all other pins are not used for that purpose (but are sampled
 * anyway, as sampling always applies to a full port).
 */
static void sample_port_g(void)
{
    PORTG = 0;
    DDRG = 0x3f;
    __asm ("nop");
    DDRG = 0;
    PORTG = 0x3f;
    __asm ("in %[s0], %[ping]" "\n\t"
           "in %[s1], %[ping]" "\n\t"
           "in %[s2], %[ping]" "\n\t"
           "in %[s3], %[ping]" "\n\t"
           "in %[s4], %[ping]" "\n\t"
           "in %[s5], %[ping]" "\n\t"
           "in %[s6], %[ping]" "\n\t"
           "in %[s7], %[ping]" "\n\t"
           "in %[s8], %[ping]" "\n\t"
           "in %[s9], %[ping]" "\n\t"
           "in %[s10], %[ping]" "\n\t"
           "in %[s11], %[ping]" "\n\t"
           "in %[s12], %[ping]" "\n\t"
           "in %[s13], %[ping]" "\n\t"
           "in %[s14], %[ping]" "\n\t"
           "in %[s15], %[ping]"
           :
           /* output operands */
           [s0] "=r" (samples_port_g[0]),
           [s1] "=r" (samples_port_g[1]),
           [s2] "=r" (samples_port_g[2]),
           [s3] "=r" (samples_port_g[3]),
           [s4] "=r" (samples_port_g[4]),
           [s5] "=r" (samples_port_g[5]),
           [s6] "=r" (samples_port_g[6]),
           [s7] "=r" (samples_port_g[7]),
           [s8] "=r" (samples_port_g[8]),
           [s9] "=r" (samples_port_g[9]),
           [s10] "=r" (samples_port_g[10]),
           [s11] "=r" (samples_port_g[11]),
           [s12] "=r" (samples_port_g[12]),
           [s13] "=r" (samples_port_g[13]),
           [s14] "=r" (samples_port_g[14]),
           [s15] "=r" (samples_port_g[15])
           :
           /* input operands */
           [ping] "I" (_SFR_IO_ADDR(PING)));
}


void cap_update_pads(uint8_t *scans)
{
    sample_port_d();
    sample_port_g();

    /*
     * Input pad portpin assignment
     *
     * row\col   0    1    2
     * 0       PD7  PD4  PD1
     * 1       PD6  PD3  PD0
     * 2       PD5  PD2  PG0
     */
    for (uint8_t i=0; i<10;i++)
    {
        switch(i)
        {
            case 0:
                scans[0] = (samples_port_d[NSAMPLE] & _BV(PD7)) ;
                break;
            case 1:
                scans[1] = (samples_port_g[NSAMPLE] & _BV(PG5)) ;
                break;
            case 2:
                scans[2] = (samples_port_g[NSAMPLE] & _BV(PG2)) ;
                break;
            case 3:
                scans[3] = (samples_port_d[NSAMPLE] & _BV(PD6)) ;
                break;
            case 4:
                scans[4] = (samples_port_g[NSAMPLE] & _BV(PG4)) ;
                break;
            case 5:
                scans[5] = (samples_port_g[NSAMPLE] & _BV(PG1)) ;
                break;
            case 6:
                scans[6] = (samples_port_d[NSAMPLE] & _BV(PD5)) ;
                break;
            case 7:
                scans[7] = (samples_port_g[NSAMPLE] & _BV(PG3)) ;
                break;
            case 8:
                scans[8] = (samples_port_g[NSAMPLE] & _BV(PG0)) ;
                break;
        }
    }

}

void xxo_scan_pads(uint8_t row)
{
    uint8_t i, *pscan, *pstate;
    KeyEvent = KEY_NONE;
    cap_update_pads(PadScans);
    pscan = &PadScans[0];
    pstate = &PadState[0];
    for (i = 0; i < 10; i++)
    {

        /* detect key events */
        if ((*pscan > 0) && (*pstate != '_'))
        {
            *pstate = '_';
        }


        if ((*pscan == 0) && (*pstate != '#'))
        {
            *pstate = '#';
            KeyEvent = i;
        }
        pscan ++;
        pstate ++;
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
