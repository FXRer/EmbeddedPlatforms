/* Copyright (c) 2012 Axel Wachtler, Joerg Wunsch
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
#include <avr/pgmspace.h>
#include <string.h>
#include "board.h"
#include "radio.h"
#include "game.h"



/* === macros ============================================================== */

#define FCF(type,ack) (                                                 \
  (2 << 14)                     /* source address mode: short */ |      \
  (2 << 10)                     /* destination address mode: short */ |\
  (1 << 6)                      /* PAN ID compression */ |\
  (ack << 5)                    /* ack request */ |\
  (type)                        /* frame type */ \
                       )

#define TYPE_BEACON 0
#define TYPE_DATA   1
#define TYPE_ACK    2
#define TYPE_CMD    3           /* MAC command */

#define CHANNEL 17
#define SHORTADDR 0x4214
#define PANID 0xF00F

/** Max. number of payload bytes per frame */
#define PAYLD_SIZE        (116)
/** Number of bytes for CRC16 */
#define CRC_SIZE          (sizeof(crc_t))
/** Maximum frame size */
#define TRX_FRAME_SIZE   (sizeof(prot_header_t) +      \
                           PAYLD_SIZE + CRC_SIZE )
/** Index of first payload byte */
#define PAYLD_START       (sizeof(prot_header_t))
/** Index of last payload byte */
#define PAYLD_END         (TRX_FRAME_SIZE - CRC_SIZE)

/* === types =============================================================== */
typedef struct
{
  uint16_t fcf;   /**< Frame control field*/
  uint8_t  seq;   /**< Sequence number */
  uint16_t panid; /**< 16 bit PAN ID */
  uint16_t dst;   /**< 16 bit destination address */
  uint16_t src;   /**< 16 bit source address */
} prot_header_t;

typedef uint16_t crc_t;


/* === globals ============================================================= */
static uint8_t RxFrame[TRX_FRAME_SIZE];
static radio_tx_done_t TxDoneStatus;
static volatile bool TxInProgress;
static volatile uint8_t RadioEvent;

/* === prototypes ========================================================== */

/* === functions =========================================================== */
void xxo_radio_init(void)
{
  radio_init(RxFrame, sizeof(RxFrame));
  radio_set_param(RP_CHANNEL(CHANNEL));
  radio_set_param(RP_IDLESTATE(STATE_RX));
  radio_set_state(STATE_OFF);
  radio_set_param(RP_SHORTADDR(SHORTADDR));
  radio_set_param(RP_PANID(PANID));
  radio_set_param(RP_TXPWR(-4));

  RadioEvent = KEY_NONE;
}

void xxo_radio_printf(const char *fmt, ...)
{
    uint8_t txbuf[TRX_FRAME_SIZE];
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
    TxInProgress = true;
    if (n >= PAYLD_SIZE)
    n = PAYLD_SIZE;

    radio_send_frame(sizeof(prot_header_t) + CRC_SIZE + n, txbuf, 0);

    while (TxInProgress)
    {
        AVR_DO_SLEEP();
    }
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    TxDoneStatus = status;
    TxInProgress = false;
    if (status != TX_OK)
    {
        xxo_radio_printf("TX status: %d\n", status);
    }
}

uint8_t xxo_radio_get_event(void)
{
uint8_t ret;

    cli();
    ret = RadioEvent;
    RadioEvent = KEY_NONE;
    sei();
    return ret;

}

uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
    if (strncmp((char*)&frm[9],"key",3) == 0)
    {
        RadioEvent = frm[14] - '0';
    }
    else if (strncmp((char*)&frm[9],"request",7) == 0)
    {
        RadioEvent = REQUEST_GAME;
    }
    else if (strncmp((char*)&frm[9],"start",5) == 0)
    {
        RadioEvent = START_GAME;
    }

    return frm;
}

