
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
/* Example for receiving frames in rx_aack mode */

#include "board.h"
#include "transceiver.h"
#include "ioutil.h"
#include "xmpl.h"

static volatile bool tx_in_progress;
static volatile bool rx_done;

static volatile uint8_t tx_cnt;
static volatile uint8_t tx_fail;
static volatile uint8_t rx_cnt;
static volatile uint8_t rx_seq;
static volatile uint8_t rx_fail;
#define SEQ_OFFSET     (2)
#define TX_FAIL_OFFSET (7)
#define TX_SRAM_OFFSET (1)

int main(void)
{
    trx_regval_t rval;
    uint8_t txfrm[] = {0x21,0x08,            /* IEEE 802.15.4 FCF:
                                                data frame with ack request */
                   42,                       /* sequence counter */
                   (PANID & 0xff), (PANID >> 8), /* destination PAN_ID */
                   (SHORT_ADDR & 0xff), (SHORT_ADDR >> 8), /* dest. short address */
                   42,                       /* TX fail counter */
                   'h','e','l','l','o',' ','u','r','a','c','o','l','i','!', /* data */
                   'X','X'    /* these bytes are overwritten from transceivers
                               * CRC generator directly before sent. */
                  };

    mcu_init();

    hif_init(HIF_DEFAULT_BAUDRATE);
    PRINTF("%s\n", __FILE__);
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
    PRINT("TRX_OFF ... ok\n");
    /* Step 2: setup transmitter
     * - configure radio channel
     * - enable transmitters automatic crc16 generation
     * - go into RX AACK state,
     * - configure address filter
     * - enable "receive end" IRQ
     */
    trx_bit_write(SR_CHANNEL,CHANNEL);
    trx_bit_write(SR_TX_AUTO_CRC_ON,1);

    trx_reg_write(RG_PAN_ID_0,(PANID&0xff));
    trx_reg_write(RG_PAN_ID_1,(PANID>>8));

    trx_reg_write(RG_SHORT_ADDR_0,(SHORT_ADDR&0xff));
    trx_reg_write(RG_SHORT_ADDR_1,(SHORT_ADDR>>8));

    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
    PRINT("RX_AACK_ON ... ok\n");

#if defined(TRX_IRQ_TRX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_TRX_END);
#elif defined(TRX_IRQ_RX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_TX_END | TRX_IRQ_RX_END);
#else
#  error "Unknown IRQ bits"
#endif
    MCU_IRQ_ENABLE();

    /* Step 3: send frames each 1000ms */
    tx_cnt = 0;
    tx_fail = false;
    tx_in_progress = false;

    rx_cnt = 0;
    rx_seq = 0;
    rx_fail = false;
    rx_done = false;

    while(1)
    {
        DELAY_MS(1000);
        if (tx_in_progress == false)
        {
            PRINTF(">TX FRAME tx: %4d, fail: %3d, tx_seq: %3d\n",
                    tx_cnt, tx_fail, txfrm[SEQ_OFFSET]);
            /* some older SPI transceivers require this coming from RX_AACK*/
            trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);
            trx_reg_write(RG_TRX_STATE, CMD_TX_ARET_ON);

            txfrm[SEQ_OFFSET] = tx_cnt;
            txfrm[TX_FAIL_OFFSET] = tx_fail;
            trx_frame_write(sizeof(txfrm), txfrm);
            tx_in_progress = true;
            TRX_SLPTR_HIGH();
            TRX_SLPTR_LOW();
        }
        if (rx_done)
        {
            rx_done = false;
            PRINTF("<RX FRAME rx: %4d, fail: %3d, rx_seq: %3d\n",
                    rx_cnt, rx_fail, rx_seq);
        }
    }
}

#if defined(TRX_IF_RFA1)
ISR(TRX24_TX_END_vect)
{
    trx_regval_t trac_status;

    trac_status = trx_bit_read(SR_TRAC_STATUS);
    tx_in_progress = false;

    /* refresh PLL and state machines */
    trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);

    LED_TOGGLE(XMPL_LED_TX);

    if (trac_status != TRAC_SUCCESS)
    {
        tx_fail ++;
    }
    else
    {
        tx_cnt ++;
    }
    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
}

ISR(TRX24_RX_END_vect)
{
    trx_regval_t trac_status;
    uint8_t rxframe[127];

    trac_status = trx_bit_read(SR_TRAC_STATUS);
    rx_done = true;
    LED_TOGGLE(XMPL_LED_RX);
    trx_frame_read(rxframe, sizeof(rxframe), NULL);
    rx_seq = rxframe[SEQ_OFFSET];
    if (trac_status != TRAC_SUCCESS)
    {
        rx_fail ++;
    }
    else
    {
        rx_cnt ++;
        rx_seq = rxframe[SEQ_OFFSET];
    }
}
#else  /* !RFA1 */
ISR(TRX_IRQ_vect)
{
    trx_regval_t irq_cause;
    trx_regval_t trac_status;
    uint8_t rxframe[127];

    irq_cause = trx_reg_read(RG_IRQ_STATUS);
    trac_status = trx_bit_read(SR_TRAC_STATUS);

    if (irq_cause & TRX_IRQ_TRX_END)
    {
        if(tx_in_progress == true)
        {
            /* refresh PLL and state machines */
            trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);

            LED_TOGGLE(XMPL_LED_TX);
            tx_in_progress = false;
            if (trac_status != TRAC_SUCCESS)
            {
                tx_fail ++;
            }
            else
            {
                tx_cnt ++;
            }

            trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);
            trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
        }
        else
        {
            LED_TOGGLE(XMPL_LED_RX);
            rx_done = true;
            if (trac_status != TRAC_SUCCESS)
            {
                rx_fail ++;
            }
            else
            {
                trx_frame_read(rxframe, sizeof(rxframe), NULL);
                rx_cnt ++;
                rx_seq = rxframe[SEQ_OFFSET];
            }
        }
    }
}
#endif  /* RFA1 */

/* EOF */
