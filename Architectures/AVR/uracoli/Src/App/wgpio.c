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

/* $Id$ */
/**
 * @file
 * @brief Implementation of the @ref grpAppWgpio.
 *
 * This is a Wireless General Purpose IO Application
 * a.k.a. a light switch application.
 *
 * The application can run in two modes:
 *  - (MP) mains powered :
 *         In this mode the receiver always on.
 *  - (RC) remote control:
 *         Battery powered remote control with power down on idle.
 *
 * @note The RC mode can be eanbled on those boards, where the
 *       the control key is wired with an interrupt pin of the MCU.
 *       All boards with memory mapped keys are not capable of
 *       entering the RC mode.
 *
 * With a long key press, you get into the configuration mode,
 * where the status LED blinks 3 times and the Actor LED displays
 * the current operation mode:
 *    - ActorLed on : MP mode (reset value)
 *    - ActorLed off : RC mode.
 *
 * @ingroup grpAppWgpio
 */

/* === includes ========================================== */
#include "wgpio.h"

/* === macros ============================================ */
#define AWAKE_TIME   MSEC(1000)
/* === types ============================================= */
/* === globals =========================================== */
/* prog ctx r/w */
actor_ctx_t ActorCtx;
uint8_t Led1Blnk = 0;

/*  IRQ Variables r/w */
volatile uint16_t TmoCounter = 0;

/*  IRQ Variables wo */
volatile event_t TrxEvent;
volatile uint8_t KeyEvent = 0;
volatile uint8_t TmoEvent = 0;
/*  IRQ Variables ro */
volatile app_state_t TrxState;

/* === prototypes ======================================== */

/* === functions ========================================= */

/**
 * Main function of WGPIO application.
 */
int main(void)
{
app_ctx_t AppCtx = {0};

    trap_if_key_pressed();
    wgpio_init();
    AppCtx.appstate = APP_RUNNING;
    while(1)
    {
        /* free irq variables and work with actual copies */
        cli();
        AppCtx.rx.cmd = TrxEvent.cmd;
        AppCtx.rx.state = TrxEvent.state;
        TrxEvent.cmd = NONE;
        AppCtx.key = KeyEvent;
        KeyEvent = NONE_PRESS;
        AppCtx.tmo = TmoEvent;
        TmoEvent = 0;
        sei();

        switch(AppCtx.appstate)
        {
            case APP_RUNNING:
                state_app_running(&AppCtx);
                break;

            case TX_SWITCH_IN_PROGRESS:
                state_tx_switch_in_progress(&AppCtx);
                break;

            case TX_STATUS_IN_PROGRESS:
                state_tx_status_in_progress(&AppCtx);
                break;

            case APP_STATUS_PENDING:
                state_app_status_pending(&AppCtx);
                break;

            case APP_CONFIG:
                state_app_config(&AppCtx);
                break;

            default:
                break;
        }

        if (AppCtx.tx.cmd != NONE && TrxState != APP_TRANSMIT)
        {
            wgpio_send_status(AppCtx.tx.cmd, AppCtx.tx.state);
            AppCtx.tx.cmd = NONE;
        }

        wgpio_idle(AppCtx.slpmode);
    }
    return 0;
}

/**
 * Initialization function.
 */
static void wgpio_init(void)
{
    mcu_init();
    LED_INIT();
    KEY_INIT();
    TIMER_INIT();
#if defined(SLEEP_ON_KEY)
    SLEEP_ON_KEY_INIT();
#endif
    transceiver_init(CHANNEL, 0);
    TrxState = APP_RECEIVE;
    sei();
    ACTOR_INIT();
    ActorCtx.role = 1;
}

static void wgpio_idle(uint8_t idlemode)
{
    if (idlemode == SLEEP_MODE_IDLE)
    {
        MCU_SLEEP_IDLE();
    }
    else if (idlemode == SLEEP_MODE_PWR_DOWN)
    {
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        TRX_SLPTR_HIGH();
        LED_SET_VALUE(0);
        #if defined(SLEEP_ON_KEY)
            SLEEP_ON_KEY();
        #else
            MCU_SLEEP_IDLE();
        #endif
        TRX_SLPTR_LOW();
    }
}

#if defined(SLEEP_ON_KEY)
    ISR(SLEEP_ON_KEY_vect)
    {
    }
#endif

static void wgpio_actor_update(uint8_t state)
{
    cli();
    if (state != 255)
    {
        ActorCtx.state = state;
    }
    else
    {
        ActorCtx.state ^= 1;
    }
    if(ActorCtx.state)
    {
        LED_SET(0);
        ACTOR_ON();
    }
    else
    {
        LED_CLR(0);
        ACTOR_OFF();
    }
    sei();
}

/**
 * @brief Send the current key status
 * @param AppCtx.keytat key status byte that is to send.
 */
static bool wgpio_send_status(event_code_t cmd, uint8_t state)
{
static uint8_t seq = 0;
key_frame_t txfrm;

    txfrm.frmctl = FCTL_KEY_FRAME;
    txfrm.seq = seq++;
    txfrm.dstpan = PANID;
    txfrm.dstaddr = 0;
    txfrm.srcaddr = 0;
    txfrm.cmd = cmd;
    txfrm.state = state;
    transceiver_send_frame((uint8_t*)&txfrm,sizeof(txfrm));
    TrxState = APP_TRANSMIT;
    return true;
}

static void state_app_running(app_ctx_t *events)
{
    events->slpmode = SLEEP_MODE_IDLE;

    if (events->key == LONG_PRESS)
    {
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        LED_SET_VALUE(ActorCtx.role);
        events->appstate = APP_CONFIG;
        status_led_config(1, 5);
    }
    else if (events->key == SHORT_PRESS)
    {
        events->appstate = TX_SWITCH_IN_PROGRESS;
        events->tx.cmd = CMD_SWITCH;
        if (ActorCtx.role == 1)
        {
            wgpio_actor_update(255);
            events->tx.state = ActorCtx.state;
        }
        else
        {
            events->tx.state = 255;
        }
    }
    else if (events->rx.cmd == CMD_SWITCH)
    {
        if (ActorCtx.role == 1)
        {
            events->appstate = TX_STATUS_IN_PROGRESS;
            events->tx.cmd = CMD_STATUS;
            events->tx.state = ActorCtx.state;
            wgpio_actor_update(events->rx.state);
        }
        status_led_config(0, 0);
    }
    else if (events->rx.cmd == CMD_QRY_STATUS)
    {
        if (ActorCtx.role == 1)
        {
            events->appstate = TX_STATUS_IN_PROGRESS;
            events->tx.cmd = CMD_STATUS;
            events->tx.state = ActorCtx.state;
        }
    }
    else if ((events->tmo) && (ActorCtx.role == 0))
    {
        events->slpmode = SLEEP_MODE_PWR_DOWN;
    }
}


static void state_tx_switch_in_progress(app_ctx_t *ctx)
{
    if (ctx->rx.cmd == CMD_TX_DONE_OK)
    {
        ctx->appstate = APP_STATUS_PENDING;
        TmoCounter = MSEC(150);
        ctx->qrycnt = 4;
        status_led_config(1, 0);
    }
    else if (ctx->rx.cmd == CMD_TX_DONE_NOK)
    {
        ctx->appstate = APP_RUNNING;
        TmoCounter = AWAKE_TIME;
        status_led_config(0, 5);
    }
}


static void state_tx_status_in_progress(app_ctx_t *ctx)
{
    /* for a status frame we do not care about the outcome */
    if (ctx->rx.cmd == CMD_TX_DONE_OK ||
        ctx->rx.cmd == CMD_TX_DONE_NOK )
    {

        TmoCounter = AWAKE_TIME;
        ctx->appstate = APP_RUNNING;
    }
}


static void state_app_status_pending(app_ctx_t *ctx)
{
    if (ctx->rx.cmd == CMD_STATUS)
    {
        cli();
        TmoCounter = 0;
        sei();
        ctx->qrycnt = 0;
        TmoCounter = AWAKE_TIME;
        ctx->appstate = APP_RUNNING;
        status_led_config(0,0);
    }
    else if (ctx->tmo != 0)
    {
        status_led_config(0, 7);
        if (ctx->qrycnt > 0)
        {
            ctx->qrycnt --;
            cli();
                TmoCounter = MSEC(150);
            sei();
            ctx->tx.cmd = CMD_QRY_STATUS;
        }
        else
        {
            TmoCounter = AWAKE_TIME;
            ctx->appstate = APP_RUNNING;
            LED_SET(1);
        }
    }
}


static void state_app_config(app_ctx_t *ctx)
{
    if (ctx->key == SHORT_PRESS)
    {
#if defined(SLEEP_ON_KEY)
        ActorCtx.role ^= 1;
        if(ActorCtx.role)
        {
            LED_SET(0);
        }
        else
        {
            LED_CLR(0);
        }
#else
        ActorCtx.role = 1;
        status_led_config(1, 4);
#endif
    }
    else if (ctx->key == LONG_PRESS)
    {
        TmoCounter = AWAKE_TIME;
        ctx->appstate = APP_RUNNING;
        status_led_config(0, 5);
        /* if role is note remote control, RDX is always on */
        if (ActorCtx.role == 1)
        {
            trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
        }
    }
}

/**
 * Transceiver IRQ routine.
 */
#if defined(DOXYGEN)
void TRX_IRQ_vect()
#elif !defined(TRX_IF_RFA1)
ISR(TRX_IRQ_vect)
{
uint8_t cause;
key_frame_t rxfrm;

    cause = trx_reg_read(RG_IRQ_STATUS);
    if (TrxState == APP_TRANSMIT)
    {
        cause = trx_bit_read(SR_TRAC_STATUS);
        TrxState = APP_RECEIVE;
        if (cause == TRAC_SUCCESS)
        {
            TrxEvent.cmd = CMD_TX_DONE_OK;
        }
        else
        {
            TrxEvent.cmd = CMD_TX_DONE_NOK;
        }
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
    }
    else if(TrxState == APP_RECEIVE)
    {
        /* protect frame buffer */
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);
        cause = trx_frame_read ((uint8_t*)&rxfrm, sizeof(rxfrm), NULL);
        TrxEvent.cmd = rxfrm.cmd;
        TrxEvent.state = rxfrm.state;
        trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
    }
}
#endif

#if defined(TRX_IF_RFA1)
ISR(TRX24_RX_END_vect)
{
    key_frame_t rxfrm;

    trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
    trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);
    (void)trx_frame_read ((uint8_t*)&rxfrm, sizeof(rxfrm), NULL);
    TrxEvent.cmd = rxfrm.cmd;
    TrxEvent.state = rxfrm.state;
    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
}

ISR(TRX24_TX_END_vect)
{
    uint8_t cause;

    cause = trx_bit_read(SR_TRAC_STATUS);
    TrxState = APP_RECEIVE;
    if (cause == TRAC_SUCCESS)
    {
        TrxEvent.cmd = CMD_TX_DONE_OK;
    }
    else
    {
        TrxEvent.cmd = CMD_TX_DONE_NOK;
    }
    trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
}
#endif  /* RFA1 */

/**
 * Application specific timer IRQ routine.
 * used to read key status (debounced).
 */
#if defined(DOXYGEN)
void TIMER_IRQ_vect()
#else
ISR(TIMER_IRQ_vect)
#endif
{
    LED_TOGGLE(1);
    if (KeyEvent == NONE_PRESS)
    {
        KeyEvent = debounce_key0();
    }

    if(TmoCounter > 0)
    {
        TmoCounter --;
        if (TmoCounter == 0)
        {
            TmoEvent = 1;
        }
    }
    LED_TOGGLE(1);
    status_led_handle();
}


static void status_led_config(uint8_t val, uint8_t blnk)
{
    if(val != IGNORE)
    {
        if (val & 1)
        {
            LED_SET(1);
        }
        else
        {
            LED_CLR(1);
        }
    }
    if (blnk != IGNORE)
    {
        Led1Blnk = blnk;
    }
}


static void status_led_handle()
{
static uint16_t tmr_tick = 0;

    tmr_tick ++;
    if (tmr_tick >= MSEC(100))
    {
        tmr_tick = 0;
        if (Led1Blnk > 1)
        {
            Led1Blnk--;
            if(Led1Blnk==0)
            {
                LED_CLR(1);
            }
            else
            {
                LED_TOGGLE(1);
            }
        }
    }
}

/* EOF */
