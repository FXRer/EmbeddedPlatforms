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

#include "board.h"
#include "hif.h"
#include "radio.h"
#include "project.h"

#define COL_PADS    (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))


/*
 * Input scan row.  Essentially, this is the previous value of 'row'.
 */
static uint8_t scanrow;

radio_tx_done_t tx_done_status;
volatile bool tx_in_progress;

#define NSAMPLES 16             /* must match the inline asm below! */
#define NSAMPLE 2
static uint8_t samples_port_g[NSAMPLES], samples_port_d[NSAMPLES];

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


void aw_update_pads(uint8_t *scans)
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



uint8_t i;
    for (i=0; i<10;i++)
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


void
usr_radio_tx_done(radio_tx_done_t status)
{
  tx_done_status = status;
  tx_in_progress = false;
  if (status != TX_OK)
    PRINTF("TX status: %d\n", status);
}

void
trx_printf(const char *fmt, ...)
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

    TRX_PRINT("aw Tic-Tac-Toe booted (cap)\n\r");
    DDRB = 0x3f;
    PORTB = 0x7;
    while(1)
    {
        aw_update_pads(scans);
        for (row=0;row<3;row++)
        {

            aw_update_pads(scans);

            pscan = &scans[row*3];
            pkeys = &keys[row*3];
            for (i = 0; i < 3; i++)
            {


#if 0
                if (*pscan < 2)
                {
                    pscan = 2;
                }
                if (*pscan > 9)
                {
                    pscan = 9;
                }
#endif

                /* detect key events */
                if ((*pscan > 0) && (*pkeys != '_'))
                {
                    *pkeys = '_';
                    //event += 1;
                }


                if ((*pscan == 0) && (*pkeys != '#'))
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
                    TRX_PRINTF("c%04d %d:k=%c: "\
                               "%02x %02x %02x  "\
                               "%02x %02x %02x  "\
                               "%02x %02x %02x\r\n",
                               evcnt, event, keypress+48,
                               scans[0],scans[1], scans[2],
                               scans[3],scans[4], scans[5],
                               scans[6],scans[7], scans[8]
                               );
#else
                    TRX_PRINTF("c%04d %d:k=%c: "\
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
                TRX_PRINTF("c%x %x %x  %x %x %x  %x %x %x\r\n",
                            scans[0],scans[1], scans[2],
                            scans[3],scans[4], scans[5],
                            scans[6],scans[7], scans[8]
                            );
#endif
            }

        }
        _delay_ms(10);

    }
    return 0;
}
