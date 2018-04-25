/* Copyright (c) 2011 Axel Wachtler
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
/* Sensor application for "Chemnitzer Linuxtage 2011" */

#include <stdio.h>
#include <string.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include "board.h"
#include "radio.h"
#include "sensorapp.h"
#include "sensors.h"
#if defined(muse231)
#include "i2c.h"
#endif	/* defined(muse231) */
/* === Macros ========================================================== */

/* === Types =========================================================== */

typedef struct {
	uint16_t vmcu; /**< [mV] */
	uint16_t sht_t; /**< SHT21 temperature [unscaled] */
	uint16_t sht_rh; /**< SHT21 relative humidity [unscaled] */
	mma7455_result_t acc; /**< acceleration */
	uint16_t light;	/**< light sensor */
} meas_result_t;

/* === Globals ========================================================= */
rt_frame_t TxFrame = { .fctl = FRAME_CTRL, .pan = RT_PANID, .dst = RT_ADDR,
		.src = 0xffff };

node_config_t PROGMEM nc_flash = { .short_addr = 0xBEEF, 
                                   .pan_id = 0x2222, 
                                   .channel = CHANNEL};

static node_config_t nc;

typedef enum {
	APP_IDLE, APP_MEASURE, APP_RESULT, APP_SEND,
} app_event_t;

volatile app_event_t app_event = APP_IDLE;

volatile meas_result_t meas_result;

/* === Implementation ================================================== */
static void app_init(void) {

	LED_INIT();
#if defined(muse231)
	i2c_init();
#endif
#if !defined(lgee231) /* lgee231 has support for HIF, but do not use here due to code size */
	hif_init(9600);
#endif
	timer_init();

    /* 1st trial: read from internal EEPROM */
    if (get_node_config_eeprom(&nc, 0) == 0)
    {
        /* using EEPROM config */;
    }
    /* 2nd trial: read from FLASHEND */
    else if (get_node_config(&nc) == 0)
    {
        /* using FLASHEND config */;
    }
    /* 3rd trial: read default values compiled into the application */
    else
    {
        /* using application default config */;
        memcpy_P(&nc, &nc_flash, sizeof(node_config_t) );
    }


    LED_SET_VALUE(1);
    
    radio_init(NULL, 0);
    radio_set_state(STATE_SLEEP);
    radio_set_param(RP_CHANNEL(nc.channel));
    radio_set_param(RP_IDLESTATE(STATE_RXAUTO));

    TxFrame.src = nc.short_addr;
    sei();
    
    LED_SET_VALUE(0);
    
}

static void app_task(void) {
	app_event_t state;
    uint8_t txfrm_size;

	cli();
	state = app_event;
	app_event = APP_IDLE;
	sei();

	do {
		switch (state) {
		case APP_IDLE:
			break;
		case APP_MEASURE:
    #if defined(muse231)
			meas_result.vmcu = measure_vmcu();
			meas_result.sht_t = measure_sht_temperature();
			meas_result.sht_rh = measure_sht_humidity();
			measure_acceleration((mma7455_result_t*)&meas_result.acc);
			meas_result.light = measure_light();
			txfrm_size = sprintf(TxFrame.data, 
                    "addr=0x%X, vmcu=%u, sht_t=%u, sht_rh=%u, "\
                    "acc=[%d,%d,%d], light=%u\n\r",
					TxFrame.src, meas_result.vmcu,
					meas_result.sht_t, meas_result.sht_rh,
					meas_result.acc.x, meas_result.acc.y,meas_result.acc.z,
					meas_result.light);
    #elif defined(tiny230)
    			meas_result.vmcu = measure_vmcu();
			//meas_result.sht_t = measure_t_kty();
			txfrm_size = sprintf(TxFrame.data, 
                    "addr=0x%X, cnt=%u, vmcu=%u\n\r",
					TxFrame.src, TxFrame.seq, 
					meas_result.vmcu);
	#elif defined(lgee231)
			meas_result.vmcu = measure_vmcu();
			txfrm_size = sprintf(TxFrame.data, 
                    "addr=0x%X, cnt=%u, vmcu=%u\n\r",
					TxFrame.src, TxFrame.seq, 
					meas_result.vmcu);
    #else
    # error "Unknown Board Type"
    #endif
            txfrm_size += 11;
			state = APP_RESULT;
			break;
		case APP_RESULT:
            LED_SET(0);
			/* take care not to exceed the limit of TxFrame.data */
			radio_set_state(STATE_TXAUTO);
			radio_send_frame(sizeof(TxFrame), (uint8_t *) &TxFrame, 0);
			state = APP_IDLE;
			break;
		default:
			state = APP_IDLE;
			break;
		}
	} while (state != APP_IDLE);
}

static inline void do_sleep(void) {
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

int main(void) {
	app_init();
#if !defined(lgee231)
	PRINTF("Sensor App\n\r This is node: 0x%04x\n\r", nc.short_addr);
#endif
	timer_start(tmr_transmit, T_TX_PERIOD, 0);
	while (1) {
		app_task();
		/** sleep, node awakes by an occuring IRQ. */
		do_sleep();
	}
}

time_t tmr_transmit(timer_arg_t t) {
	app_event = APP_MEASURE;
	return 0;
}

/*
 * \brief Called on Rx slot timeout
 * No matter if a frame was received or not
 */
time_t tmr_rxslot(timer_arg_t t)
{
	radio_set_state(STATE_SLEEP);
	timer_start(tmr_transmit, T_TX_PERIOD, 0);
	return 0;	/* do not restart */
}

uint8_t* usr_radio_receive_frame(
		uint8_t len,
		uint8_t *frm,
		uint8_t lqi,
		int8_t ed,
		uint8_t crc_fail)
{

	/* parse frame here */

	return frm;
}

void usr_radio_tx_done(radio_tx_done_t status) {
    LED_CLR(0);
	if (status == TX_OK) {
		TxFrame.seq++;
	}
	timer_start(tmr_rxslot, T_RX_SLOT, 0);
}
/* EOF */
