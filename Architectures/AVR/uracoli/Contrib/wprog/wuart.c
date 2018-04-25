/* Copyright (c) 2012 Daniel Thiele, Axel Wachtler
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

#include <board.h>
#include <timer.h>
#include <transceiver.h>
#include <radio.h>
#include <p2p_protocol.h>

#include "wuart.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* IEEE802.15.4 parameters for communication */
const node_config_t PROGMEM nc_flash =
{
	.short_addr = 0xFECA,
	.pan_id = 0x3412,
	.channel = 17
};

node_config_t NodeConfig;

timer_hdl_t thdl = NONE_TIMER; /* flush timeout handle */
time_t timeout_ms = MSEC(50);

uint8_t rxbuf[MAX_FRAME_SIZE];
uint8_t txbuf[MAX_FRAME_SIZE];
uint8_t *rxptr;
volatile uint8_t rxlen = 0;

static uint8_t tx_idx = sizeof(p2p_wuart_data_t);

/* === prototypes ========================================================== */

/* === functions =========================================================== */

void wuart_init(void)
{
	timer_init();

	/* === read node configuration data ===================================== */
	/* 1st trial: read from EEPROM */
	if (get_node_config_eeprom(&NodeConfig, 0) == 0)
	{
		/* using EEPROM config */;
	}
	/* 2nd trial: read from FLASHEND */
	else if (get_node_config(&NodeConfig) == 0)
	{
		/* using FLASHEND config */;
	}
	/* 3rd trial: read default values compiled into the application */
	else

	{
		/* using application default config */;
		memcpy_P(&NodeConfig, (PGM_VOID_P) & nc_flash, sizeof(node_config_t));
	}
	
	radio_init(rxbuf, sizeof(rxbuf));
	radio_set_param(RP_CHANNEL(NodeConfig.channel));
	radio_set_param(RP_PANID(NodeConfig.pan_id));
	radio_set_param(RP_SHORTADDR(NodeConfig.short_addr));
	radio_set_param(RP_IDLESTATE(STATE_RX));
}

/*
 * \brief Radio receive
 *
 *  Callback from module "radio"
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
		int8_t ed, uint8_t crc_fail)
{
	static uint8_t lastseq = 255;

	p2p_hdr_t *hdr = (p2p_hdr_t*) frm;
	switch (hdr->cmd)
	{
	case P2P_WUART_DATA:
		if(rxlen > 0){
			/* TODO: error, Rx overrun */
		}
		rxlen = len-sizeof(p2p_wuart_data_t)-2;
		rxptr = &frm[sizeof(p2p_wuart_data_t)];
		break;
	case P2P_PING_REQ:
		break;
	case P2P_PING_CNF:
		break;
	case P2P_JUMP_BOOTL:
		jump_to_bootloader();
		break;

	default:
		break;
	}

	lastseq = hdr->seq;

	return frm;
}

/*
 * \brief Radio error
 *
 *  Callback from module "radio"
 */
void usr_radio_error(radio_error_t err)
{
	/* TODO */
}

/*
 * \brief Radio Tx done
 *
 *  Callback from module "radio"
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
	switch (status)
	{
	case TX_OK:
		break;
	default:
		break;
	} /* switch(status) */
}

void wuart_flush(void)
{
	/* TODO: wait for previous Tx done */
	
	p2p_hdr_t *hdr = (p2p_hdr_t*) txbuf;
	hdr->fcf = 0x8841;
	hdr->seq += 1;
	hdr->pan = NodeConfig.pan_id;
	hdr->dst = 0xffff;
	hdr->src = NodeConfig.short_addr;
	hdr->cmd = P2P_WUART_DATA;
	radio_set_state(STATE_TX);
	radio_set_state(STATE_TXAUTO);
	radio_send_frame(tx_idx + 2, txbuf, 1); /* +2: add CRC bytes (FCF) */
	tx_idx = sizeof(p2p_wuart_data_t);
}

time_t timeout_func(timer_arg_t p)
{
	wuart_flush();
}

int wuart_putc(int c, FILE *stream)
{
	txbuf[tx_idx++] = c;

	if( ('\n' == c) || ('\r' == c) || (tx_idx >= (MAX_FRAME_SIZE - 2)) ) {
		wuart_flush();	
	} else {
		if(NONE_TIMER == thdl){
			thdl = timer_start(timeout_func, timeout_ms, 0);
		}else{
			timer_restart(thdl, timeout_ms);
		}
	}

	return c;
}

int wuart_getc(FILE *stream)
{
	if(rxlen > 0) {
		rxlen--;
		return *rxptr++;
	} else {
		return EOF;
	}
}

/* EOF */
