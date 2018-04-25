/* Copyright (c) 2007, 2008 Axel Wachtler
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

#ifndef WGPIO_H
#define WGPIO_H

/* === includes ============================================================ */
#include "transceiver.h"
#include "ioutil.h"
#include "timer.h"

/* === macros ============================================================== */
#if defined(stb230) || defined(stb230b)
    #define ACTOR_INIT() DDRE |= (1<<PE4)
    #define ACTOR_ON()   PORTE |= (1<<PE4)
    #define ACTOR_OFF()  PORTE &= ~(1<<PE4)
#else
    #define ACTOR_INIT()
    #define ACTOR_ON()
    #define ACTOR_OFF()
#endif

#define PANID   (0x4f49)
#define CHANNEL (15)
#define FCTL_KEY_FRAME ( FCTL_DATA |  FCTL_IPAN  | FCTL_DST_SHORT | FCTL_SRC_SHORT )


#define SHORT_PRESS_TICKS  (MSEC(10))
#define LONG_PRESS_TICKS   (MSEC(1000))

#define NONE_PRESS   (0)
#define SHORT_PRESS  (1)
#define LONG_PRESS   (2)

#define IGNORE       (255)
/* === types =============================================================== */
/**
 * actor context data structure
 */
typedef struct
{
    uint8_t state;
    uint8_t role;
} actor_ctx_t;

typedef enum SHORTENUM
{
    NONE = 0x00,
    CMD_SWITCH = 's', /**< command code for a switch event */
    CMD_STATUS = 'r', /**< command code for a status event */
    CMD_TX_DONE_OK = 'T',
    CMD_TX_DONE_NOK = 't',
    CMD_QRY_STATUS = 'q',
} event_code_t;

typedef struct
{
    event_code_t cmd;
    uint8_t state;
} event_t;

typedef enum SHORTENUM
{
    APP_RUNNING,
    APP_STATUS_PENDING,
    APP_CONFIG,
    TX_SWITCH_IN_PROGRESS,
    TX_STATUS_IN_PROGRESS,
    APP_SLEEP,
    APP_TRANSMIT,
    APP_RECEIVE,
} app_state_t;

typedef struct
{
    uint16_t frmctl;
    uint8_t  seq;
    uint16_t dstpan;
    uint16_t dstaddr;
    uint16_t srcaddr;
    event_code_t  cmd;
    uint8_t  state;
    uint16_t crc;
} key_frame_t;

typedef struct
{
    uint8_t key;
    uint8_t tmo;
    uint8_t slpmode;
    uint8_t qrycnt;
    event_t rx;
    event_t tx;
    app_state_t appstate;
} app_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
static void wgpio_init(void);
static void wgpio_idle(uint8_t idlemode);
static void wgpio_actor_update(uint8_t state);
static bool wgpio_send_status(event_code_t cmd, uint8_t state);

static void state_app_running(app_ctx_t *pevents);
static void state_tx_switch_in_progress(app_ctx_t *ctx);
static void state_tx_status_in_progress(app_ctx_t *ctx);
static void state_app_status_pending(app_ctx_t *ctx);
static void state_app_config(app_ctx_t *ctx);

static void status_led_handle(void);
static void status_led_config(uint8_t val, uint8_t blnk);

static inline void transceiver_init(uint8_t channel, uint16_t addr)
{

    trx_io_init(DEFAULT_SPI_RATE);
    LED_SET_VALUE(1);

    trx_init();
    LED_SET_VALUE(2);

    if(trx_identify() != 1)
    {
        while(1);
    };
    LED_SET_VALUE(3);

#if defined(TRX_IRQ_TRX_END)
    trx_reg_write(RG_IRQ_MASK, TRX_IRQ_TRX_END);
#elif defined(TRX_IRQ_RX_END) && defined(TRX_IRQ_TX_END)
    trx_reg_write(RG_IRQ_MASK, TRX_IRQ_RX_END | TRX_IRQ_TX_END);
#else
#  error "Unknown TRX_END IRQ bits"
#endif
    trx_bit_write(SR_TX_AUTO_CRC_ON, 1);
    trx_bit_write(SR_CHANNEL, channel);
    trx_set_shortaddr(addr);
    trx_set_panid(PANID);

    LED_SET_VALUE(0);
    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
}

void inline transceiver_send_frame(uint8_t *pdata, uint8_t sz)
{
    trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
    trx_reg_write(RG_TRX_STATE, CMD_TX_ARET_ON);
    trx_frame_write(sz, pdata);
    TRX_SLPTR_HIGH();
    TRX_SLPTR_LOW();
}

/**
 * simple debouncing of key "0" with 4 states
 *
 * @return
 *     - @ref NONE_PRESS no key pressed
 *     - @ref SHORT_PRESS key pressed normal (~100ms)
 *     - @ref LONG_PRESS very long (>1s)
 */
static uint8_t debounce_key0(void)
{
uint8_t ret, tmp;
static uint16_t ocnt=0;

    ret = NONE_PRESS;
    tmp = (KEY_GET() & 1);
    if (tmp != 0)
    {
        ocnt ++;
        if(ocnt > LONG_PRESS_TICKS)
        {
            ocnt = LONG_PRESS_TICKS;
        }
    }
    else
    {
        if(ocnt > SHORT_PRESS_TICKS)
        {
            ret = SHORT_PRESS;
        }
        if(ocnt >= LONG_PRESS_TICKS)
        {
            ret = LONG_PRESS;
        }
        ocnt = 0;
    }
    return ret;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif  /* #ifndef WGPIO_H */
