/* Copyright (c) 2010 Daniel Thiele
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
 * 
 * @file
 * @brief IBDT Ranging Hardware with ATmega644 + AT86RF231
 *
 * The wiring of the radio and the ATmega is shown below:
 *
 * @todo correct wiring
 *
<pre>
     AVR       RF231
     ---       -----

     PD3:2   <--  KEY
     PD5:4 -->  LED

</pre>


@par Build Options

 */

/** ID String for this hardware */
#ifndef BOARD_XPROTRX_H
#define BOARD_XPROTRX_H

/*=== Hardware Components ============================================*/

#if defined(xprotrx_soc)

#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_ATMEGA1284RFR2)
#endif

#elif defined(xprotrx_ext232)

#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF232)
#endif

#elif defined(xprotrx_ext212)

#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF212)
#endif

#else
#error "Unsupported BOARD_TYPE"
#endif

/*=== Compile time parameters ========================================*/

/*=== TRX pin access macros ==========================================*/
#if BOARD_TYPE == BOARD_XPROTRX_EXT

/* define empty macros, since RSTN pin is not used */
#define TRX_RESET_INIT() do{ }while(0)
#define TRX_RESET_HIGH() do{ }while(0)
#define TRX_RESET_LOW()  do{ }while(0)
 
#define DDR_TRX_SLPTR   DDRE            /**< PORT register for SLP_TR pin */
#define PORT_TRX_SLPTR  PORTE           /**< DDR register for SLP_TR pin */
#define MASK_TRX_SLPTR  (1<<PE0)      /**< PIN mask for SLP_TR pin */

#endif /* BOARD_TYPE == BOARD_XPROTRX_EXT */

/*=== IRQ access macros ==============================================*/

#if BOARD_TYPE == BOARD_XPROTRX_EXT

# define TRX_IRQ_vect    TIMER1_CAPT_vect      /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{ } while(0) /** high level INT0 */

/** disable TRX interrupt */
#define DI_TRX_IRQ() { }
/** enable TRX interrupt */
#define EI_TRX_IRQ() { }

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

#endif /* BOARD_TYPE == BOARD_XPROTRX_EXT */

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDRB)   /**< DDR register for SPI port */
#define PORT_SPI (PORTB)  /**< PORT register for SPI port */

#define SPI_SS   (1<<PB0)  /**< PIN mask for SS pin */
#define SPI_SCK  (1<<PB1)  /**< PIN mask for SCK pin */
#define SPI_MOSI (1<<PB2)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<PB3)  /**< PIN mask for MISO pin */

#define SPI_DATA_REG SPDR  /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |= SPI_SCK | SPI_SS;

    SPCR = ((1<<SPE) | (1<<MSTR));

    SPCR &= ~((1<<SPR1) | (1<<SPR0) );
    SPSR &= ~(1<<SPI2X);

    SPCR |= (spirate & 0x03);
    SPSR |= ((spirate >> 2) & 0x01);

}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()        do { while((SPSR & (1<<SPIF)) == 0);} while(0)

/*=== LED access macros ==============================================*/
# define NO_LEDS       (1)

/*=== KEY access macros ==============================================*/
#define NO_KEYS        (1)

/*=== Host Interface ================================================*/
#define HIF_TYPE    HIF_UART_1

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= ((1<<CS10)); \
        TIMSK1 |= (1<<TOIE1); \
    }while(0)
#define TIMER_IRQ_vect   TIMER1_OVF_vect
#endif /* BOARD_XPROTRX_H */
