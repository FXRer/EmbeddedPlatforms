/* Copyright (c) 2007 Axel Wachtler
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
/* Example for using module timer */

#include "board.h"
#include "timer.h"
#include "hif.h"
#include "ioutil.h"

/* === prototypes =========================================================== */
time_t usr_timer_cb_0(timer_arg_t ta);
time_t usr_timer_cb_1(timer_arg_t ta);

int main(void)
{
    uint8_t targ; /**< extra argument to give to timer callback */

    mcu_init();
    LED_INIT();
    hif_init(HIF_DEFAULT_BAUDRATE);
    timer_init();
    MCU_IRQ_ENABLE();

    targ = 42;
    timer_start(usr_timer_cb_0, MSEC(1000), (timer_arg_t) targ);
    timer_start(usr_timer_cb_1, MSEC(300), 0);

    while(1)
    {
    	/* MCU sleep, wake up by Timer ISR */
        MCU_SLEEP_IDLE();
    }
}

time_t usr_timer_cb_0(timer_arg_t targ)
{
    PRINTF("usr_timer_cb: targ=%d now = %ld ticks\n", (uint8_t)targ, timer_systime());
    LED_TOGGLE(0);
    /* restart timer afer 1000ms */
    return MSEC(1000);
}

time_t usr_timer_cb_1(timer_arg_t targ)
{
	LED_TOGGLE(1);
	return MSEC(300);
}

/*EOF*/
