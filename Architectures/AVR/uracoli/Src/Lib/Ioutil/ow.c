/* Copyright (c) 2014 Daniel Thiele, Axel Wachtler
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

/**
 * @file One-Wire protocol using a single AVR I/O-pin
 * @brief ....
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */
#include <util/delay.h>
#include <util/crc16.h>
#include "board.h"
#include "ow.h"

/* === macros ============================================================== */
#ifndef DELAY_US
#define DELAY_US(x) _delay_us(x) // TODO: use the uracoli macro
#endif

#define OW_DEBUG (0)
#if OW_DEBUG == 1
# include "hif.h"
# define DBG(...) PRINTF(__VA_ARGS__)
# define DBGP(x) PRINT(x)
#else
# define DBG(...)
# define DBGP(x)
#endif

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */

void ow_init(void)
{
#if defined(HAS_OW)

#if defined(DBG_SLOT)
    PINOUTP(DBG_SLOT);
    PINHI(DBG_SLOT);
#endif

    PININP(DQ);
    PINLO(DQ); /* disable pullup */
    DBG("ow_init %d\n\r", 42);
#endif /* if defined(HAS_OW)*/
}

/*
 * \brief Issue Reset pulse and check for presence pulse
 *
 *
 */
uint8_t ow_reset(void)
{

    uint8_t ret = 0;

#if defined(HAS_OW)

#if defined(DBG_SLOT)
    PINLO(DBG_SLOT);
#endif

    /* Reset Pulse */
    PINLO(DQ);
    PINOUTP(DQ);
    DELAY_US(480);
    PININP(DQ); /* let charge to high */
    DELAY_US(240);

    ret = PINGET(DQ);
    DELAY_US(240);

#if defined(DBG_SLOT)
    PINHI(DBG_SLOT);
#endif
#endif /* if defined(HAS_OW)*/
    DBG("ow_reset ret = %d\n\r", ret);
    return ret;
}

static inline uint8_t ow_bit_rw(uint8_t bit, uint8_t read)
{
    volatile uint8_t ret = 0;

#if defined(HAS_OW)
#if defined(DBG_SLOT)
    PINLO(DBG_SLOT);
#endif

    PINOUTP(DQ); /* pull low */
    DELAY_US(4); /* >1 us acc. to spec */

    if( bit || read )
    {
        PININP(DQ); /* "1" let charge to high */
        DELAY_US(8);
        ret=PINGET(DQ); /* also for write action */
        DELAY_US(56-8);
    }
    else
    {
        DELAY_US(56); /* "0" let pulled low */
        PININP(DQ); /* "1" let charge to high */
    }

#if defined(DBG_SLOT)
    PINHI(DBG_SLOT);
#endif

    DELAY_US(16); /* Recovery */
#endif /* if defined(HAS_OW)*/

    return ret;
}

void ow_byte_write(uint8_t byte)
{
#if defined(HAS_OW)
    uint8_t i;
    /* LSB first */
    for(i=0; i<8; i++){
        ow_bit_rw(byte & (1<<i), 0);
    }
    DBG("ow_wr=%02x\n", byte);

#endif /* if defined(HAS_OW)*/
}

uint8_t ow_byte_read(void)
{
    uint8_t byte = 0;

#if defined(HAS_OW)
    uint8_t i;
    /* LSB first */
    for(i=0;i<8;i++)
    {
        if(ow_bit_rw(0, 1))
        {
            byte |= (1<<i);
        }
    }
    DBG("ow_rd=%02x\n", byte);

#endif /* if defined(HAS_OW)*/

    return byte;
}

bool ow_crc_valid(uint8_t * pdata, uint8_t size)
{
    uint8_t i, crc = 0;
    for (i = 0; i < size; i++)
    {
        crc = _crc_ibutton_update(crc, pdata[i]);
    }
    DBG("ow_crc_valid: size=%d, crc = 0x%02x\n", size, crc);
    return (crc == 0);
}

/*
 * \brief
 * @return >0 if more devices available, 0 when finished
 *
 *
 * Implemenation acc. to iButton Standard, Figure 5-3
 * http://pdfserv.maximintegrated.com/en/an/AN937.pdf
 *
 *
 */
bool ow_master_searchrom(ow_serial_t *ser, bool first)
{
    bool rv ;

    rv = false;
#if defined(HAS_OW)
    /* re-entrant variables */
    static uint8_t lastdiscr = 0;
    static uint8_t done = 0;

    uint8_t discr;
    uint8_t romidx;
    uint8_t rombit;
    uint8_t bita, bitb;

    /* dummy address . crc breaker */
    *ser = 0XFEEDEDDEADBEEFLL;

    if(first)
    {
        lastdiscr = 0;
        done = 0;
        DBGP("first search round\n");
    }

    if( done != 0)
    {
        DBGP("search done\n");
        done = 0;
        rv = false;
    }
    else
    {
        if(0 == ow_reset())
        {
            DBGP("no presence pulse\n");
            lastdiscr = 0;
            rv = false;
        }
        else
        {
            DBGP("issue SEARCH_ROM cmd\n");
            romidx = 1;
            discr = 0;
            ow_byte_write(OW_ROMCMD_SEARCH);
            do
            {

                bita = ow_bit_rw(0, 1);
                bitb = ow_bit_rw(0, 1);

                if( (bita == 1) && (bitb == 1))
                {
                    DBGP("no device present\n");
                    lastdiscr = 0;
                    done = 1;
                    rv = false;
                    break;
                }
                else if( (bita == 0) && (bitb == 0) )
                {
                    DBG("[%d] discrepancy\n", romidx);
                    if(romidx == lastdiscr)
                    {
                        rombit = 1;
                    }
                    else if(romidx > lastdiscr)
                    {
                        rombit = 0;
                        discr = romidx;
                    }
                    else if(rombit == 0)
                    {
                        discr = romidx;
                    }
                }
                else
                {
                    //DBG("[%d] bita != bitb -> no discrepancy\n", romidx);
                    rombit = bita;
                }

                /* explicit write of each bit, because we added a CRC breaker */
                if (rombit)
                {
                    *ser |= (1LL<<(romidx-1));
                }
                else
                {
                    *ser &= ~(1LL<<(romidx-1));
                }
                ow_bit_rw(rombit, 0);

            }
            while(++romidx <= 64);

            lastdiscr = discr;
            if( 0 == lastdiscr )
            {
                done = 1;
            }
            else
            {
                rv = true;
            }
        }
    }
#endif /* if defined(HAS_OW)*/

    return rv; /* more available */
}

void ow_master_matchrom(ow_serial_t ser)
{
#if defined(HAS_OW)
    uint8_t i;
    ow_reset();
    ow_byte_write(OW_ROMCMD_MATCH);

    for(i=0; i<64; i++)
    {
        ow_bit_rw(ser&(1LL<<i)? 1 : 0 , 0);
    }
#endif /* if defined(HAS_OW)*/

}

/* EOF */
