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
 * @brief Atmel RZ600 Stick attached to J1 of XMEGA-E5 Xplained Board
 *        Any of the three sticks can be used (RF230, RF231, RF212)
 *
 * The wiring of the Radio and the MCU is shown below:
 *
<pre>
          ATxmega32E5    AT86RF2XX
        ------------    ---------
        PC0             RSTN
        PC1             DIG2 (misc)
        PC2             IRQ
        PC3             SLPTR
        PC4             SELN
        PC5             SCK
        PC6             MISO
        PC7             MOSI

        -- mounted on XMEGA-E5 Xplained --
        PD4             LED0
        PD5             LED1
        PD0             SW0
        PD2             SW1
</pre>

**/

#ifndef BOARD_XME5RZ2XX_H
#define BOARD_XME5RZ2XX_H

#if defined(xme5rz230)
# define RADIO_TYPE (RADIO_AT86RF230)
#elif defined(xme5rz231)
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(xme5rz212)
# define RADIO_TYPE (RADIO_AT86RF212)
#else
#error "Undefined board"
#endif


/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/


/**< PORT register for RESET pin */
#define PORT_TRX_RESET  PORTC
/**< PIN mask for RESET pin */
#define MASK_TRX_RESET  (1<<0)

/**< DDR register for SLP_TR pin */
#define PORT_TRX_SLPTR  PORTC
/**< PIN mask for SLP_TR pin */
#define MASK_TRX_SLPTR  (1<<3)


/**< RESET pin IO initialization */
#define TRX_RESET_INIT() PORT_TRX_RESET.DIRSET = MASK_TRX_RESET
/**< set RESET pin to high level */
#define TRX_RESET_HIGH() PORT_TRX_RESET.OUTSET = MASK_TRX_RESET
/**< set RESET pin to low level */
#define TRX_RESET_LOW()  PORT_TRX_RESET.OUTCLR = MASK_TRX_RESET

/** SLP_TR pin IO initialization */
#define TRX_SLPTR_INIT() PORT_TRX_SLPTR.DIRSET = MASK_TRX_SLPTR
/** set SLP_TR pin to high level */
#define TRX_SLPTR_HIGH() PORT_TRX_SLPTR.OUTSET = MASK_TRX_SLPTR
/**< set SLP_TR pin to low level */
#define TRX_SLPTR_LOW()  PORT_TRX_SLPTR.OUTCLR = MASK_TRX_SLPTR


/*=== IRQ access macros ==============================================*/

/**< interrupt vector name */
# define TRX_IRQ_vect    PORTC_INT_vect
/**< interrupt PORT */
# define PORT_TRX_IRQ    PORTC
/**< interrupt mask for PORTC */
# define TRX_IRQ         (1<<2)



/** init interrupt handling
 *  - rising edge triggers ICP1 (ICES1),
 *  - timer capture is enabled (ICF1)
 */
# define TRX_IRQ_INIT()  do{\
                            PORT_TRX_IRQ.PIN2CTRL = PORT_ISC_RISING_gc; \
                            PORT_TRX_IRQ.INTCTRL = PORT_INTLVL_HI_gc; \
                            PMIC.CTRL |= PMIC_HILVLEN_bm;\
                          } while(0)

/** disable TRX interrupt */
#define DI_TRX_IRQ() {PORT_TRX_IRQ.INTMASK &= ~TRX_IRQ; PORT_TRX_IRQ.INTFLAGS |= TRX_IRQ; }

/** enable TRX interrupt */
#define EI_TRX_IRQ() {PORT_TRX_IRQ.INTFLAGS |= TRX_IRQ; PORT_TRX_IRQ.INTMASK |= TRX_IRQ;}

#define ACK_TRX_IRQ() {PORT_TRX_IRQ.INTFLAGS |= TRX_IRQ;}

/** timestamp register for RX_START event
 * FIXME: add and test the enabling of input capture for separate RX_START (AT86RF231/212)
 *        currently we use the timer register.
 */
#define TRX_TSTAMP_REG TCC5.CNT


/*=== SPI access macros ==============================================*/
#define PORT_SPI (PORTC)  /**< PORT register for SPI port */

#define SPI_MOSI (1<<7)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<6)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<5)  /**< PIN mask for SCK pin */
#define SPI_SS   (1<<4)  /**< PIN mask for SS pin */

#define SPI_DATA_REG SPIC.DATA  /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    PORT_SPI.OUTSET = SPI_SCK | SPI_SS;
    PORT_SPI.DIRSET = SPI_MOSI | SPI_SCK | SPI_SS;
    PORT_SPI.DIRCLR = SPI_MISO;

    SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm;

    SPIC.CTRL |= (spirate & 0x03);
    SPIC.CTRL |= ((spirate << 5) & 0x80); /* CLK2X */
}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI.OUTCLR = SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI.OUTSET = SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()        do { while((SPIC.STATUS & SPI_IF_bm) == 0);} while(0)


/*=== LED access macros ==============================================*/

/* buttons on Xmega-E5 Xplained
 *
 * LED0: PD4
 * LED1: PD5
 */
#define LED_PORT     PORTD_OUT
#define LED_DDR      PORTD_DIR
#define LED_MASK     (0x30) // PD4, PD5
#define LED_SHIFT    (4)
#define LEDS_INVERSE (0)
#define LED_NUMBER   (2)


/*=== KEY access macros ==============================================*/

/* buttons on Xmega-E5 Xplained
 *
 * SW0: PD0
 * SW1: PD2
 */
# define MASK_KEY     (0x05)
# define SHIFT_KEY    (0)
# define INVERSE_KEYS (1)
# define KEY_INIT() do{ \
        PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;  \
        PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;  \
        PORTD.DIRCLR = MASK_KEY;             \
    }while(0)
# define KEY_GET()   ((PORTD.IN & MASK_KEY) >> SHIFT_KEY)


#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() \
        do{\
        } while(0)

#define SLEEP_ON_KEY_vect PORTD_INT_vect

/*=== Host Interface ================================================*/

#define HIF_TYPE (HIF_USARTD0)

/*=== TIMER Interface ===============================================*/

#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCC5.CNT)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCC5.CTRLA = TC45_CLKSEL_DIV1_gc; \
        TCC5.INTFLAGS |= TC5_OVFIF_bm; \
        TCC5.INTCTRLA = TC45_OVFINTLVL_HI_gc; \
    }while(0)
#define TIMER_IRQ_vect TCC5_OVF_vect

/* Header J1 on XMEGA-E5 Xplained */
#define TWI_INST TWIC

#endif /* BOARD_XME5RZ2XX_H */
