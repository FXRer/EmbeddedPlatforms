/* Copyright (c) 2007 - 2012 Axel Wachtler
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
 * @addtogroup grpAppWuart
 * @{
 *
 * @file
 * @brief Implementation of a Wireless UART
 *
 * This Application implements a wireless UART bridge.
 *
 */

#include <string.h>
#include "board.h"
#include "transceiver.h"
#include "radio.h"
#include "ioutil.h"
#include "wuart.h"

/* === Macros ======================================== */
#define SW_VERSION (0x03)
#define LED_RX (0)
#define LED_TX (1)

/* === Types ========================================= */

/* === globals ====================================== */
static const node_config_t PROGMEM nc_flash = { .short_addr = DEFAULT_SHORT_ADDRESS,
                                         .pan_id = DEFAULT_PAN_ID,
                                         .channel = DEFAULT_RADIO_CHANNEL };
static node_config_t NodeConfig;
static uint16_t PeerAddress;
static uint32_t FrameFromNode = 0;
static bool ChatMode = false;

static volatile uint8_t RxbufIdx = 0;
static wuart_buffer_t RxBuf[2];

static wuart_buffer_t TxBuf;
static volatile bool TxPending;

static volatile wuart_state_t WuartState;
static volatile int16_t EscapeTmoCounter;
static volatile int8_t LastTransmitCounter;
static volatile uint16_t SendPingReply = 0xffff;

static const p2p_ping_cnf_t PROGMEM pingrep = {
    .hdr.cmd= P2P_PING_CNF,
    .hdr.fcf = 0x8841,              /* short addressing, frame type: data, no ACK requested */
    .version = SW_VERSION,
    .appname = APP_NAME,
    .boardname = BOARD_NAME,
};


/* === prototypes ================================= */

/* === functions ================================== */

/**
 * @brief Initialize MCU ressources.
 */
static void wuart_init(void)
{
char cfg_location = '?';

    mcu_init();

    /* setup peripherals */
    LED_INIT();
    LED_SET_VALUE(LED_TX);
    LED_SET_VALUE(LED_RX);
    KEY_INIT();
    TIMER_INIT();

    hif_init(HIF_DEFAULT_BAUDRATE);

    /* get configuration data */

    /* 1st trial: read from internal EEPROM */
    if (get_node_config_eeprom(&NodeConfig, 0) == 0)
    {
        /* using EEPROM config */;
        cfg_location = 'E';
    }
    /* 2nd trial: read from FLASHEND */
    else if (get_node_config(&NodeConfig) == 0)
    {
        /* using FLASHEND config */;
        cfg_location = 'F';
    }
    /* 3rd trial: read default values compiled into the application */
    else
    {
        /* using application default config */;
        memcpy_P(&NodeConfig, &nc_flash, sizeof(node_config_t) );
        cfg_location = 'D';
    }

    if (((1UL<<NodeConfig.channel) & TRX_SUPPORTED_CHANNELS) == 0)
    {
        /* fall back to DEFAULT_CHANNEL,*/
        PRINTF("Invalid channel: %d use now: %d",
                NodeConfig.channel, DEFAULT_RADIO_CHANNEL);
        NodeConfig.channel = DEFAULT_RADIO_CHANNEL;
    }

    /* @todo: add application config structure for eeprom */
    PeerAddress = NodeConfig.short_addr;

    RxbufIdx = 0;
    memset(RxBuf, 0, sizeof(RxBuf));
    memset(&TxBuf, 0, sizeof(TxBuf));

    /* radio setup */
    radio_init(RxBuf[RxbufIdx].data.buf, UART_FRAME_SIZE);
    configure_radio();
    MCU_IRQ_ENABLE();

    PRINTF("Wuart %d.%d chan=%d node=%04x:%04x radio=%02x.%02x cfg %c"EOL,
            (SW_VERSION >> 4), (SW_VERSION &0x0f),
            NodeConfig.channel,
            NodeConfig.pan_id,
            NodeConfig.short_addr,
            trx_reg_read(RG_PART_NUM),
            trx_reg_read(RG_VERSION_NUM),
            cfg_location);


    /*Don't care about the revision, verify if PART_NUM does match. */

    if (trx_identify() & INVALID_PART_NUM)
    {
        PRINTF("radio does not match: expected %d.%d got %d.%d"EOL,
              RADIO_PART_NUM, RADIO_VERSION_NUM,
              trx_reg_read(RG_PART_NUM),
              trx_reg_read(RG_VERSION_NUM)
              );
    }
}

static void configure_radio(void)
{

    MCU_IRQ_DISABLE();
    /* initialization after Update */

    TxBuf.start = sizeof(p2p_wuart_data_t);
    TxBuf.end = UART_FRAME_SIZE - CRC_SIZE;

    FILL_P2P_HEADER_NOACK((&TxBuf.data.hdr.hdr),
                          NodeConfig.pan_id,
                          PeerAddress,
                          NodeConfig.short_addr,
                          P2P_WUART_DATA);
    TxBuf.data.hdr.mode = 0x55;

    radio_set_param(RP_IDLESTATE(STATE_RX));
    radio_set_param(RP_CHANNEL(NodeConfig.channel));
    radio_set_param(RP_SHORTADDR(NodeConfig.short_addr));
    radio_set_param(RP_PANID(NodeConfig.pan_id));
    radio_set_state(STATE_RX);
    MCU_IRQ_ENABLE();
}

/**
 * Configure wuart parameters.
 */
void do_configure_dialog(void)
{
bool refresh = true, chatmode;
uint16_t val, peer;
int inchar;

node_config_t nc;

    memcpy(&nc, &NodeConfig, sizeof(node_config_t) );
    peer = PeerAddress;
    chatmode = ChatMode;
    do
    {
        if (refresh)
        {
            PRINT(EOL"MENU:"EOL);
            PRINTF("[a] node address: 0x%04x"EOL, nc.short_addr);
            PRINTF("[p] peer address: 0x%04x"EOL, peer);
            PRINTF("[P] PAN ID:       0x%04x"EOL, nc.pan_id);
            PRINTF("[c] channel:      %d"EOL, nc.channel);
            PRINTF("[C] chat mode:      %d"EOL, chatmode);
            PRINT("[r] reset changes"EOL
                  "[e] save and exit"EOL
                  "[f] reset to factory defaults"EOL
                  "[q] discard changes and exit"EOL
                  "==="EOL);
        }
        refresh = true;

        inchar = hif_getc();

        switch(inchar)
        {
            case 'a':
                PRINT("enter short address:");
                nc.short_addr = hif_get_number(16);
                break;
            case 'p':
                PRINT("enter peer address:");
                peer = hif_get_number(16);
                break;
            case 'P':
                PRINT("enter PAN ID:");
                nc.pan_id = hif_get_number(16);
                break;
            case 'c':
                PRINT("enter channel:");
                val = hif_get_number(10);
                if (((1UL<<val) & TRX_SUPPORTED_CHANNELS) != 0)
                {
                    nc.channel = val;
                }
                else
                {
                    PRINT("unsupported channel");
                }
                break;
            case 'C':
                PRINT("set chatmode:");
                val = hif_get_number(10);
                chatmode = val ? true : false;
                break;
            case 'r':
                memcpy(&nc, &NodeConfig, sizeof(node_config_t) );
                break;
            case 'e':
                PRINT("save config and exit"EOL);
                memcpy(&NodeConfig, &nc, sizeof(node_config_t) );
                store_node_config_eeprom(&nc, 0);
                PeerAddress = peer;
                ChatMode = chatmode;
                break;
            case 'f':
                PRINT("reset to factory defaults"EOL);
                /* delete eeprom record */
                eeprom_write_byte( (uint8_t *)0, 0 );
                eeprom_write_byte( (uint8_t *)1, 0 );
                memcpy_P(&nc, &nc_flash, sizeof(node_config_t) );
                memcpy_P(&NodeConfig, &nc_flash, sizeof(node_config_t) );
                peer = PeerAddress = nc.short_addr;
                chatmode = ChatMode = false;
                break;
            case 'q':
                PRINT("discard changes and exit"EOL);
                break;
            default:
                refresh = false;
                break;
        }
    }
    while ((inchar != 'e') && (inchar != 'q'));
}

static void inline send_ping_reply(void)
{
    p2p_ping_cnf_t _pr;
    memcpy_P(&_pr, &pingrep, sizeof(p2p_ping_cnf_t));
    MCU_IRQ_DISABLE();
    _pr.hdr.src = NodeConfig.short_addr;
    _pr.hdr.pan = NodeConfig.pan_id;
    _pr.hdr.dst = SendPingReply;
    SendPingReply = 0xffff;
    TxPending = true;
    MCU_IRQ_ENABLE();
    radio_set_state(STATE_TXAUTO);
    radio_send_frame(sizeof(p2p_ping_cnf_t)+ 2,
                    (uint8_t*)&_pr, 1);

}

/**
 * Main function of the WUART application.
 *
 */
int main(void)
{
int inchar;
uint8_t pluscnt = 0;
bool do_send;
wuart_buffer_t *prx;
uint32_t from;

    wuart_init();
    WuartState = DATA_MODE;
    do
    {
        inchar = hif_getc();
        /* state machine to detect Hayes '302 break condition */
        switch (WuartState)
        {
            case DATA_MODE:
                if (EOF == inchar)
                {
                    EscapeTmoCounter = 255;
                    WuartState = WAIT_GUARDTIME;
                }
                break;

            case WAIT_GUARDTIME:
                if (EOF != inchar)
                {
                    EscapeTmoCounter = 0;
                    WuartState = DATA_MODE;
                }
                else if (EscapeTmoCounter == 0)
                {
                    if (pluscnt != 3)
                    {
                        pluscnt = 0;
                        WuartState = WAIT_PLUS;
                    }
                    else
                    {
                        pluscnt = 0;
                        WuartState = DO_CONFIGURE;
                    }
                }
                break;

            case WAIT_PLUS:
                if (inchar == '+')
                {
                    pluscnt ++;
                    if (pluscnt == 3)
                    {
                        WuartState = WAIT_GUARDTIME;
                    }
                }
                else if (EOF != inchar)
                {
                    WuartState = DATA_MODE;
                }
                break;

            case DO_CONFIGURE:
                radio_set_state(STATE_OFF);
                do_configure_dialog();
                configure_radio();
                WuartState = DATA_MODE;
                break;
        }

        if (WuartState != DO_CONFIGURE)
        {
            /* handle data "coming from air" */
            prx = &RxBuf[RxbufIdx^1];
            if (prx->start <= prx->end)
            {
                if(ChatMode)
                {
                    from = GetFullSrcAddr(&prx->data.buf[0]);
                    if (FrameFromNode != from)
                    {
                        PRINTF("\n<%08lx>\n", from);
                        FrameFromNode = from;
                    }
                }
                while(prx->start <= prx->end)
                {
                    hif_putc(prx->data.buf[prx->start++]);
                }
            }
            MCU_IRQ_DISABLE();
            prx->end = 0;
            MCU_IRQ_ENABLE();

            /* handle data "going to air" */
            do_send = false;
            if ((TxBuf.start + pluscnt) >= TxBuf.end)
            {
                do_send = true;
            }
            else if ((LastTransmitCounter == 0) &&
                     (TxBuf.start > sizeof(p2p_wuart_data_t)))
            {
                do_send = true;
            }

            if (do_send == true)
            {
                /* wait here, while TX is busy */
                while(TxPending == true)
                    ;
                radio_set_state(STATE_TXAUTO);
                TxPending = true;
                TxBuf.data.hdr.hdr.seq ++;
                radio_send_frame(TxBuf.start + CRC_SIZE, TxBuf.data.buf, 0);
                if (ChatMode)
                {
                    from = GetFullSrcAddr(&TxBuf.data.buf[0]);
                    if (FrameFromNode != from)
                    {
                        PRINTF("\n<*%08lx>\n", from);
                        FrameFromNode = from;
                    }
                    uint8_t idx = sizeof(p2p_wuart_data_t);
                    while(idx < TxBuf.start)
                    {
                        hif_putc(TxBuf.data.buf[idx++]);
                    }
                }
                LastTransmitCounter = 100;
                TxBuf.start = sizeof(p2p_wuart_data_t);
                TxBuf.end = UART_FRAME_SIZE - CRC_SIZE;
                LED_SET(LED_TX);
            }
        }

        if (WuartState == DATA_MODE)
        {
            /* handle data coming from hif/uart */
            while (pluscnt)
            {
                --pluscnt;
                TxBuf.data.buf[TxBuf.start++] = '+';
            }
            if (EOF != inchar)
            {
                /* fill in new bytes */
                TxBuf.data.buf[TxBuf.start++] = (uint8_t)inchar;
            }
        }

        /* always respond to a ping request */
        if ((SendPingReply != 0xffff) && (TxPending == false))
        {
            send_ping_reply();
        }
    }
    while(1);

    return 0;
}

void usr_radio_error(radio_error_t err)
{
    PRINTF("radio error %d", err);
}


/**
 * Implementation of callback function @ref usr_radio_tx_done.
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
    LED_CLR(LED_TX);
    TxPending = false;
}

/**
 * Implementation of callback function for wuart.
 * @copydoc    usr_radio_receive_frame
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
                                  int8_t ed, uint8_t crc)
{
p2p_hdr_t *pfrm;

    MCU_IRQ_DISABLE();
    if ((crc == 0) && (len > sizeof(p2p_hdr_t)))
    {
        pfrm = (p2p_hdr_t*) frm;
        if (pfrm->cmd == P2P_WUART_DATA)
        {
            if( RxBuf[(RxbufIdx^1)].end == 0)
            {
                LED_TOGGLE(LED_RX);
                /* yes, we can do a swap */
                RxBuf[RxbufIdx].start = sizeof(p2p_wuart_data_t);
                RxBuf[RxbufIdx].end = len - CRC_SIZE - 1;
                RxbufIdx ^= 1;
                frm = RxBuf[RxbufIdx].data.buf;
            }
        }
        else if (pfrm->cmd == P2P_JUMP_BOOTL)
        {
            jump_to_bootloader();
        }
        else if ((pfrm->cmd == P2P_PING_REQ) &&
                 (pfrm->dst == NodeConfig.short_addr))
        {
            SendPingReply = pfrm->src;
        }
    }
    MCU_IRQ_ENABLE();
    return frm;
}

/**
 * Timer ISR
 */
ISR(TIMER_IRQ_vect)
{
    if (EscapeTmoCounter > 0)
    {
        EscapeTmoCounter--;
    }

    if (LastTransmitCounter > 0)
    {
        LastTransmitCounter--;
    }
}

/** @} */
