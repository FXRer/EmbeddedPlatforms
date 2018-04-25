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


/*
 * There are two different multiplex scan scenarios.  In scan state
 * SCAN_LED_ORANGE, the respective row output actively drives high
 * level, while the inactive rows remain high Z.  LEDs that are to be
 * driven in orange get their column outputs set to low, LEDs that are
 * turned off keep the column outputs at high level.
 *
 * During that state, the resistive input pads are scanned, too.
 *
 * In state SCAN_LED_GREEN, the active row output drives low level,
 * with the inactive rows being high Z, again.  Column outputs for
 * LEDs that are to be driven in green are set to high output level,
 * column outputs for LEDs that are turned off are low.
 */
typedef enum
{
    SCAN_LED_ORANGE, SCAN_LED_GREEN
}
    scan_t;
#if 0
/*
 * Global array of LED state.  Can be set externally to LED_OFF,
 * LED_GREEN, or LED_ORANGE.  Values will be examined by the multiplex
 * matrix scan algorithm.
 */
led_color_t leds[NROWS][NCOLS];

/*
 * Global array of touch pad state.  `true' means `this pad is
 * active'.  Updated during pad scan, can be examined by application.
 */
bool pads[NROWS][NCOLS];

/*
 * Row, column of last pad that changed state.  Updated during pad
 * scan, can be examined by application.
 */
volatile row_col_t last_pad;

/*
 * Flag that indicates at least one pad changed state during last
 * input pad scan.  Can be examined, and must be cleared by
 * application.
 */
volatile bool pad_toggled;

/*
 * Current scan state of the multiplex driver.
 */
static scan_t scan_state;

/*
 * Currently active multiplex driver row.
 */
static uint8_t row;

#endif



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


ISR(TIMER0_COMPA_vect)
{
}

ISR(TIMER0_OVF_vect)
{
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

    DDRB  = COL_LEDS | ROW_IO_MASK;
    PORTB = COL_LEDS | ROW_IO_MASK;
    DDRD  = 0;
    PORTD = 0;
}



int main(void)
{
    uint8_t row;
    uint16_t scan;

    uint8_t scans[9], i, *pscan;
    uint8_t keys[9], event, *pkeys, evcnt=0, keypress, keyrelease;
    ioinit();
    my_radio_init();

    TRX_PRINT("aw Tic-Tac-Toe booted\n\r");

    while(1)
    {
        for (row=0;row<3;row++)
        {


            /* discharge PIND */
            DDRD = COL_PADS;
            PORTB = 0;
            _delay_ms(1);
            /* charge PIND */
            //PORTD = 7;
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
            _delay_ms(6);
            scan = PIND;

            pscan = &scans[row*3];
            pkeys = &keys[row*3];
            for (i = 0; i < 3; i++)
            {
                *pscan += (scan & _BV(i+5)) ? +1 : -1;
                if (*pscan < 3)
                {
                    *pscan = 3;
                }
                if (*pscan > 9 )
                {
                    *pscan = 9;
                }

                /* detect key events */
                if ((*pscan < 4) && (*pkeys != '_'))
                {
                    *pkeys = '_';
                    //event += 1;
                }


                if ((*pscan > 8) && (*pkeys != '#'))
                {
                    *pkeys = '#';
                    event += 1;
                    keypress = 3*row + i;
                }

                pscan ++;
                pkeys ++;
            }

            /* event dump after all 3 rows are scanned*/
            if (row == 2)
            {
                if (event)
                {
                    evcnt ++;
#if 0
                    TRX_PRINTF("r%04d %d:k=%c: "\
                               "%02x %02x %02x  "\
                               "%02x %02x %02x  "\
                               "%02x %02x %02x\r\n",
                               evcnt, event, keypress+48,
                               scans[0],scans[1], scans[2],
                               scans[3],scans[4], scans[5],
                               scans[6],scans[7], scans[8]
                               );
#else
                    TRX_PRINTF("r%04d %d:k=%c: "\
                               "%c %c %c  "\
                               "%c %c %c  "\
                               "%c %c %c\r\n",
                               evcnt, event, keypress+48,
                               keys[0], keys[1], keys[2],
                               keys[3], keys[4], keys[5],
                               keys[6], keys[7], keys[8]
                               );

#endif
                    event = 0;
                    keypress = 15;
                    keyrelease = 15;
                }
#if 0
                TRX_PRINTF("r%x %x %x  %x %x %x  %x %x %x\r\n",
                            scans[0],scans[1], scans[2],
                            scans[3],scans[4], scans[5],
                            scans[6],scans[7], scans[8]
                            );
#endif
            }

        }
    }
    return 0;
}
