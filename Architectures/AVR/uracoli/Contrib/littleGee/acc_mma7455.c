/* Copyright (c) 2009 
    Daniel Thiele, 
    Axel Wachtler
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
 * @brief Driver Functions for Accelerometer MMA7455 from Frescale.
 * @_addtogroup grpApp...
 */
#include <stdint.h>
#include <util/twi.h>
#include <board.h>
#include "acc_mma7455.h"

/* === includes ============================================================ */


/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/**
  * \brief Read a register of MMA7455L
  *
  */
uint8_t acc_regrd(uint8_t addr)
{
	ACC_SELN_LO();
    /* MSB = 0: read command, LSB: dont care */
	SPDR = 0x00 | ( (addr & 0x3F) << 1);  
	SPI_WAITFOR();
	SPDR = 0;			/* dummy */
	SPI_WAITFOR();
	ACC_SELN_HI();
	return SPDR;
}

/**
  * \brief Read a number of registers of MMA7455L
  *
  */
uint8_t acc_regrd_multiple(uint8_t addr, uint8_t count, uint8_t *buffer)
{
	ACC_SELN_LO();
	SPDR = 0x00 | ( (addr & 0x3F) << 1);		// MSB = 0: read command, LSB: dont care
	SPI_WAITFOR();
	do{
		SPDR = 0;			// dummy
		SPI_WAITFOR();
		*buffer++ = SPDR;
	}while(--count);
	ACC_SELN_HI();
	return 0;
}

/**
  * \brief Read a register of MMA7455L
  *
  */
uint8_t acc_regwr(uint8_t addr, uint8_t data)
{
	ACC_SELN_LO();
	SPDR = 0x80 | ( (addr & 0x3F) << 1);		// MSB = 1: write command, LSB: dont care
	SPI_WAITFOR();
	SPDR = data;
	SPI_WAITFOR();
	ACC_SELN_HI();
	return data;
}

/**
  * \brief Initialize the accelerometer in SPI mode
  *
  */
void acc_init(void)
{
	ACC_SELN_PORT |= _BV(ACC_SELN_bp);
	ACC_SELN_DDR |= _BV(ACC_SELN_bp);
	acc_regwr(0x0D, 0x80);		/* set the I2C disable bit */
}

/**
  * \brief Set the measurement mode
  *
  */
void acc_setmode(uint8_t mode)
{
	uint8_t tmp;
	tmp = acc_regrd(0x16);
	tmp &= ~0x03;				// clear mode bits
	tmp |= mode;				// set measurement mode
	acc_regwr(0x16, tmp);
}

/* EOF */
