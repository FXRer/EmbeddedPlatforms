/* Copyright (c) 2011 ... 2014 Axel Wachtler, Daniel Thiele
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
 * @brief Atmel REB-CBB with Radio Extender Board (REB) attached
 *        Any type of REB is supported
 *
 * The wiring of the REB and the MCU is shown below:
 *
<pre>
          ATxmega256A3    AT86RF2XX
          ------------    ---------
        PC0             RSTN
        PC1             DIG2
        PC2             IRQ
        PC3             SLPTR
        PC4             SELN
        PC5             MOSI
        PC6             MISO
        PC7             SCK
        PD0             CLKM
        PD1             TXCW

        PB0             LED1
        PB1             LED2
        PB2             LED3
        PB3             KEY1
</pre>

Atmel Zigbit Series:

 * At86rf233 + Xmega256A3:
    * ZigBit Module: http://www.atmel.com/tools/ATZB-X0-256-3-0-C.aspx
    * USB Stick:     http://www.atmel.com/tools/ATZB-X-233-USB.aspx
    * XPRO Ext:      http://www.atmel.com/tools/ATZB-X-233-XPRO.aspx
 * At86rf212B + Xmega256A3:
    * ZigBit Module: http://www.atmel.com/tools/ATZB-X0-256-4-0-CN.aspx
    * USB Stick:     http://www.atmel.com/tools/ATZB-X-RF212B-USB.aspx
    * XPRO Ext:      http://www.atmel.com/tools/ATZB-X-212B-XPRO.aspx
**/

#ifndef BOARD_CBB2XX_H
#define BOARD_CBB2XX_H

#if defined(cbb230)
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(cbb230b)
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(cbb231)
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(cbb212)
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(cbb232)
# define RADIO_TYPE (RADIO_AT86RF232)
#elif defined(cbb233)
# define RADIO_TYPE (RADIO_AT86RF233)
#elif defined(atzbx233xpro)
# define RADIO_TYPE (RADIO_AT86RF233)
#elif defined(atzbx212bxpro)
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(atzbx233usb)
# define RADIO_TYPE (RADIO_AT86RF233)
#elif defined(xma1u233xpro)
# define RADIO_TYPE (RADIO_AT86RF233)
#else
#error "Undefined board"
#endif


/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/


#if BOARD_TYPE == BOARD_XMA1U233XPRO

/**< PORT register for RESET pin */
#define PORT_TRX_RESET  PORTE
/**< PIN mask for RESET pin */
#define MASK_TRX_RESET  (1<<1)

/**< DDR register for SLP_TR pin */
#define PORT_TRX_SLPTR  PORTR
/**< PIN mask for SLP_TR pin */
#define MASK_TRX_SLPTR  (1<<1)

#else // BOARD_TYPE == BOARD_XMA1U233XPRO

/**< PORT register for RESET pin */
#define PORT_TRX_RESET  PORTC
/**< PIN mask for RESET pin */
#define MASK_TRX_RESET  (1<<0)

/**< DDR register for SLP_TR pin */
#define PORT_TRX_SLPTR  PORTC
/**< PIN mask for SLP_TR pin */
#define MASK_TRX_SLPTR  (1<<3)

#endif // BOARD_TYPE == BOARD_XMA1U233XPRO


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

#if BOARD_TYPE == BOARD_XMA1U233XPRO

/**< interrupt vector name */
# define TRX_IRQ_vect    PORTR_INT0_vect
/**< interrupt PORT */
# define PORT_TRX_IRQ    PORTR
/**< interrupt mask for PORTR */
# define TRX_IRQ         (1<<0)

/**< PINxCTRL register, matches to pin position of macro TRX_IRQ */
#define PORT_TRX_IRQ_PINCTRL PORT_TRX_IRQ.PIN0CTRL

#elif BOARD_TYPE == BOARD_CBB230 || BOARD_TYPE == BOARD_CBB230B

/**< interrupt vector name */
# define TRX_IRQ_vect    PORTC_INT0_vect
/**< interrupt PORT */
# define PORT_TRX_IRQ    PORTC
/**< interrupt mask for PORTC */
# define TRX_IRQ         (1<<1)

/**< PINxCTRL register, matches to pin position of macro TRX_IRQ */
#define PORT_TRX_IRQ_PINCTRL PORT_TRX_IRQ.PIN1CTRL

#else

/**< interrupt vector name */
# define TRX_IRQ_vect    PORTC_INT0_vect
/**< interrupt PORT */
# define PORT_TRX_IRQ    PORTC
/**< interrupt mask for PORTC */
# define TRX_IRQ         (1<<2)

/**< PINxCTRL register, matches to pin position of macro TRX_IRQ */
#define PORT_TRX_IRQ_PINCTRL PORT_TRX_IRQ.PIN2CTRL

#endif // BOARD_TYPE == BOARD_XMA1U233XPRO

/** init interrupt handling
 *  - rising edge triggers ICP1 (ICES1),
 *  - timer capture is enabled (ICF1)
 */
# define TRX_IRQ_INIT()  do{\
                            PORT_TRX_IRQ.PIN2CTRL = PORT_ISC_RISING_gc; \
                            PORT_TRX_IRQ_PINCTRL = PORT_ISC_RISING_gc; \
                            PORT_TRX_IRQ.INTCTRL = PORT_INT0LVL_HI_gc; \
                            PMIC.CTRL |= PMIC_HILVLEN_bm;\
                          } while(0)

/** disable TRX interrupt */
#define DI_TRX_IRQ() {PORT_TRX_IRQ.INT0MASK &= ~TRX_IRQ;}

/** enable TRX interrupt */
#define EI_TRX_IRQ() {PORT_TRX_IRQ.INT0MASK |= TRX_IRQ;}

#define ACK_TRX_IRQ() {PORT_TRX_IRQ.INTFLAGS |= TRX_IRQ;}

/** timestamp register for RX_START event
 * FIXME: add and test the enabling of input capture for separate RX_START (AT86RF231/212)
 *        currently we use the timer register.
 */
#define TRX_TSTAMP_REG TCD0.CNT


/*=== SPI access macros ==============================================*/
#define PORT_SPI (PORTC)  /**< PORT register for SPI port */

#define SPI_MOSI (1<<5)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<6)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<7)  /**< PIN mask for SCK pin */
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
#if BOARD_TYPE == BOARD_ATZBX233XPRO || \
    BOARD_TYPE == BOARD_ATZBX212BXPRO

/*
 * RED    : PA6 : 0
 * YELLOW : PA7 : 1
 * GREEN  : PA5 : 2
 *
 * all low-active
 */
#define LED_PORT PORTA
#define LED_0_MSK (1<<6)
#define LED_1_MSK (1<<7)
#define LED_2_MSK (1<<5)
#define LED_NUMBER (3)
#define LED_ACTIVITY (0)

#define LED_INIT() \
    do {\
    LED_PORT.OUTCLR = (LED_0_MSK|LED_1_MSK|LED_2_MSK);\
    LED_PORT.DIRSET = (LED_0_MSK|LED_1_MSK|LED_2_MSK);\
    } while(0)


#define LED_SET(x) \
  switch (x) { \
  case 0: LED_PORT.OUTCLR = LED_0_MSK; break; \
  case 1: LED_PORT.OUTCLR = LED_1_MSK; break; \
  case 2: LED_PORT.OUTCLR = LED_2_MSK; break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 0: LED_PORT.OUTSET = LED_0_MSK; break; \
  case 1: LED_PORT.OUTSET = LED_1_MSK; break; \
  case 2: LED_PORT.OUTSET = LED_2_MSK; break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) LED_PORT.OUTCLR = LED_0_MSK; else LED_PORT.OUTSET = LED_0_MSK; \
  if (x & 2) LED_PORT.OUTCLR = LED_1_MSK; else LED_PORT.OUTSET = LED_1_MSK; \
  if (x & 4) LED_PORT.OUTCLR = LED_2_MSK; else LED_PORT.OUTSET = LED_1_MSK; \
  } while (0)

#define LED_GET_VALUE() ( \
  ((LED_PORT.IN & LED_0_MSK)? 1: 0) | \
  ((LED_PORT.IN & LED_1_MSK)? 2: 0) | \
  ((LED_PORT.IN & LED_2_MSK)? 4: 0) \
              )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: LED_PORT.OUTTGL = LED_0_MSK; break; \
  case 1: LED_PORT.OUTTGL = LED_1_MSK; break; \
  case 2: LED_PORT.OUTTGL = LED_2_MSK; break; \
  }

#elif BOARD_TYPE == BOARD_ATZBX233USB

/*
 * LED0 : PA5 : 0
 * LED1 : PA4 : 1
 *
 * all low-active
 */
#define LED_PORT PORTA
#define LED_0_MSK (1<<5)
#define LED_1_MSK (1<<4)
#define LED_NUMBER (2)
#define LED_ACTIVITY (0)

#define LED_INIT() \
    do {\
    LED_PORT.OUTCLR = (LED_0_MSK|LED_1_MSK);\
    LED_PORT.DIRSET = (LED_0_MSK|LED_1_MSK);\
    } while(0)


#define LED_SET(x) \
  switch (x) { \
  case 0: LED_PORT.OUTSET = LED_0_MSK; break; \
  case 1: LED_PORT.OUTSET = LED_1_MSK; break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 0: LED_PORT.OUTCLR = LED_0_MSK; break; \
  case 1: LED_PORT.OUTCLR = LED_1_MSK; break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) LED_PORT.OUTSET = LED_0_MSK; else LED_PORT.OUTCLR = LED_0_MSK; \
  if (x & 2) LED_PORT.OUTSET = LED_1_MSK; else LED_PORT.OUTCLR = LED_1_MSK; \
  } while (0)

#define LED_GET_VALUE() ( \
  ((LED_PORT.IN & LED_0_MSK)? 0: 1) | \
  ((LED_PORT.IN & LED_1_MSK)? 0: 2) \
              )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: LED_PORT.OUTTGL = LED_0_MSK; break; \
  case 1: LED_PORT.OUTTGL = LED_1_MSK; break; \
  }


#elif BOARD_TYPE == BOARD_XMA1U233XPRO

/* LED located on XMEGA A1U Xplained Pro */

#define LED_PORT     PORTQ_OUT
#define LED_DDR      PORTQ_DIR
#define LED_MASK     (0x08)
#define LED_SHIFT    (3)
#define LEDS_INVERSE (0)
#define LED_NUMBER   (1)

#else

#define LED_PORT     PORTB_OUT
#define LED_DDR      PORTB_DIR
#define LED_MASK     (0x07)
#define LED_SHIFT    (0)
#define LEDS_INVERSE (0)
#define LED_NUMBER   (3)

#endif /* BOARD_TYPE == BOARD_ATZBX233XPRO  .... */


/*=== KEY access macros ==============================================*/

#if BOARD_TYPE == BOARD_ATZBX233XPRO || BOARD_TYPE == BOARD_ATZBX212BXPRO

# define MASK_KEY     (1<<2)
# define SHIFT_KEY    (2)
# define KEY_INIT()  do{ PORTF.PIN2CTRL = PORT_OPC_PULLUP_gc; PORTF.DIRCLR = MASK_KEY; }while(0)
# define KEY_GET()   ((PORTF.IN & MASK_KEY) >> SHIFT_KEY)

#elif BOARD_TYPE == BOARD_ATZBX233USB
#define NO_KEYS (1)

#elif BOARD_TYPE == BOARD_XMA1U233XPRO

/* Pushbutton located on XMEGA A1U Xplained Pro */
# define MASK_KEY     (1<<2)
# define SHIFT_KEY    (2)
# define KEY_INIT()  do{ PORTQ.PIN2CTRL = PORT_OPC_PULLUP_gc; PORTQ.DIRCLR = MASK_KEY; }while(0)
# define KEY_GET()   ((PORTQ.IN & MASK_KEY) >> SHIFT_KEY)

#else

# define MASK_KEY     (0x08)
# define SHIFT_KEY    (3)
# define INVERSE_KEYS (1)
# define KEY_INIT() do{ PORTB.PIN3CTRL = PORT_OPC_PULLUP_gc; PORTB.DIRCLR = MASK_KEY; }while(0)
# define KEY_GET()   ((PORTB.IN & MASK_KEY) >> SHIFT_KEY)

#endif

#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() \
        do{\
        } while(0)

#define SLEEP_ON_KEY_vect PORTD_INT0_vect

/*=== Host Interface ================================================*/
#if BOARD_TYPE == BOARD_ATZBX233USB

//#define HIF_TYPE (HIF_ATXMUSB)
#define HIF_TYPE (HIF_NONE)
#define USB_VID             URACOLI_USB_VID
#define USB_PID             URACOLI_USB_PID
#define USB_BCD_RELEASE     URACOLI_USB_BCD_RELEASE
#define USB_VENDOR_NAME     URACOLI_USB_VENDOR_NAME
#define USB_PRODUCT_NAME    URACOLI_USB_PRODUCT_NAME

#elif BOARD_TYPE == BOARD_XMA1U233XPRO

/* Virtual COM of EDBG connected to USARTE0 */
#define HIF_TYPE (HIF_USARTE0)

#else

#define HIF_TYPE (HIF_USARTD0)

#endif /* BOARD_TYPE == BOARD_ATZBX233USB */

/*=== TIMER Interface ===============================================*/

#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCD0.CNT)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCD0.CTRLA = TC_CLKSEL_DIV1_gc; \
        TCD0.INTFLAGS = TC0_OVFIF_bm; \
        TCD0.INTCTRLA = TC_OVFINTLVL_HI_gc; \
    }while(0)
#define TIMER_IRQ_vect TCD0_OVF_vect

#define TWI_INST TWIE

#endif /* BOARD_CBB2XX_H */
