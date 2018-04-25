/* Copyright (c) 2011 Axel Wachtler, Joerg Wunsch
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>
#include <util/delay.h>

#include "project.h"
#include "board.h"
#include "hif.h"
#include "radio.h"

#define COL_PADS    (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))


static volatile uint8_t LedState[9] = {0,0,0, 0,0,0, 0,0,0};

radio_tx_done_t tx_done_status;
volatile bool tx_in_progress;

void usr_radio_tx_done(radio_tx_done_t status)
{
  tx_done_status = status;
  tx_in_progress = false;
  if (status != TX_OK)
    PRINTF("TX status: %d\n", status);
}

void trx_printf(const char *fmt, ...)
{
  uint8_t txbuf[UART_FRAME_SIZE];
  va_list ap;
  int n;
  static uint8_t seqno;

  va_start(ap, fmt);
  n = vsnprintf_P((char *)txbuf + PAYLD_START, PAYLD_SIZE, fmt, ap);
  va_end(ap);

  prot_header_t *hdr = (prot_header_t*)txbuf;
  hdr->fcf = FCF(TYPE_DATA, 0);
  hdr->seq = seqno++;
  hdr->panid = PANID;
  hdr->dst = 0xffff;
  hdr->src = SHORTADDR;
  radio_set_state(STATE_TXAUTO);
  tx_in_progress = true;
  if (n >= PAYLD_SIZE)
    n = PAYLD_SIZE;

  radio_send_frame(sizeof(prot_header_t) + CRC_SIZE + n, txbuf, 0);

  while (tx_in_progress)
    sleep_mode();
}


static uint8_t PadScans[9] = { 0 };
static uint8_t PadState[9] = { 0 };
static volatile uint8_t KeyEvent;

#define KEY_NONE (255)
#define SCAN_LOWER (2)
#define SCAN_UPPER (6)
#define SCAN_THRS_RELEASE (SCAN_LOWER + 1)
#define SCAN_THRS_PRESS   (SCAN_UPPER - 1)

uint8_t scan_pads(uint8_t row)
{
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
            ret = keyidx;
        }

        pscan ++;
        pstate ++;
        keyidx ++;
    }
    return ret;
}


void display_leds(row)
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
    pled_state = &LedState[3*row];

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



ISR(TIMER0_COMPA_vect)
{

}

ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;

    KeyEvent = scan_pads(row);

    display_leds(row);

    row ++;
    if (row > 2)
    {
        row = 0;
    }
}

static void
my_radio_init(void)
{
  radio_init(NULL, 0);          /* no RX functionality */
  radio_set_param(RP_CHANNEL(CHANNEL));
  radio_set_param(RP_IDLESTATE(STATE_OFF));
  radio_set_state(STATE_OFF);
  radio_set_param(RP_SHORTADDR(SHORTADDR));
  radio_set_param(RP_PANID(PANID));
  radio_set_param(RP_TXPWR(-4));
}

static void
ioinit(void)
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
    sei();

    DDRB  = COL_LEDS | ROW_IO_MASK;
    PORTB = COL_LEDS | ROW_IO_MASK;
    PORTB = _BV(PB0);
    DDRD  = 0;
    PORTD = 0;

    KeyEvent = KEY_NONE;
}

int main(void)
{
    uint8_t curr_key, evcnt = 0;

    ioinit();
    my_radio_init();

    TRX_PRINT("aw Tic-Tac-Toe booted\n\r");

    while(1)
    {

        cli();
        curr_key = KeyEvent;
        sei();

        if (curr_key != KEY_NONE)
        {

            evcnt ++;
            LedState[curr_key]++;
            if (LedState[curr_key] > 2)
            {
                LedState[curr_key] = 0;
            }

#if 0
            TRX_PRINTF("r%04d %d "\
                       "%02x %02x %02x  "\
                       "%02x %02x %02x  "\
                       "%02x %02x %02x\r\n",
                       evcnt, curr_key,
                       PadScans[0],PadScans[1], PadScans[2],
                       PadScans[3],PadScans[4], PadScans[5],
                       PadScans[6],PadScans[7], PadScans[8]
                       );
#else
            TRX_PRINTF("r%04d %d: "\
                       "%c %c %c  "\
                       "%c %c %c  "\
                       "%c %c %c\r\n"\
                       "         "\
                       "%d %d %d  "\
                       "%d %d %d  "\
                       "%d %d %d\r\n",
                       evcnt, curr_key,
                       PadState[0], PadState[1], PadState[2],
                       PadState[3], PadState[4], PadState[5],
                       PadState[6], PadState[7], PadState[8],
                       LedState[0], LedState[1], LedState[2],
                       LedState[3], LedState[4], LedState[5],
                       LedState[6], LedState[7], LedState[8]

                       );
#endif

#if 0
            TRX_PRINTF("r%x %x %x  %x %x %x  %x %x %x\r\n",
                        PadScans[0],PadScans[1], PadScans[2],
                        PadScans[3],PadScans[4], PadScans[5],
                        PadScans[6],PadScans[7], PadScans[8]
                        );
#endif

        }

        /* sleep until next IRQ */
        sleep_mode();
    }
    return 0;
}
