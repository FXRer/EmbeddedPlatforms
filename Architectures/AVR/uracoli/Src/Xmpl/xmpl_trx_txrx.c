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
/* Example for receiving frames */

#include "board.h"
#include "transceiver.h"
#include "ioutil.h"
#include "xmpl.h"


static uint8_t rxfrm[MAX_FRAME_SIZE];
static uint8_t rx_cnt;
static bool tx_in_progress;
static uint8_t tx_cnt;
static uint8_t txfrm[] = {1,0, /* faked ieee 802.15.4 data frame control field
                         * this is just needed, that a sniffer has to display something.*/
                   42,  /* sequence counter, updated by software */
                   'h','e','l','l','o',' ','u','r','a','c','o','l','i','!', /* data */
                   'X','X' /* these bytes are overwritten from transceivers CRC generator just before sent. */
                  };

int main(void)
{
    trx_regval_t rval;

    mcu_init();

    /* This will stop the application before initializing the radio transceiver
     * (ISP issue with MISO pin, see FAQ)
     */
    trap_if_key_pressed();

    /* Step 0: init MCU peripherals */
    LED_INIT();
    trx_io_init(SPI_RATE_1_2);

    /* Step 1: initialize the transceiver */
    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    trx_reg_write(RG_TRX_STATE,CMD_TRX_OFF);
    DELAY_US(TRX_INIT_TIME_US);
    rval = trx_bit_read(SR_TRX_STATUS);
    ASSERT(TRX_OFF == rval);

    /* Step 2: setup transmitter
     * - configure radio channel
     * - go into RX state,
     * - enable "receive end" IRQ
     */
    trx_bit_write(SR_CHANNEL,CHANNEL);
    trx_bit_write(SR_TX_AUTO_CRC_ON,1);
    trx_reg_write(RG_TRX_STATE,CMD_RX_ON);
#if defined(TRX_IRQ_TRX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_TRX_END);
#elif defined(TRX_IRQ_RX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_RX_END);
#else
#  error "Unknown IRQ bits"
#endif
    MCU_IRQ_ENABLE();

    /* Step 3: Going to receive frames */
    rx_cnt = 0;
    tx_cnt = 0;

    while(1)
    {
        DELAY_MS(500);
        if (tx_in_progress == false)
        {
            trx_reg_write(RG_TRX_STATE,CMD_PLL_ON);

            txfrm[2] = tx_cnt;
            trx_frame_write (sizeof(txfrm), txfrm);
            tx_in_progress = true;
            TRX_SLPTR_HIGH();
            TRX_SLPTR_LOW();
        }
    }
}

/*
 * \brief Routine for Tx frame handling, common for
 *        SPI transceivers and SOC (TRX_IF_RFA1)
 *
 */
static void handle_tx_frame()
{
    /* transmission completed */
    tx_cnt++;
    LED_TOGGLE(XMPL_LED_TX);
    trx_reg_write(RG_TRX_STATE,CMD_RX_ON);
}

/*
 * \brief Routine for Rx frame handling, common for
 *        SPI transceivers and SOC (TRX_IF_RFA1)
 *
 */
static void handle_rx_frame()
{
    uint8_t flen, *pfrm;
    uint16_t crc;

    /* upload frame and check for CRC16 validity */
    pfrm = rxfrm;
    flen = trx_frame_read(pfrm, sizeof(rxfrm), NULL);
    crc = 0;
    do
    {
        crc = _crc_ccitt_update(crc, *pfrm++);
    }
    while(flen--);
    /* if crc is correct, update RX frame counter */
    if (crc == 0)
    {
        rx_cnt ++;
    }
    /* display current rx statistics
     * LED[0] toggles with every received frame
     * LED[1:n] display the count of frames received with valid CRC
     */
    LED_TOGGLE(XMPL_LED_RX);
}

#if defined(TRX_IF_RFA1)
ISR(TRX24_TX_END_vect)
{
    handle_tx_frame();
}
ISR(TRX24_RX_END_vect)
{
    handle_rx_frame();
}
#else  /* !RFA1 */
ISR(TRX_IRQ_vect)
{
    trx_regval_t irq_cause = trx_reg_read(RG_IRQ_STATUS);

    if (irq_cause & TRX_IRQ_TRX_END)
    {
        if(tx_in_progress)
        {
            handle_tx_frame();
            tx_in_progress = false;
        }
        else
        {
            handle_rx_frame();
        }
    }
}
#endif

/* EOF */
