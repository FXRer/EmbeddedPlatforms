/* Copyright (c) 2014 Axel Wachtler
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
 * @brief Radio Sensor Application
 */

/* === includes ============================================================ */
#include "board.h"
#include "radio.h"
#include "transceiver.h"
#include "sensor.h"
#include "ioutil.h"
#include "rtc.h"
#include "p2p_protocol.h"
#include <string.h>

/* === macros ============================================================== */
#define EOL "\n\r"

#define MASK_LOG_MODE (1)
#define MASK_TX_MODE (2)
#define MASK_RX_MODE (4)

#if ((HIF_TYPE == HIF_NONE) || (FLASHEND <= 8*1024))
# define HAS_MENU (0)
#else
# define HAS_MENU (1)
#endif

#define DEFAULT_UPDATE_INTERVALL (5*60)
#define SW_VERSION (0x11)
/* === types =============================================================== */
typedef struct
{
    uint16_t frmctrl;
    uint8_t seq;
    uint16_t pan;
    uint16_t dst;
    uint16_t src;
    uint8_t data[1];
} rsensor_frame_t;

typedef struct
{
    uint8_t  buf[127];
    uint16_t *psrc_addr;
    uint16_t *psrc_pan;
    uint8_t  *ppay_load;
    uint8_t  ed;
} rsensor_buf_t;

/** application states */
typedef enum
{
    APP_CONFIGURE,
    APP_TRIGGER,
    APP_MEASURE,
    APP_IDLE
} rsensor_state_t;

static const p2p_ping_cnf_t PROGMEM pingrep = {
    .hdr.cmd= P2P_PING_CNF,
    .hdr.fcf = 0x8841,              /* short addressing, frame type: data, no ACK requested */
    .version = SW_VERSION,
    .appname = APP_NAME,
    .boardname = BOARD_NAME,
};

/* === globals ============================================================= */

/* flash configuration */
const node_config_t PROGMEM nc_flash = {
        .short_addr = 0x0001,
        .pan_id = 0xAA55,
        .channel = 17,
        ._reserved_[0] = MASK_LOG_MODE + MASK_TX_MODE + MASK_RX_MODE
    };

static node_config_t NodeConfig;
rsensor_buf_t TxBuf;
rsensor_buf_t RxBuf;

bool LogMode;
bool TxMode;
bool RxMode;
volatile uint16_t UpdateTime;
volatile uint16_t UpdateCnt;
rsensor_state_t RsensorState;

volatile uint16_t SendPingReply = 0xffff;

/* === prototypes ========================================================== */
static void rsensor_init(void);
static void rsensor_configure(void);
static void rsensor_menu(void);
static void rsensor_rtc_cb(void);

/* === functions =========================================================== */
/**
 * Application initialisation
 */
static void rsensor_init()
{
    char cfg_location = '?';

    /* === init ressources === */
    mcu_init();
    hif_init(HIF_DEFAULT_BAUDRATE);
    LED_INIT();
    LED_SET(0);
    rtc_init(rsensor_rtc_cb);
    /* === init all sensors === */
    i2c_init( 4000000UL );
    ow_init();
    create_board_sensors();

    /* === read node configuration data === */
    /* 1st trial: read from EEPROM */
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
        memcpy_P(&NodeConfig, (PGM_VOID_P) & nc_flash, sizeof(node_config_t));
        cfg_location = 'D';
    }

    sei();

    LED_CLR(0);

    /* === init radio === */
    radio_init(RxBuf.buf, sizeof(RxBuf.buf));
    rsensor_configure();

    UpdateTime = 4;

    rtc_start();
    PRINTF("rsensor: board=%s cfg=%c addr=%04x_%04x chan=%d"EOL,
            BOARD_NAME, cfg_location, *TxBuf.psrc_pan, *TxBuf.psrc_addr,
            NodeConfig.channel);

    /* avoid warning in case of PRINTF is an empty macro */
    cfg_location = cfg_location;

}

static void rsensor_configure(void)
{
    p2p_sensor_data_t * pfrm;

    radio_set_state(STATE_OFF);

    radio_set_param(RP_CHANNEL(NodeConfig.channel));
    radio_set_param(RP_PANID(NodeConfig.pan_id));
    radio_set_param(RP_SHORTADDR(NodeConfig.short_addr));

    #ifdef TRX_RX_LNA_EI
    radio_set_param(RP_RX_LNA(1));
    #endif
    #ifdef TRX_TX_PA_EI
    radio_set_param(RP_TX_PA(1));
    #endif
    /* === init TX buffer === */
    pfrm = (p2p_sensor_data_t*)TxBuf.buf;
    pfrm->hdr.fcf = 0x8841;
    pfrm->hdr.dst = 0xffff;
    pfrm->hdr.pan = NodeConfig.pan_id;
    pfrm->hdr.src = NodeConfig.short_addr;
    pfrm->hdr.cmd = P2P_SENSOR_DATA;
    TxBuf.psrc_addr = &pfrm->hdr.src;
    TxBuf.psrc_pan = &pfrm->hdr.pan;
    TxBuf.ppay_load = &pfrm->data[0];
    TxBuf.ed = 0;

    /* === init app flags === */
    LogMode = NodeConfig._reserved_[0] & MASK_LOG_MODE ? true : false;
    TxMode = NodeConfig._reserved_[0] & MASK_TX_MODE ? true : false;
    RxMode = NodeConfig._reserved_[0] & MASK_RX_MODE ? true : false;

    if(RxMode)
    {
        radio_set_param(RP_IDLESTATE(STATE_RX));
        radio_set_state(STATE_RX);
    }
    else
    {
        radio_set_param(RP_IDLESTATE(STATE_SLEEP));
        radio_set_state(STATE_SLEEP);
    }
    UpdateCnt = UpdateTime;
}

/**
 * Configure wuart parameters.
 */
static void rsensor_menu(void)
{
#if HAS_MENU != 0
bool dirty = false, refresh = true;
uint16_t val;
int inchar;
node_config_t nc;
uint16_t idle_cnt = 100;

    memcpy(&nc, &NodeConfig, sizeof(node_config_t) );
    do
    {
        if (refresh)
        {
            PRINT(EOL"MENU:"EOL);
            PRINTF("[a] short addr: 0x%04x"EOL, nc.short_addr);
            PRINTF("[p] panid:      0x%04x"EOL, nc.pan_id);
            PRINTF("[c] channel:    %d"EOL, nc.channel);
            PRINTF("[l] LogMode:    %s"EOL, (nc._reserved_[0] & MASK_LOG_MODE) ? "on" : "off");
            PRINTF("[t] TxMode:     %s"EOL, (nc._reserved_[0] & MASK_TX_MODE) ? "on" : "off");
            PRINTF("[r] RxMode:     %s"EOL, (nc._reserved_[0] & MASK_RX_MODE) ? "on" : "off");
            PRINT("[R] reset changes"EOL
                  "[F] load factory defaults"EOL
                  "[S] save changes and exit"EOL
                  "[Q] discard changes and exit"EOL
                  "==="EOL);
        }

        refresh = true;

        inchar = hif_getc();

        switch(inchar)
        {
            case 'a':
                PRINT("enter short address: ");
                nc.short_addr = hif_get_number(16);
                dirty = true;
                break;
            case 'p':
                PRINT("enter pan id: ");
                nc.pan_id = hif_get_number(16);
                dirty = true;
                break;
            case 'c':
                PRINT("enter channel: ");
                val = hif_get_number(10);
                if (((1UL<<val) & TRX_SUPPORTED_CHANNELS) != 0)
                {
                    nc.channel = val;
                    dirty = true;
                }
                else
                {
                    PRINT("unsupported channel");
                }
                break;
            case 'l':
                PRINT("set LogMode [0=off, 1=on]: ");
                if (hif_get_number(10))
                {
                    nc._reserved_[0] |= MASK_LOG_MODE;
                }
                else
                {
                    nc._reserved_[0] &= ~MASK_LOG_MODE;
                }
                dirty = true;
                break;
            case 't':
                PRINT("set TxMode [0=off, 1=on]: ");
                if (hif_get_number(10))
                {
                    nc._reserved_[0] |= MASK_TX_MODE;
                }
                else
                {
                    nc._reserved_[0] &= ~MASK_TX_MODE;
                }
                dirty = true;
                break;
            case 'r':
                PRINT("set RxMode [0=off, 1=on]: ");
                if (hif_get_number(10))
                {
                    nc._reserved_[0] |= MASK_RX_MODE;
                }
                else
                {
                    nc._reserved_[0] &= ~MASK_RX_MODE;
                }
                dirty = true;
                break;
            case 'R':
                PRINT("reset configuration"EOL);
                memcpy(&nc, &NodeConfig, sizeof(node_config_t) );
                dirty = false;
                break;
            case 'F':
                PRINT("Load factory defaults");
                if (get_node_config(&nc) == 0)
                {
                    /* using FLASHEND config */;
                    PRINT(" from flash end"EOL);
                }
                else
                {
                    /* using application default config */;
                    memcpy_P(&nc, (PGM_VOID_P) & nc_flash, sizeof(node_config_t));
                    PRINT(" from application"EOL);
                }
                dirty = true;
                break;
            case 'S':
                if (dirty)
                {
                    PRINT("save changes and exit"EOL);
                    memcpy(&NodeConfig, &nc, sizeof(node_config_t) );
                    store_node_config_eeprom(&nc, 0);
                }
                else
                {
                    PRINT("no changes to save"EOL);
                }
                break;
            case 'Q':
                PRINT("discard changes and exit"EOL);
                break;

            case EOF:
                idle_cnt --;
                if (idle_cnt < 8)
                {
                    PRINT(".");
                }
                DELAY_MS(100);
                /* fall through */
            default:
                refresh = false;
                break;
        }
        if (refresh)
        {
            idle_cnt = 100;
        }
    }
    while ((inchar != 'S') && (inchar != 'Q') && idle_cnt);
    PRINT(EOL);
#endif
}

/**
 * RTC Handler
 */
static void rsensor_rtc_cb(void)
{
    LED_TOGGLE(2);
    if (UpdateCnt > 1)
    {
        UpdateCnt -= 1;
    }
    else if (RsensorState == APP_IDLE)
    {
        RsensorState = APP_TRIGGER;
        UpdateCnt = UpdateTime;
        if (UpdateTime < DEFAULT_UPDATE_INTERVALL)
        {
            UpdateTime *= 2;
            if (UpdateTime > DEFAULT_UPDATE_INTERVALL)
            {
                UpdateTime = DEFAULT_UPDATE_INTERVALL;
            }
        }
    }
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
    MCU_IRQ_ENABLE();
    radio_set_state(STATE_TXAUTO);
    radio_send_frame(sizeof(p2p_ping_cnf_t)+ 2,
                    (uint8_t*)&_pr, 1);

}

/**
 * radio receive callback function
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
                                  int8_t ed, uint8_t crc_fail)
{
    p2p_sensor_data_t *pfrm;
    if (!crc_fail)
    {
        LED_TOGGLE(1);
        pfrm = (p2p_sensor_data_t*)frm;

        if(pfrm->hdr.cmd == P2P_SENSOR_DATA)
        {
            RxBuf.psrc_addr = &pfrm->hdr.src;
            RxBuf.psrc_pan = &pfrm->hdr.pan;
            RxBuf.ppay_load = pfrm->data;
            RxBuf.ed = ed;
            #if HAS_MENU != 0
            char lbuf[127];
            if (LogMode)
            {
                PRINTF("addr: %04x_%04x,%s ed: %d, seq: %d"EOL,
                        *RxBuf.psrc_pan, *RxBuf.psrc_addr,
                        sensor_decode(RxBuf.ppay_load, lbuf, sizeof(lbuf)),
                        RxBuf.ed, RxBuf.buf[2]);
            }
            #endif
        }
        else if ((pfrm->hdr.cmd == P2P_PING_REQ) &&
                 (pfrm->hdr.dst == NodeConfig.short_addr))
        {
            SendPingReply = pfrm->hdr.src;
        }
        else if (pfrm->hdr.cmd == P2P_JUMP_BOOTL)
        {
            jump_to_bootloader();
        }
    }
    return frm;
}

/**
 * application
 */
int main(void)
{

    int inchar;
    uint8_t data_size;

    rsensor_init();
    RsensorState = APP_TRIGGER;

    /* format tx buffer */
    TxBuf.ppay_load = &TxBuf.buf[10];
    TxBuf.ed = 0;

    do
    {
        inchar = hif_getc();
        if (inchar != EOF)
        {
            RsensorState = APP_CONFIGURE;
        }
        switch(RsensorState)
        {

            case APP_TRIGGER:
                //PRINT("trg"EOL);
                sensor_trigger(ALL_SENSORS, 1);
                RsensorState = APP_MEASURE;
                LED_SET(0);
                DELAY_MS(100);
                LED_CLR(0);
                break;

            case APP_MEASURE:
                //PRINT("meas"EOL);
                data_size = sensor_get(ALL_SENSORS, TxBuf.ppay_load);
                RsensorState = APP_IDLE;
                #if HAS_MENU
                if (LogMode)
                {
                    char line_buf[127];
                    PRINTF("addr*: %04x_%04x,%s ed: none, seq: %d"EOL,
                        *TxBuf.psrc_pan, *TxBuf.psrc_addr,
                        sensor_decode(TxBuf.ppay_load, line_buf, sizeof(line_buf)),
                        TxBuf.buf[2], UpdateTime);
                }
                #endif
                if (TxMode)
                {
                    LED_SET(0);
                    TxBuf.buf[2]++;
                    radio_set_state(STATE_TXAUTO);
                    radio_send_frame(sizeof(rsensor_frame_t) + data_size + 2, TxBuf.buf, 0);
                    LED_CLR(0);
                }
                break;

            case APP_IDLE:
                LED_CLR(2);
                MCU_SLEEP_IDLE();
                LED_SET(2);
                break;

            case APP_CONFIGURE:
                //radio_set_state(STATE_OFF);
                radio_force_state(STATE_OFF);
                rsensor_menu();
                rsensor_configure();
                RsensorState = APP_TRIGGER;
                break;

            default:
                break;
        }

        /* always respond to a ping request */
        if (SendPingReply != 0xffff)
        {
            send_ping_reply();
        }

    } while(1);

    return 0;
}
