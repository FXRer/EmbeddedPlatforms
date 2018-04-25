/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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
 * @file Driver for RTC
 * @brief ....
 * @_addtogroup grpApp...
 *
 *
 */

/* === includes ============================================================ */

/* avr-libc */
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* project */
#include "rtc.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

uint16_t systime_sec = 0;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

ISR(TIMER2_OVF_vect)
{

}

ISR(TIMER2_COMP_vect)
{
	systime_sec++;
	rtc_tick_cb(systime_sec);
}

void rtc_init(void)
{
	ASSR |= (1<<AS2);

	TCCR2A = 0;
	while(ASSR & (1<<TCR2UB));
	
	TCNT2 = 0;
	while(ASSR & (1<<TCN2UB));

	OCR2A = 0x7F;
	while(ASSR & (1<<OCR2UB));
	
	TIFR2 |= (1<<TOV2) | (1<<OCF2A); /* clear flags */
	TIMSK2 = (1<<OCIE2A);

	TCCR2A =  (1<<WGM21) | (1<<CS22) | (1<<CS21); /* start CTC mode, PS=256 */
	while(ASSR & (1<<TCR2UB));
}

/* EOF */
