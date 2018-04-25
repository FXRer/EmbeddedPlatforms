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

/*
 * The purpose of this software is to verify the assembled printed
 * cicuit board (PCB) works as designed.
 *
 * For that reason, the LED matrix is cycled through different display
 * patterns.  LEDs that have not been soldered correctly, or IO ports
 * that have no connection to the LEDs can easily be seen.
 *
 * As the LED matrix must be multiplexed in order to work, the
 * multiplexing is performed by 8-bit timer 0 as in the actual
 * application.  The basic code for this has been copied in from the
 * application.
 *
 * Timer 5 is setup for a 500 ms turnover (2 Hz), to cycle through the
 * LED patterns.  Some of the LED patterns are kept for more than one
 * timer tick, most of them change each 500 ms.  This is done by a
 * state machine implemented in the interrupt service routine (ISR)
 * for timer 5's "compare match" event.
 *
 * Timer 4 is setup for a 10 ms turnover (100 Hz), and is used to
 * toggle the auxiliary IO pins.  This affects all the port pins that
 * are not used to drive the LED matrix (including pins that are used
 * for input pads on the resistive or capacitive board versions,
 * respectively).  The main purpose is to verify the portpins have
 * been soldered correctly.  All these pins are configured as output
 * pins, and then switch a certain pattern.  When an internal counter
 * (within the ISR) turns over to 0, all portpins are reset to low
 * value.  On subsequent invocations of the interrupt, portpins are
 * turned high again, starting with pin 0 of each port (thus forming a
 * low pulse lasting only 10 ms), up to pin 7 of each port (thus
 * forming a low pulse lasting 80 ms, followed by a 10 ms high pulse).
 * Each portpin then generates pulses with a period of 90 ms but
 * different duty cycle.  That way, shortcuts between neighbouring
 * portpins can be detected on the oscilloscope, as the generate a
 * "stair" pulse rather than a clear low->high transition.
 *
 * The ISRs for timer 0 and timer 5, respectively, have been made
 * interruptible (keyword ISR_NOBLOCK), in order to give the pulse
 * generation of timer 4 effectively the highest priority.  This
 * avoids jitter when monitoring the auxiliary portpin signals on an
 * oscilloscope.
 *
 * Finally, while toggling through the LED patterns, the transceiver
 * block is activated and deactivated.  While active, it transmits
 * just a CW signal.  This allows to check the basic transceiver
 * functionality: if something is wrong with the 16 MHz crystal, the
 * transceiver will fail to start, and the test gets stuck at that
 * point.  By monitoring the transmitted signal on a test receiver
 * (for example, a spectrum analyzer, but even a different transceiver
 * in receive mode would do), the resulting signal strength must be in
 * the expected range.  If not, something could be wrong with the
 * antenna pins, or the supply pins that are only examined by the
 * analog circuitry of the transceiver.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>
#include <util/delay.h>

#include "board.h"
#include "radio.h"
#include "hwtest.h"

#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

//#define CHANNEL 24 /* center frequency 2470 MHz */
#define TX_PWR 11 /* -5 dBm */

/*
 * There are two different multiplex scan scenarios.  In scan state
 * SCAN_LED_RED, the respective row output actively drives high
 * level, while the inactive rows remain high Z.  LEDs that are to be
 * driven in red get their column outputs set to low, LEDs that are
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
    SCAN_LED_RED, SCAN_LED_GREEN
}
    scan_t;

/*
 * Global array of LED state.  Can be set externally to LED_OFF,
 * LED_GREEN, or LED_RED.  Values will be examined by the multiplex
 * matrix scan algorithm.
 */
led_color_t leds[NROWS][NCOLS] =
{
  {LED_RED,   LED_OFF,   LED_OFF},
  {LED_GREEN, LED_RED,   LED_OFF},
  {LED_GREEN, LED_OFF,   LED_RED},
};

/*
 * Current scan state of the multiplex driver.
 */
static scan_t scan_state;

/*
 * Currently active multiplex driver row.
 */
static uint8_t row;

/* Transmission Flag
 * It is set in the send routine and reset at 
 * the TX_END_IRQ
 */
static volatile bool TxInProgress;
node_config_t NodeConfig;

ISR(TIMER0_COMPA_vect)
{
}

ISR(TIMER0_OVF_vect, ISR_NOBLOCK)
{
    uint8_t colval, rowval = 0, porttmp;
    uint8_t colmask, col;

    porttmp = PORTB & ~(ROW_IO_MASK | COL_LEDS);

    switch (scan_state)
    {
        case SCAN_LED_RED:
            for (colval = col = 0, colmask = _BV(PB0);
                 col < NCOLS;
                 col++, colmask <<= 1)
            {
                if (leds[row][col] == LED_RED)
                {
                    colval |= colmask;
                }
            }
            switch (row)
            {
                case 0:
                    rowval = _BV(PB3);
                    row = 1;
                    break;

                case 1:
                    rowval = _BV(PB4);
                    row = 2;
                    break;

                case 2:
                    rowval = _BV(PB5);
                    row = 0;
                    scan_state = SCAN_LED_GREEN;
                    break;
            }
            PORTB = porttmp | rowval | (~colval & COL_LEDS);
            DDRB = (DDRB & ~ROW_IO_MASK) | rowval;
            break;

        case SCAN_LED_GREEN:
            for (colval = col = 0, colmask = _BV(PB0);
                 col < NCOLS;
                 col++, colmask <<= 1)
            {
                if (leds[row][col] == LED_GREEN)
                {
                    colval |= colmask;
                }
            }
            switch (row)
            {
                case 0:
                    rowval = _BV(PB3);
                    row = 1;
                    break;

                case 1:
                    rowval = _BV(PB4);
                    row = 2;
                    break;

                case 2:
                    rowval = _BV(PB5);
                    row = 0;
                    scan_state = SCAN_LED_RED;
                    break;
            }
            PORTB = porttmp | (~rowval & ROW_IO_MASK) | colval;
            DDRB = (DDRB & ~ROW_IO_MASK) | rowval;
            break;
    }
}

void xxo_radio_init(node_config_t *ncfg)
{

    if (0 != get_node_config_eeprom(ncfg, 0))
    {
        /* Standardwerte setzen, fall ungültige Daten im EEPROM */
        ncfg->short_addr = SHORTADDR;
        ncfg->pan_id = PANID;
        ncfg->channel = CHANNEL;
    }

    radio_init(NULL, 0);
    radio_set_param(RP_CHANNEL(ncfg->channel));
    radio_set_param(RP_SHORTADDR(ncfg->short_addr));
    radio_set_param(RP_PANID(ncfg->pan_id));
    radio_set_param(RP_IDLESTATE(STATE_OFF));
    radio_set_param(RP_TXPWR(-4));
    radio_set_state(STATE_OFF);
}

/* === transmit functions === */
void xxo_send(uint8_t event, node_config_t *ncfg)
{
    static uint8_t seqno = 0;
    xxo_frame_t txbuf;
    /* fill frame information */
    txbuf.fcf = 0x8841;
    txbuf.seq = seqno++;
    txbuf.panid = ncfg->pan_id;
    txbuf.dst = 0xffff;
    txbuf.src = ncfg->short_addr;
    radio_set_state(STATE_TXAUTO);

    /* fill payload */
    txbuf.event = event;

    /* send frame */
    TxInProgress = true;

    radio_send_frame(sizeof(xxo_frame_t), (uint8_t*)&txbuf, 0);

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (TxInProgress)
    {
        sleep_mode();
    }
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    TxInProgress = false;
}



ISR(TIMER5_COMPA_vect, ISR_NOBLOCK)
{
    static uint8_t ledstate;
    unsigned int *lp = (unsigned int *)leds;

    ledstate++;

    switch (ledstate)
    {
        case 0 ... 2:
            break;

        case 3:
            memset(leds, 0, sizeof leds);
            //send_cw(CHANNEL, 'r', TX_PWR);
	    xxo_send(42, &NodeConfig);
        case 4 ... 11:
            lp[ledstate - 3] = LED_GREEN;
            break;

        case 12:
            memset(leds, 0, sizeof leds);
        case 13 ... 20:
            lp[ledstate - 12] = LED_RED;
            break;

        case 21:
            memset(leds, 0, sizeof leds);
            //stop_cw();
            break;

        case 22 ... 24:
            lp[0] = lp[2] = lp[4] = lp[6] = lp[8] = LED_GREEN;
            lp[1] = lp[3] = lp[5] = lp[7] = LED_RED;
            break;

        case 25:
            memset(leds, 0, sizeof leds);
            break;

        case 26 ... 28:
            lp[0] = lp[2] = lp[4] = lp[6] = lp[8] = LED_RED;
            lp[1] = lp[3] = lp[5] = lp[7] = LED_GREEN;
            break;

        case 29:
            memset(leds, 0, sizeof leds);
            ledstate = 0;
    }
}

ISR(TIMER4_COMPA_vect)
{
    static uint8_t bitno;

    if (bitno == 8)
    {
        PORTD = 0;
        PORTE = 0;
        PORTG = 0;
        PORTB &= ~0xc0;
        PORTF &= ~0x0f;
        bitno = 0;
        return;
    }

    uint8_t bitmask = 1 << bitno;

    /* ports D and E are AUX ports on all bits */
    PORTD |= bitmask;
    PORTE |= bitmask;
    /* port G: bits 0 ... 5 are AUX, but that's all bits here */
    PORTG |= bitmask;
    /* port B: only bits 6 and 7 are AUX */
    if (bitno >= 6)
        PORTB |= bitmask;
    /* port F: only bits 0 ... 3 are AUX */
    if (bitno < 4)
        PORTF |= bitmask;

    bitno++;
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

    /*
     * Timer 5 with prescaler 8, CTC mode, overall frequency
     * 2 Hz.
     */
    OCR5A = (F_CPU / 8 / 2) - 1;
    TCCR5B = _BV(WGM52) | _BV(CS51);
    TIMSK5 = _BV(OCIE5A);
    sei();

    /*
     * Timer 4 with prescaler 1, CTC mode, frequency 100 Hz
     */
    OCR4A = (F_CPU / 1 / 100) - 1;
    TCCR4B = _BV(WGM42) | _BV(CS40);
    TIMSK4 = _BV(OCIE4A);

    DDRB  = ROW_IO_MASK | COL_LEDS;
    PORTB = ROW_IO_MASK | COL_LEDS;
    scan_state = SCAN_LED_RED;
    DDRD  = 0;
    PORTD = 0;

    /* AUX IO pins */
    DDRB |= 0xc0;               /* PB[6:7] */
    DDRD = 0xff;                /* PD[0:7] */
    DDRE = 0xff;                /* PE[0:7] */
    DDRF = 0x0f;                /* PF[0:3] */
    DDRG = 0x3f;                /* PG[0:5] */

    /* tune LED driver pins to 8 mA */
    DPDS0 = _BV(PBDRV1) | _BV(PBDRV0);
}

int main(void)
{

    ioinit();
    xxo_radio_init( &NodeConfig);


    while(1)
    {
    }
    return 0;
}
