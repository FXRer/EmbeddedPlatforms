/* Copyright (c) 2009 - 2015 Daniel Thiele, Axel Wachtler
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
 * @brief Radiofaro, Arduino like board
 *
 *
 *
<pre>

Fuses/Locks:
     LF: 0xF6 - 16MHz Crystal oscillator
     HF: 0x91 - without boot loader
     HF: 0x90 - with boot loader
     EF: 0xFE
     LOCK: 0xEF - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes
</pre>


@par Build Options
  - radiofaro : Rev 1.3 .. Rev 1.5
  - radiofaro_v1 : < Rev 1.3
  - radiofaro2 : > Rev 2.0

 */

#ifndef BOARD_RADIOFARO_H
#define BOARD_RADIOFARO_H

#if defined(radiofaro)
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#elif defined(radiofaro_v1)
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(radiofaro2)
#define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#endif
/*=== Compile time parameters ========================================*/

/*=== Hardware Components ============================================*/

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

#if BOARD_TYPE == BOARD_RADIOFARO || BOARD_TYPE == BOARD_RADIOFARO_V1
# define LED_PORT      PORTG
# define LED_DDR       DDRG
# define LED_MASK      (0x06)
# define LED_SHIFT     (1)
# define LEDS_INVERSE  (0)
# define LED_NUMBER    (2)
# define LED_DEFAULT   (0)

#elif BOARD_TYPE == BOARD_RADIOFARO2

#define LED_INIT() \
    do {\
        PORTB &= ~(1<<PB1); \
        PORTG &= ~((1<<PG2) | (1<<PG5));\
        DDRB |= (1<<PB1); \
        DDRG |= (1<<PG2) | (1<<PG5);\
    } while(0)


#define LED_SET(x) \
  switch (x) { \
  case 0: PORTG |= (1<<PG2); break; \
  case 1: PORTG |= (1<<PG5); break; \
  case 2: PORTB |= (1<<PB1); break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 0: PORTG &= ~(1<<PG2); break; \
  case 1: PORTG &= ~(1<<PG5); break; \
  case 2: PORTB &= ~(1<<PB1); break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) PORTG |= (1<<PG2); else PORTG &= ~(1<<PG2); \
  if (x & 2) PORTG |= (1<<PG5); else PORTG &= ~(1<<PG5); \
  if (x & 4) PORTB |= (1<<PB1); else PORTB &= ~(1<<PB1); \
  } while (0)

#define LED_GET_VALUE() ( \
  ((PORTG & (1<<PG2))? 1: 0) | \
  ((PORTG & (1<<PG5))? 2: 0) | \
  ((PORTB & (1<<PB1))? 4: 0) \
              )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: PORTG ^= (1<<PG2); break; \
  case 1: PORTG ^= (1<<PG5); break; \
  case 2: PORTB ^= (1<<PB1); break; \
  }

#define LED_NUMBER (3)
#define LED_ACTIVITY (0)

#endif

/*=== KEY access macros ==============================================*/

#define NO_KEYS        (1)

/*=== Host Interface ================================================*/
#if BOARD_TYPE == BOARD_RADIOFARO_V1
# define HIF_TYPE    HIF_UART_0
#else
# define HIF_TYPE    HIF_UART_1
#endif


#define TRX_RESET_LOW()   do { TRXPR &= ~(1<<TRXRST); } while (0)
#define TRX_RESET_HIGH()  do { TRXPR |= (1<<TRXRST); } while (0)
#define TRX_SLPTR_LOW()   do { TRXPR &= ~(1<<SLPTR); } while (0)
#define TRX_SLPTR_HIGH()  do { TRXPR |= (1<<SLPTR); } while (0)

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

#endif /* BOARD_RADIOFARO_H */
