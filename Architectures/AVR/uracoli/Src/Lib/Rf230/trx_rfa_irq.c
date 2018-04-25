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

/* === Includes ============================================================ */
#include "board.h"
#include "transceiver.h"
#include <stdlib.h>
#include <avr/interrupt.h>

#if defined(TRX_IF_RFA1)
/* === globals ============================================================= */
static trx_irq_handler_t pIrqHandler = 0;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/* === internal functions =================================================== */

/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_PLL_LOCK_vect();
#else
ISR(TRX24_PLL_LOCK_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_PLL_LOCK);
    }
}

/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_PLL_UNLOCK_vect();
#else
ISR(TRX24_PLL_UNLOCK_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_PLL_UNLOCK);
    }
}

/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_RX_START_vect();
#else
ISR(TRX24_RX_START_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_RX_START);
    }
}
/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_RX_END_vect();
#else
ISR(TRX24_RX_END_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_RX_END);
    }
}
/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_CCA_ED_DONE_vect();
#else
ISR(TRX24_CCA_ED_DONE_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_CCA_ED);
    }
}

/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX24_XAH_AMI_vect();
#else
ISR(TRX24_XAH_AMI_vect, ISR_NOBLOCK)
#endif
{
    if (pIrqHandler)
    {
        pIrqHandler(TRX_IRQ_AMI);
    }
}

/* following vectors are combined with other vectors to be abstracted with 
 * SPI transceivers. 
 * If applications require them, they can be re-defined inside application 
 * and they would override the ones defined here because of weak-links
 */
 
/* these for RFA and RFR devices */
ISR(TRX24_TX_END_vect, ISR_ALIASOF(TRX24_RX_END_vect));
ISR(TRX24_AWAKE_vect,  ISR_ALIASOF(TRX24_CCA_ED_DONE_vect));

/* these for RFR devices only */
#if defined(__AVR_ATmega64RFR2__) || defined(__AVR_ATmega128RFR2__) || defined(__AVR_ATmega256RFR2__) || \
	defined(__AVR_ATmega644RFR2__) || defined(__AVR_ATmega1284RFR2__) || defined(__AVR_ATmega2564RFR2__)
EMPTY_INTERRUPT(TRX24_TX_START_vect);
ISR(TRX24_AMI0_vect,        ISR_ALIASOF(TRX24_XAH_AMI_vect));
ISR(TRX24_AMI1_vect,        ISR_ALIASOF(TRX24_XAH_AMI_vect));
ISR(TRX24_AMI2_vect,        ISR_ALIASOF(TRX24_XAH_AMI_vect));
ISR(TRX24_AMI3_vect,        ISR_ALIASOF(TRX24_XAH_AMI_vect));

#endif


/* === external functions =================================================== */

void trx_set_irq_handler(trx_irq_handler_t irqhandler)
{
    pIrqHandler = irqhandler;
}

#endif /* #if defined(TRX_IF_RFA1) */
/* EOF */
