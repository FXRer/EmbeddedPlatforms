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


/*
 * ===========================================================================
 * This file contains refactored code from hif_rf230.c,
 * which is part of Atmels software package "IEEE 802.15.4 MAC for AVR Z-Link"
 * ===========================================================================
 */
/*
 * Copyright (c) 2006, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* $Id$ */
/**
 * @file
 * @brief Implementation of CRC frame read.
 *
 */

/* === Includes ============================================================ */
#include "board.h"
#include "transceiver.h"
#include <stdlib.h>
#include <stdbool.h>
#include <util/crc16.h>

#if !defined(TRX_IF_RFA1)
/* === Globals ============================================================= */

uint8_t trx_frame_read_crc(uint8_t *data, uint8_t datasz, bool *crc_ok)
{
    uint8_t length = 0;
    uint8_t i, tmp;
    uint16_t crc16 = 0;

    /* Select transceiver */
    SPI_SELN_LOW();

    /* start frame read */
    SPI_DATA_REG = TRX_CMD_FR;
    SPI_WAITFOR();

    /* read length */
    SPI_DATA_REG = 0;
    SPI_WAITFOR();
    length = SPI_DATA_REG;

    if ( length <= datasz)
    {
        i = length;
        crc16 = 0;
        do
        {
            SPI_DATA_REG = 0;   /* dummy out */
            SPI_WAITFOR();
            tmp = SPI_DATA_REG;
            crc16 = _crc_ccitt_update(crc16, tmp);
            *data++ = tmp;
        }
        while(--i);

        if(crc_ok != NULL)
        {
            *crc_ok = (crc16 == 0) ? true : false;
        }

    }
    else
    {
        /* we drop the frame */
        length = 0x80 | length;
    }
    SPI_SELN_HIGH();
    return length;
}


uint8_t trx_frame_read_data_crc(uint8_t *data, uint8_t datasz,
                                uint8_t *lqi, bool *crc_ok)
{
    uint8_t length = 0;
    uint8_t i, tmp;
    uint16_t crc16 = 0;
    /* Select transceiver */
    SPI_SELN_LOW();

    /* start frame read */
    SPI_DATA_REG = TRX_CMD_FR;
    SPI_WAITFOR();

    /* read length */
    SPI_DATA_REG = 0;
    SPI_WAITFOR();
    length = SPI_DATA_REG;

    if ( (length - 2) <= datasz)
    {
        i = length;
        do
        {
            SPI_DATA_REG = 0;   /* dummy out */
            SPI_WAITFOR();
            tmp = SPI_DATA_REG;
            crc16 = _crc_ccitt_update(crc16, tmp);
            if (i > 1)
            {
                *data++ = tmp;
            }
        }
        while(--i);

        if(crc_ok != NULL)
        {
            *crc_ok = (crc16 == 0) ? true : false;
        }

        if (lqi != NULL)
        {
            SPI_DATA_REG = 0;   /* dummy out */
            SPI_WAITFOR();
            *lqi = SPI_DATA_REG;
        }
    }
    else
    {
        /* we drop the frame */
        length = 0x80 | length;
    }
    SPI_SELN_HIGH();
    return length;
}
#endif /* #if !defined(TRX_IF_RFA1) */
/* EOF */