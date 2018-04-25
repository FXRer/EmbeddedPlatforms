/* Copyright (c) 2015 Axel Wachtler, Daniel Thiele
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
 * @brief Implementation of the @ref grpAppRdiag
 *
 * @ingroup grpAppRdiag
 */
/**
 * This application provides some basic test functions of the radio chip.
 *
 */
#include <string.h>
#include "board.h"
#include "transceiver.h"
#include "radio.h"
#include "ioutil.h"
#include "timer.h"

/* === Types ===============================================*/

typedef struct
{
    uint8_t new;
    uint8_t len;
    uint8_t crc;
    uint8_t lqi;
    uint8_t ed;
    uint8_t seq;
} rx_frame_t;


typedef struct
{
    uint32_t rxcnt;
    uint32_t crcerr;
    int32_t ed;
    uint32_t lqi;

    uint32_t txcnt;
    time_t   start;
} statistic_t;


typedef struct
{
    channel_t channel;
    txpwr_t   txpwr;
    ccamode_t ccamode;
    rxidle_t  rxidle;
} rdiag_ctx_t;

typedef struct
{
    trx_regaddr_t offs;
    trx_regval_t  val;
} regval_t;


/* === Prototypes ==========================================*/
void rdiag_init(void);
void set_next_channel(channel_t chaninc);
void set_next_pwr(int8_t pwrinc);
void set_next_cca(void);

void toggle_rxon_idle(void);
void send_frame(uint8_t seq);
void send_continous(void);
void show_statistic(bool reset);
void print_helpscreen(void);
time_t blink(timer_arg_t t);

void test_all(void);
void test_regreset(void);
void test_sram(void);
void test_sram_const(uint8_t pattern);
void test_sram_runbit(void);
void test_sram_address(void);
void test_trxirq(void);
void diag_irq_handler(uint8_t cause);

/* === Globals =============================================*/

volatile uint8_t irqcause = 0;
volatile uint16_t irqcnt = 0;
volatile uint16_t badirqcnt = 0;

static uint8_t RxBuf[MAX_FRAME_SIZE];
static uint8_t TxBuf[MAX_FRAME_SIZE];
static rx_frame_t rxfrm;

statistic_t RdiagStat;
static rdiag_ctx_t RdiagCtx =
    {.txpwr = 0,
     .ccamode = 1,
     .channel = TRX_MIN_CHANNEL,
     .rxidle = false
    };

int8_t verbose;
bool   conttx = false;
uint8_t tx_length = 42;
#define TBLINK_PERIOD (500)
#define NL "\n\r"
timer_hdl_t  th_blink;

/** factory defaults of radio parameter  */
const trx_param_t PROGMEM trxp_flash =
{
    .chan=13,
    .txp=0,
    .cca=1,
    .edt=11,
    .clkm=0
};

/** todo: the register resetvalue list should go to at86rf*.h*/
const regval_t regrst[] = {
    {RG_TRX_STATUS,      0 },
    {RG_TRX_STATE,      0 },
    {RG_TRX_CTRL_0,      0x19 },
    {RG_PHY_TX_PWR,      0 },
    {RG_PHY_RSSI,      0 },
    {RG_PHY_ED_LEVEL,      0 },
    {RG_PHY_CC_CCA,      0x2b },
    {RG_CCA_THRES,      0xc7 },
    {RG_IRQ_MASK,      0xff },
    {RG_IRQ_STATUS,      0 },
    {RG_VREG_CTRL,      0 },
    {RG_BATMON,      0x02 },
    {RG_XOSC_CTRL,      0xf0 },
    {RG_PLL_CF,      0x5f },
    {RG_PLL_DCU,      0x20 },
    {RG_PART_NUM,      0x02 },
    {RG_VERSION_NUM,      0x01 },
    {RG_MAN_ID_0,      0x1f },
    {RG_MAN_ID_1,      0x00 },
    {RG_SHORT_ADDR_0,      0 },
    {RG_SHORT_ADDR_1,      0 },
    {RG_PAN_ID_0,      0 },
    {RG_PAN_ID_1,      0 },
    {RG_IEEE_ADDR_0,      0 },
    {RG_IEEE_ADDR_1,      0 },
    {RG_IEEE_ADDR_2,      0 },
    {RG_IEEE_ADDR_3,      0 },
    {RG_IEEE_ADDR_4,      0 },
    {RG_IEEE_ADDR_5,      0 },
    {RG_IEEE_ADDR_6,      0 },
    {RG_IEEE_ADDR_7,      0 },
#if RADIO_TYPE == RADIO_AT86RF230 || RADIO_TYPE == RADIO_AT86RF230B
    {RG_XAH_CTRL,      0x38 },
#endif
    {RG_CSMA_SEED_0,      0xea },
    {RG_CSMA_SEED_1,      0xc2 },
};



/**
 * @brief Main function of diagradio application.
 *
 * This routine performs the initialization of the
 * hardware modules and stays in a endless loop, which
 * interpretes the commands, received from the host interface.
 *
 */
int main(void)
{

int inchar;
uint8_t tmp, *p;
trx_param_t trxp;

	/* init trx layer and execute tests */

	mcu_init();
	LED_INIT();
	LED_SET_VALUE(3);
	trx_io_init(SPI_RATE_1_2);
	trx_set_irq_handler(diag_irq_handler);
	hif_init(HIF_DEFAULT_BAUDRATE);

	MCU_IRQ_ENABLE();
	test_all();


	/* now initialize radio layer */

	MCU_IRQ_DISABLE();

	memset(TxBuf, 0x55, sizeof(TxBuf));
    memset(RxBuf, 0xff, sizeof(RxBuf));
    TRX_RESET_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    radio_init(RxBuf, MAX_FRAME_SIZE);

    MCU_IRQ_ENABLE();

	/* init radio parameter */
    radio_set_param(RP_CHANNEL(RdiagCtx.channel));
    radio_set_param(RP_TXPWR(RdiagCtx.txpwr));
    radio_set_param(RP_CCAMODE(RdiagCtx.ccamode));
    radio_set_param(RP_IDLESTATE(STATE_OFF));

    PRINT(NL"Radio Diag 0.30"NL);
    tmp = trx_reg_read(RG_TRX_STATUS);
    PRINTF(">RADIO INIT[%s]: %s (status=0x%02x)"NL, BOARD_NAME, (tmp == 8 ? "OK" : "FAIL"), tmp );
    PRINTF("Timer-Tick %lx"NL, TIMER_TICK);
    tmp = trx_reg_read(RG_PART_NUM);
    PRINTF("RADIO_PART_NUM = 0x%02x, RG_PART_NUM = 0x%02x"NL, RADIO_PART_NUM, tmp);
    tmp = trx_reg_read(RG_VERSION_NUM);
    PRINTF("RADIO_VERSION_NUM = 0x%02x, RG_VERSION_NUM = 0x%02x"NL, RADIO_VERSION_NUM, tmp);

    PRINT(NL"----- Menu -----"NL); 
    print_helpscreen();

    th_blink = timer_start(blink,TBLINK_PERIOD,0);

    while(1)
    {
        inchar = hif_getc();
        if(EOF != inchar)
        {
            switch (inchar)
            {

                case '+':
                    set_next_channel(+1);
                    break;
                case '-':
                    set_next_channel(-1);
                    break;

                case 'p':
                    set_next_pwr(-1);
                    break;
                case 'P':
                    set_next_pwr(1);
                    break;

                case 'c':
                    tmp = radio_do_cca();
                    PRINTF(">CCA: status=%d"NL, tmp);
                    break;
                case 'C':
                    set_next_cca();
                    break;

                case 'R':
                    toggle_rxon_idle();
                    break;

                case 'r':
                    radio_set_state(STATE_RX);
                    PRINT(">STATE_RX"NL);
                    break;

                case 't':
                    radio_set_state(STATE_TX);
                    PRINT(">STATE_TX"NL);
                    break;

                case 'o':
                    radio_set_state(STATE_OFF);
                    PRINT(">STATE_OFF"NL);
                    break;

                case 'z':
                    radio_set_state(STATE_SLEEP);
                    PRINT(">STATE_SLEEP"NL);
                    break;

                case 'l':
                    tx_length += 1;
                    tx_length &= 0x7f;
                    PRINTF(">TX LEN = %d"NL,tx_length);
                    break;

                case 'L':
                    tx_length += 10;
                    tx_length &= 0x7f;
                    PRINTF(">TX LEN = %d"NL,tx_length);
                    break;

                case 's':
                    send_frame(tx_length);
                    break;

                case 'S':
                    send_continous();
                    break;

                case 'i':
                    show_statistic(0);
                    break;

                case 'I':
                    show_statistic(1);
                    break;

                case 'h':
                    print_helpscreen();
                    break;

                case 'v':
                    if(verbose>0) verbose --;
                    PRINTF(">VERBOSE: %d"NL,verbose);
                    break;

                case 'V':
                    if(verbose<2) verbose ++;
                    PRINTF(">VERBOSE: %d"NL,verbose);
                    break;

                case 'g':
                    trx_parms_get(&trxp);
                    PRINTF("{"\
                           "chan: %d, txp: %d,"\
                           " cca: %d,\n\r edt: %d,"\
                           " clkm: %d}"NL,
                            trxp.chan, trxp.txp, trxp.cca,
                            trxp.edt, trxp.clkm);
                    p = (uint8_t*)&trxp;
                    PRINTF("avrdude: w ee 8 0x%x 0x%x 0x%x 0x%x 0x%x"NL,
                           p[0],p[1],p[2],p[3],p[4]);
                    break;

                case 'b':
                    jump_to_bootloader();

                    /* this code is never reached */
                    while(1){}
                    break;
            default:
                    PRINTF("unsupported command: %c, type 'h' for list of commands"NL, inchar);
                    break;
            }
        }

        if(rxfrm.new == 1)
        {
            if (verbose > 0)
            {
                PRINTF("++FRAME len=%d crc=%3s lqi=%d ed=%d seq=%d"NL,
                    rxfrm.len, rxfrm.crc ? "ERR" : "OK",
                    rxfrm.lqi, rxfrm.ed, rxfrm.seq);
            }
            if (verbose > 1)
            {
                DUMP(rxfrm.len, RxBuf);
            }
            rxfrm.new = 0;
        }

    }
    return 0;

}


void rdiag_init(void)
{
    memset(TxBuf, 0x55, sizeof(TxBuf));
    memset(RxBuf, 0xff, sizeof(RxBuf));
    /* reset ressource  radio parameter */
    mcu_init();
    LED_INIT();
    KEY_INIT();
    timer_init();
    hif_init(HIF_DEFAULT_BAUDRATE);
    radio_init(RxBuf, MAX_FRAME_SIZE);

    MCU_IRQ_ENABLE();
}

/**
 * @brief Increment/decrement channel.
 */
void set_next_channel(int8_t chaninc)
{

    if (chaninc > 0)
    {
        RdiagCtx.channel = TRX_NEXT_CHANNEL_WRAP(RdiagCtx.channel);
    }
    else if (chaninc < 0)
    {
        RdiagCtx.channel = TRX_PREV_CHANNEL_WRAP(RdiagCtx.channel);
    }

    radio_set_param(RP_CHANNEL(RdiagCtx.channel));
    PRINTF(">CHAN: %d"NL, RdiagCtx.channel);
}

/**
 * @brief Enable/disable mode RX_ON_IDL
 */
void toggle_rxon_idle(void)
{
    RdiagCtx.rxidle ^= 1;
    if (RdiagCtx.rxidle == 1)
    {
        radio_set_param(RP_IDLESTATE(STATE_RX));
    }
    else
    {
        radio_set_param(RP_IDLESTATE(STATE_OFF));
    }
    PRINTF(">RXON_IDLE: %d"NL, RdiagCtx.rxidle);
}

/**
 * @brief Increment/decrement TX power.
 */
void set_next_pwr(int8_t pwrinc)
{

    RdiagCtx.txpwr +=  pwrinc;
    if (RdiagCtx.txpwr > TRX_MAX_TXPWR)
    {
        RdiagCtx.txpwr = TRX_MIN_TXPWR;
    }
    else if (RdiagCtx.txpwr < TRX_MIN_TXPWR)
    {
        RdiagCtx.txpwr = TRX_MAX_TXPWR;
    }
    radio_set_param(RP_TXPWR(RdiagCtx.txpwr));
    PRINTF(">PWR: %d"NL, RdiagCtx.txpwr);
}


/**
 * @brief Select CCA Mode.
 */
void set_next_cca(void)
{

char *pcca[] = {"?","ED","CS","CS&ED"};

    RdiagCtx.ccamode += 1;
    if (RdiagCtx.ccamode > 3)
    {
        RdiagCtx.ccamode = 1;
    }

    radio_set_param(RP_CCAMODE(RdiagCtx.ccamode));

    PRINTF(">CCA: mode=%d (%s)"NL, RdiagCtx.ccamode, pcca[RdiagCtx.ccamode]);
}


/**
 * @brief Transmit a frame with a given payload length.
 */
void send_frame(uint8_t frmlen)
{
    static uint8_t seqnb;

    seqnb ++;
    if (frmlen < 5)
    {
        frmlen = 5;
    }
    if (frmlen > MAX_FRAME_SIZE)
    {
        frmlen = MAX_FRAME_SIZE;
    }
    TxBuf[0] = 1;
    TxBuf[1] = 0;
    TxBuf[2] = seqnb;
    TxBuf[frmlen - 2] = 0;
    TxBuf[frmlen - 1] = 0;
    radio_set_state(STATE_TX);
    radio_send_frame(frmlen, TxBuf, 0);

    PRINTF(">SEND len=%d"NL,frmlen);
    if (verbose > 1)
    {
        DUMP(frmlen, TxBuf);
    }
}

/**
 * @brief Send frames permanently (next frame is triggered at TX_END_IRQ).
 */
void send_continous(void)
{
static bool active = 0;

    active  ^= 1;

    if(active)
    {
        RdiagCtx.rxidle = 2;
        radio_set_param(RP_IDLESTATE(STATE_TX));
    }
    else
    {
        RdiagCtx.rxidle = 0;
        radio_set_param(RP_IDLESTATE(STATE_OFF));

    }
    PRINTF(">SEND CONTINUOUS %s"NL, (RdiagCtx.rxidle ? "START":"STOP"));
    if(RdiagCtx.rxidle)
    {
        send_frame(tx_length);
    }
}



/**
 * @brief Callback function for frame reception.
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
                                  int8_t ed, uint8_t crc)
{
    rxfrm.new = 1;
    rxfrm.len = len;
    rxfrm.crc = crc;
    rxfrm.lqi = lqi;
    rxfrm.ed = ed;
    if (rxfrm.len > 3)
    {
        rxfrm.seq = frm[2];
    }

    RdiagStat.crcerr += ( (crc != 0)||(len==0) ? 1 : 0);
    RdiagStat.rxcnt ++;
    RdiagStat.lqi += lqi;
    RdiagStat.ed += ed;
    return frm;
}

/**
 * @brief Callback function for TX_END IRQ.
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
static uint8_t sn;
   RdiagStat.txcnt++;
   if(conttx)
   {
        /* force next frame sending */
        TRX_SLPTR_HIGH();
        TRX_SLPTR_LOW();
        trx_sram_write(3, 1, &sn);
        sn ++;
   }
   else if(verbose >= 1) // do not output in conttx mode
   {
       PRINTF("TX done status=%02X"NL, status);
   }
}

/**
 * @brief Display RX/TX transceiver statistic and state.
 */
void show_statistic(bool reset)
{

time_t now;

    now = timer_systime();
    PRINTF(
        ">STAT"
        " duration: %ld ticks"NL,
            now - RdiagStat.start);

    PRINTF(
        " RX:"
        " frames: %ld"
        " crcerr: %ld"NL,
            RdiagStat.rxcnt, RdiagStat.crcerr);
    PRINTF(
        " RX: channel %d"
        " avg ed: %ld"
        " avg lqi:  %ld"NL,
            RdiagCtx.channel,
            RdiagStat.ed/RdiagStat.rxcnt, RdiagStat.lqi/RdiagStat.rxcnt);

    PRINTF(
        " TX:"
        " frames: %ld"NL,
            RdiagStat.txcnt);
    if (reset)
    {
        memset(&RdiagStat, 0, sizeof(RdiagStat));
        RdiagStat.start = timer_systime();
    }
}


/**
 * @brief Callback for errors in radio module functions.
 */
void usr_radio_error(radio_error_t err)
{
uint8_t regval, r2, r3;
uint16_t t;
    regval = trx_reg_read(RG_TRX_STATUS);
    r2 = trx_reg_read(0x30);
    r3 = trx_reg_read(RG_VREG_CTRL);
    while(1)
    {

        PRINTF(NL"RADIO ERROR : %d radio status : 0x%02x:0x%02x:0x%02x"NL,
                err, regval, r2, r3);
        t = 2000;
        while(t--)
        {
            DELAY_US(1000);
        }
    }
}



/**
 * @brief Print help for hotkeys.
 */
void print_helpscreen(void)
{
    PRINT("i/I : show / show and reset statistic"NL);
    PRINT("o   : set state OFF"NL);
    PRINT("r   : set state RX"NL);
    PRINT("t   : set state TX"NL);
    PRINT("z   : set state SLEEP"NL);
    PRINT("+/- : incr./decr. channel"NL);
    PRINT("s   : send a test frame"NL);
    PRINT("S   : start/stop continous sending frames"NL);
    PRINT("R   : toggle RXON_IDLE parameter"NL);
    PRINT("P/p : incr./decr. power"NL);
    PRINT("c   : do CCA measurement"NL);
    PRINT("C   : change CCA mode"NL);
    PRINT("V/v : incr./decr. verbosity (0...2)"NL);
    PRINT("G/g : store/show seetings"NL);
}




/**
 *  @brief Life LED timer service routine.
 */
time_t blink(timer_arg_t t)
{
#if LED_NUMBER > 0
static volatile uint8_t btick;
#endif
    LED_SET_VALUE(btick++);
    /* restart the timer again */
    return  TBLINK_PERIOD;
}

void test_all(void)
{
    test_regreset();
    test_sram();
    test_trxirq();
}

/**
 * @brief  Test the reset values of the chip
 *
 * Perform a chip reset and check all registers
 * according to there power on reset values.
 * Note:
 * If the transceiver is not in power on state,
 * the following errors are reported:
 *
<pre>
    ERR reset 0 offs=0x01 val=0x08 exp=0x00
    ERR reset 10 offs=0x10 val=0x04 exp=0x00
    ERR reset 11 offs=0x11 val=0x22 exp=0x02
    ERR reset 13 offs=0x18 val=0x64 exp=0x58
</pre>
 *
 *
 */
void test_regreset(void)
{
uint8_t i, err, val ;

    err = 0;

    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();

    for(i=0;i<sizeof(regrst)/sizeof(regval_t); i++)
    {
        val = trx_reg_read(regrst[i].offs);
        if (val != regrst[i].val)
        {
            PRINTF("ERR regreset nb=%d offs=0x%02x val=0x%02x exp=0x%02x"NL,
                i, regrst[i].offs, val, regrst[i].val);
            err ++;
        }
    }
    if (err == 0)
    {
        PRINT("OK regreset"NL);
    }
}

/**
 * @brief Test the SRAM
 *
 * This stresses the SPI bus and
 * shows if glitches disturb or influence the data
 * transfer
 *
 * If problems occur, try to reduce the SPI speed.
 */
void test_sram(void)
{

    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    trx_bit_write(SR_TRX_CMD,CMD_TRX_OFF);
    test_sram_const(0x00);
    test_sram_const(0x55);
    test_sram_const(0xaa);
    test_sram_const(0xff);
    test_sram_runbit();
    test_sram_address();

}

/**
 * @brief Test SRAM with constant pattern
 */
void test_sram_const(uint8_t pattern)
{
    uint8_t buf[MAX_FRAME_SIZE];
    uint8_t i, err = 0;
    /* test all 0 */
    memset(buf, pattern, sizeof(buf));
    trx_sram_write(0, sizeof(buf), buf);
    memset(buf, pattern ^ 0xff, sizeof(buf));
    trx_sram_read(0, sizeof(buf), buf);
    for (i=0;i<sizeof(buf); i++)
    {
        if (buf[i] != pattern)
        {
            PRINTF("ERR sram_const: addr=%d, pattern=0x%02x, exp=0x%02x"NL,
             i, buf[i], pattern);
             err ++;
        }
    }

    if (err == 0)
    {
        PRINTF("OK sram_const(0x%02x)"NL, pattern);
    }
}

/**
 * @brief Test SRAM with running 1 and running 0 pattern
 */
void test_sram_runbit(void)
{
    uint8_t buf[MAX_FRAME_SIZE];
    uint8_t i, err = 0, exp;

    /* running 1 */
    for (i=0; i<sizeof(buf); i++)
    {
        buf[i] = _BV(i&7);
    }
    trx_sram_write(0, sizeof(buf), buf);
    memset(buf, 0xff, sizeof(buf));
    trx_sram_read(0, sizeof(buf), buf);

    for (i=0;i<sizeof(buf); i++)
    {
        exp = _BV(i&7);
        if (buf[i] != exp)
        {
            PRINTF("ERR sram_runbit: addr=%d, pattern=0x%02x, exp=0x%02x"NL,
             i, buf[i], _BV(i&7));
             err ++;
        }
    }

    /* running 0 */
    for (i=0; i<sizeof(buf); i++)
    {
        buf[i] = ~_BV(i&7);
    }
    trx_sram_write(0, sizeof(buf), buf);
    memset(buf, 0xff, sizeof(buf));
    trx_sram_read(0, sizeof(buf), buf);
    for (i=0;i<sizeof(buf); i++)
    {
        exp = ~_BV(i&7);
        if (buf[i] != exp)
        {
            PRINTF("ERR sram_runbit: addr=%d, pattern=0x%02x, exp=0x%02x"NL,
                 i, buf[i], exp);
            err ++;
        }
    }

    if (err == 0)
    {
        PRINT("OK sram_runbit"NL);
    }

}

/**
 * @brief Test SRAM with value = address pattern
 */
void test_sram_address(void)
{
    uint8_t buf[MAX_FRAME_SIZE];
    uint8_t i, err = 0;
    /* running 1 */
    for (i=0; i<sizeof(buf); i++)
    {
        buf[i] =i;
    }
    trx_sram_write(0, sizeof(buf), buf);
    memset(buf, 0xff, sizeof(buf));
    trx_sram_read(0, sizeof(buf), buf);

    for (i=0;i<sizeof(buf); i++)
    {
        if (buf[i] != i)
        {
            PRINTF("ERR sram_runbit: addr=%d, pattern=0x%02x, exp=0x%02x"NL,
             i, buf[i], i);
             err ++;
        }
    }

    if (err == 0)
    {
        PRINT("OK sram_address"NL);
    }
}


/**
 * Test IRQ occurence
 */
void test_trxirq(void)
{
uint8_t cnt;
    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    irqcnt = 0;
    irqcause = 0;

    trx_bit_write(SR_TRX_CMD,CMD_TRX_OFF);
    trx_bit_write(SR_TRX_CMD,CMD_PLL_ON);
    trx_bit_write(SR_CHANNEL,21);
    trx_bit_write(SR_CHANNEL,11);

    /* PLL Lock should occur within 128us */

    cnt = 32;
    do
    {
        _delay_us(180);
    }
    while (cnt-- && (irqcnt == 0) );

    if ((irqcnt > 0) && (irqcause & TRX_IRQ_PLL_LOCK))
    {
        PRINTF("OK trxirq %d, irqs=%d, badirqs=%d"NL,
                cnt, irqcnt, badirqcnt);
    }
    else
    {
        PRINTF("ERR trxirq %d, irqs=%d, badirqs=%d cause=0x%x"NL,
                cnt, irqcnt, badirqcnt,irqcause);
    }

}

void diag_irq_handler(uint8_t cause)
{

    irqcause |= cause;
    if (cause != 0)
    {
        irqcnt++;
    }
    else
    {
        badirqcnt++;
    }
    LED_SET_VALUE(irqcnt);
}

/* EOF */
