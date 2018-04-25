/* Copyright (c) 2009 Axel Wachtler
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

/* fetch symbol HAS_RTC indirectly from generated board_cfg.h */
#include "board.h" 

#if defined(HAS_RTC)
#include "rtc.h"

#if defined(RTC_AVR_MEGA) || defined(RTC_AVR_XMEGA)
#include <avr/io.h>
#elif defined(RTC_AVR_MEGA_WATCHDOG)
#include <avr/io.h>
#include <avr/wdt.h>
#else
#endif

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

static void (*tickfunc)(void);

/* === functions =========================================================== */
#if defined(RTC_AVR_MEGA)
void rtc_init(void (*tf)(void))
{
    /* enable async Timer2, it can take up to 2 seconds to ramp up 32kHz crystal */
    ASSR |= (1<<AS2);

    TCCR2A = 0;
    TCCR2B = 0;
    TIMSK2 = 0;

    if(tf != 0x0000)
    {
        tickfunc=tf;

        /* TODO: check if atomic or timer disable is necessary */
        TIMSK2 |= (1<<OCIE2A);
    }
}

void rtc_start(void)
{
    while(ASSR & (1<<TCR2AUB));
	TCCR2A = (1<<WGM21); /* CTC mode TOP=OCRA */

	while(ASSR & (1<<TCR2BUB));
    TCCR2B |= RTC_PRESCALER_BITS & 0x07;

    while(ASSR & (1<<OCR2AUB));
    OCR2A = RTC_HWTICK_NB;
}

void rtc_stop(void)
{
    while(ASSR & (1<<TCR2BUB));
    TCCR2B = 0;
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK)
{
    /* it has to be ensured from outside it is not NULL,
     * no checking here to speed up
     */
    tickfunc();
}

#elif defined(RTC_AVR_MEGA_WATCHDOG)

ISR(WDT_vect, ISR_NOBLOCK)
{
	tickfunc();
}

void rtc_init(void (*tickfunc)(void))
{
	wdt_disable();

    if(tf != 0x0000)
    {
        tickfunc=tf;
    }
}

void rtc_start(void)
{
	WDTCSR = (1 << WDCE) | (1 << WDE); /* Enable configuration change */
	WDTCSR = (1 << WDIF) | (1 << WDIE) | /* Enable Watchdog Interrupt Mode */
	(timeout & 0x08 ? _WD_PS3_MASK : 0x00) | (timeout & 0x07);
}

void rtc_stop(void)
{
}


#elif defined(RTC_AVR_XMEGA)

void rtc_init(void (*tf)(void))
{
	CLK.RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.PER = 1023;
	RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;

    if(tf != 0x0000)
    {
        tickfunc=tf;
    }
}

void rtc_start(void)
{
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.CTRL = 1;
}

void rtc_stop(void)
{
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.CTRL = 0;
}

ISR(RTC_OVF_vect)
{
    tickfunc();
}

#else

#error "RTC undefined, this source should not go into compilation"

#endif /* defined(RTC_ */
#endif /* if defined(HAS_RTC) */
/* EOF */
