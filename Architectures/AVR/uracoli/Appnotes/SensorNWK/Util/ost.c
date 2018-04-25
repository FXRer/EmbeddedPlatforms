/*
 * ost.c
 *
 *  Created on: 28.02.2011
 *      Author: dthiele
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ost.h"

#define TICK_US (128UL)

static ost_cb_t *func;

#if defined(__AVR_ATmega88P__) || defined(__AVR_ATmega88PA__) || \
		defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PA__) || \
		defined(__AVR_ATmega1281__) || defined(__AVR_ATmega128RFA1__) ||\
		defined(__AVR_ATmega256RFR2__)

#define TIMERx_OVF_vect TIMER0_OVF_vect
#define TIMERx_COMPA_vect TIMER0_COMPA_vect
#define TIMERx_COMPB_vect TIMER0_COMPB_vect

#define TCCRxA TCCR0A
#define TCCRxB TCCR0B
#define TIMSKx TIMSK0
#define OCIExA OCIE0A
#define TCNTx TCNT0
#define OCRxA OCR0A
#define TIFRx TIFR0
#define CSx0 CS00
#define CSx1 CS01
#define CSx2 CS02
#define WGMx0 WGM00
#define WGMx1 WGM01

#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)

#define TIMERx_OVF_vect TIMER1_OVF_vect
#define TIMERx_COMPA_vect TIMER1_COMPA_vect
#define TIMERx_COMPB_vect TIMER1_COMPB_vect

#define TCCRxA TCCR1A
#define TCCRxB TCCR1B
#define TIMSKx TIMSK1
#define OCIExA OCIE1A

#define TCNTx TCNT1
#define OCRxA OCR1A
#define TIFRx TIFR1
#define CSx0 CS10
#define CSx1 CS11
#define CSx2 CS12
#define WGMx0 WGM10
#define WGMx1 WGM11

#else
//#error "Unsupported MCU"
#define NO_OST_AVAILABLE (1)
#endif

#ifndef NO_OST_AVAILABLE
ISR(TIMERx_OVF_vect, ISR_BLOCK)
{
    /* do nothing */
}
#endif

#ifndef NO_OST_AVAILABLE
ISR(TIMERx_COMPA_vect, ISR_BLOCK)
{
    if(!(TCCRxA & (1<<WGMx1)))	/* indicator for periodic */
    {
        TCCRxB=0; /* stop */
        TIMSKx &= ~(1<<OCIExA);
    }
    func();
}
#endif

#ifndef NO_OST_AVAILABLE
ISR(TIMERx_COMPB_vect, ISR_BLOCK)
{
    /* do nothing */
}
#endif

void ost_start(ost_cb_t *cb, uint32_t duration_us, ost_type_t type)
{
#ifndef NO_OST_AVAILABLE
    TCCRxA = 0; /* normal operation */
    TCCRxB = 0; /* stop */
    TCNTx = 0;
    TIFRx = 0xFF; /* clear all flags */

    func = cb;

    TIMSKx = (1<<OCIExA);
    OCRxA = (duration_us * (F_CPU / 1000000UL)) / TICK_US;
    if(OST_PERIODIC == type)
    {
        TCCRxA = (1<<WGMx1); /* CTC mode: indicator for periodic */
    } else {
    	/* do nothing */
    }
    TCCRxB |= (1<<CSx2) | (1<<CSx0); /* PS=1024, Tick=128us@F_CPU=8MHz */
#endif
}

/*
 * \brief Restart running timer
 * Keep settings from previously ost_start and zero current timer value
 */
void ost_restart(void)
{
#ifndef NO_OST_AVAILABLE
    TCNTx = 0;
#endif
}

uint32_t ost_stop()
{
#ifndef NO_OST_AVAILABLE
    TCCRxB = 0;
    TIMSKx &= ~(1<<OCIExA);
    return (TCNTx*TICK_US) / (F_CPU / 1000000UL);
#endif
}

uint32_t ost_getperiod()
{
#ifndef NO_OST_AVAILABLE
    return (OCRxA * 64UL) / (F_CPU / 1000000UL);
#endif
}


/* EOF */
