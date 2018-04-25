/* Copyright (c) 2011 Daniel Thiele
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

#include <stdint.h>
#include <string.h>

#include <transceiver.h>
#include <radio.h>

#include "config.h"

#include "sensornwk.h"
#include "ost.h"

static sensornwk_devtype_t dev = SENSORNWK_DEVTYPE_NONE;

/* short address of my coordinator (devtype NODE)
 *
 * 0xFFFF means invalid (not associated)
 */
static uint16_t mycoord = 0xFFFF;

static uint8_t txfrm[MAX_FRAME_SIZE];
static uint8_t rxbuf[MAX_FRAME_SIZE];
static node_config_t nc;

static sensornwk_association_req_t association_req;
static sensornwk_association_rep_t association_rep;

static uint32_t ccafail = 0; /* statistics: track CCA fails */
static uint32_t ackfail = 0; /* statistics: track ACK fails */

static sensornwk_hdr_t *hdr;

static struct
{
    uint8_t idx;
    uint8_t flen;
    uint16_t dest_addr;
} txqueue = { .idx = 0 };

/* default values if no config is found in EEPROM of FLASH */
static const node_config_t PROGMEM nc_flash =
{
    .short_addr = SENSORNWK_DEFAULT_ADDR,
    .pan_id = SENSORNWK_DEFAULT_PANID,
    .channel = SENSORNWK_DEFAULT_CHANNEL
};

void sensornwk_init(sensornwk_devtype_t devtype, uint8_t boardtype)
{
    dev = devtype;

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
        memcpy_P(&nc, &nc_flash, sizeof(node_config_t));
    }

    radio_init(rxbuf, sizeof(rxbuf) / sizeof(rxbuf[0]));
    radio_set_param(RP_CHANNEL(nc.channel));
    radio_set_param(RP_IDLESTATE(STATE_RXAUTO));
    radio_set_param(RP_PANID(nc.pan_id));
    radio_set_param(RP_SHORTADDR(nc.short_addr));

    if (SENSORNWK_DEVTYPE_COORD == dev)
    {
        /* already in RXAUTO */
    }
    else if (SENSORNWK_DEVTYPE_NODE == dev)
    {
        radio_set_state(STATE_SLEEP);
    }
    else
    {
        /* unhandled */
    }

    hdr = (sensornwk_hdr_t*) txfrm;
    hdr->FCF = SENSORNWK_DEFAULT_FCF; /* data frame, panid compr, no ACK requested */
    hdr->srcaddr = nc.short_addr;
    hdr->destpanid = nc.pan_id;
    hdr->destaddr = SENSORNWK_DEFAULT_COORD_ADDR; /* as long as no other coordinator is set */
    hdr->boardtype = boardtype;

    association_req.hdr.FCF = 0x8841;
    association_req.hdr.destpanid = 0xFFFF; /* broadcast */
    association_req.hdr.destaddr = 0xFFFF; /* broadcast */
    association_req.hdr.srcaddr = nc.short_addr;
    association_req.hdr.boardtype = boardtype;
    association_req.hdr.frametype = SENSORNWK_FRAMETYPE_ASSOCIATION_REQ;

    association_rep.hdr.FCF = SENSORNWK_DEFAULT_FCF;
    association_rep.hdr.destpanid = nc.pan_id;
    association_rep.hdr.srcaddr = nc.short_addr;
    association_rep.hdr.boardtype = boardtype;
    association_rep.hdr.frametype = SENSORNWK_FRAMETYPE_ASSOCIATION_REP;
}

node_config_t* sensornwk_get_config(void)
{
    return &nc;
}

/*
 * \brief Called on Rx slot timeout
 * No matter if a frame was received or not
 */
void tmr_rxslot(void)
{
    radio_set_state(STATE_SLEEP);
    cb_sensornwk_done();
}

/*
 * \brief Callback of libradio_
 */
uint8_t* usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
                                 int8_t ed, uint8_t crc_fail)
{
    sensornwk_hdr_t *lhdr = (sensornwk_hdr_t*) frm;

    if (SENSORNWK_DEVTYPE_NODE == dev)
    {
        ost_stop();
        switch (lhdr->frametype)
        {
        case SENSORNWK_FRAMETYPE_ASSOCIATION_REP:
            mycoord = lhdr->srcaddr;
            ((sensornwk_hdr_t*)txfrm)->destaddr = lhdr->srcaddr;
            cb_sensornwk_association_cnf(lhdr->srcaddr);
            break;
        default:
            cb_sensornwk_rx(len, frm, lqi, ed);
            break;
        } /* switch (hdr->frametype) */
    }
    else if (SENSORNWK_DEVTYPE_COORD == dev)
    {
        switch (lhdr->frametype)
        {
        case SENSORNWK_FRAMETYPE_ASSOCIATION_REQ:
            association_rep.hdr.destaddr = lhdr->srcaddr;
            radio_set_state(STATE_TXAUTO);
            radio_send_frame(sizeof(sensornwk_association_rep_t), (uint8_t*)&association_rep, 0);
            cb_sensornwk_association_cnf(lhdr->srcaddr);
            break;
        default:
            if ((0 < txqueue.idx) && (lhdr->srcaddr == txqueue.dest_addr))
            {
                radio_set_state(STATE_TXAUTO);
                radio_send_frame(txqueue.flen, txfrm, 0);
                txqueue.idx = 0;
            }
            cb_sensornwk_rx(len, frm, lqi, ed);
            break;
        } /* switch (hdr->frametype) */
    }

    return frm;
}

/*
 * \brief Callback of libradio_
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
    switch (status)
    {
    case TX_CCA_FAIL:
        ccafail++;
        break;
    case TX_NO_ACK:
        ackfail++;
        break;
    default:
        break;
    }

    radio_set_state(STATE_RXAUTO);

    if (SENSORNWK_DEVTYPE_NODE == dev)
    {
        ost_start(tmr_rxslot, SENSORNWK_TIME_RX_SLOT_US, OST_ONESHOT);
    }
    else if (SENSORNWK_DEVTYPE_COORD == dev)
    {
        cb_sensornwk_done();
    }
    else
    {

    }
}

void sensornwk_tx_association_req(void)
{
    /* association_req.hdr.
     * fields are assigned in _init() function
     */
    radio_set_state(STATE_TXAUTO);
    radio_send_frame(sizeof(sensornwk_association_req_t), (uint8_t*)&association_req, 0);
}

void sensornwk_tx(uint8_t *data, uint8_t len, uint8_t frametype,
                  uint8_t ack_req)
{
    uint8_t flen = sizeof(sensornwk_hdr_t);

    sensornwk_hdr_t *hdr = (sensornwk_hdr_t*) txfrm;

    hdr->frametype = frametype;

    memcpy((uint8_t*) &txfrm[flen], data, len);
    flen += len;
    flen += sizeof(sensornwk_ftr_t);

    radio_set_state(STATE_TXAUTO);
    radio_send_frame(flen, txfrm, 0);
}

uint8_t sensornwk_txqueue(uint16_t dest_addr, uint8_t *data, uint8_t len,
                          uint8_t frametype)
{
    uint8_t flen = sizeof(sensornwk_hdr_t);
    sensornwk_hdr_t *hdr = (sensornwk_hdr_t*) txfrm;

    if (0 == txqueue.idx)
    {

        hdr->frametype = frametype;
        hdr->destaddr = dest_addr;

        memcpy((uint8_t*) &txfrm[flen], data, len);
        flen += len;
        flen += sizeof(sensornwk_ftr_t);

        txqueue.dest_addr = dest_addr;
        txqueue.flen = flen;
        txqueue.idx = 1;

        return 0;
    }
    else
    {
        return 1;
    }
}

/* EOF */
