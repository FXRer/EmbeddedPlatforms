/* Copyright (c) 2008 Axel Wachtler
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
/* Example use of the radio and ioutil functions for a simple range test */

#include <stdio.h>
#include "board.h"
#include "hif.h"
#include "radio.h"
#include "sensorapp.h"

/* === Macros ========================================================== */

/* === Types =========================================================== */

/* === Globals ========================================================= */
rt_frame_t TxFrame = {
        .fctl = FRAME_CTRL,
        .pan = RT_PANID,
        .dst = RT_ADDR,
        .src = 0xffff
};


node_config_t nc = {.short_addr = 0, .pan_id = 0};

typedef enum
{
    APP_IDLE,
    APP_MEASURE,
    APP_RESULT,
    APP_SEND,
} app_event_t;

volatile app_event_t app_event = APP_IDLE;

uint8_t station_temp = 23;
uint8_t station_id = 0x01 ;

/* === Implementation ================================================== */
void app_init(void)
{
    LED_INIT();
    hif_init(9600);
    timer_init();
    radio_init(NULL, 0);
    radio_set_state(STATE_SLEEP);
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_param(RP_IDLESTATE(STATE_SLEEP));

    /* copy flash settings */
    get_node_config(&nc);
    TxFrame.src = station_id ;
    sei();
}


void app_task(void)
{
    app_event_t state;

    cli();
    state = app_event;
    app_event = APP_IDLE;
    sei();

    do
    {
        switch(state)
        {
            case APP_IDLE:
                break;
            case APP_MEASURE:
                temp_start();
                state = APP_RESULT;
                break;
            case APP_RESULT:
                sprintf(TxFrame.data, "n: 0x%04x, t: %d\n\r",
                        TxFrame.src, temp_get());
                state = APP_SEND;
                break;
            case APP_SEND:
                radio_set_state(STATE_TXAUTO);
                radio_send_frame(sizeof(TxFrame) , (uint8_t *)&TxFrame, 0);
                state = APP_IDLE;
                break;
            default:
                state = APP_IDLE;
                break;
        }
    } while(state != APP_IDLE);
}


int main(void)
{
    app_init();
    PRINTF("Sensor App\n\r This is node: 0x%04x\n\r", nc.short_addr);
    timer_start(tmr_transmit, T_TX_PERIOD, 0);
    while(1)
    {
        app_task();
        /** sleep, node awakes by an occuring IRQ. */
        do_sleep();
    }
}

time_t tmr_transmit(timer_arg_t t)
{
    LED_TOGGLE(0);
    app_event = APP_MEASURE;
    return 0;
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    LED_TOGGLE(0);
    if (status == TX_OK)
    {
        TxFrame.seq++;
    }
    timer_start(tmr_transmit, T_TX_PERIOD, 0);
}
/* EOF */
