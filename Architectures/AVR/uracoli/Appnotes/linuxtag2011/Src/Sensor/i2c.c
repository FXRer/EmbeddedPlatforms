/* Copyright (c) 2011 Daniel Thiele
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

/* avr-libc inclusions */
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

/* project inclusions */
#include "sensorapp.h"
#include "i2c.h"

#if HAS_I2C == 1

/*
 * \brief Initialize TWI module
 *
 * SCL_freq = F_CPU / (16 + 2*TWBR*TWPS)
 */
void i2c_init(void)
{
	TWSR |= _BV(TWPS0);	/* PS=1 */
	TWBR = 8;
}

/*
 * \brief Send start condition onto bus
 *
 * @return Status code
 */
uint8_t i2c_startcondition(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	while(!(TWCR & _BV(TWINT)));
	return TW_STATUS;
}

/*
 * \brief Send start condition onto bus
 *
 * @return Status code
 */
uint8_t i2c_stopcondition(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	_delay_us(10);	/* needed for next start condition, to be analyzed */
	return TW_STATUS;
}

/*
 * \brief Write a single byte to I2C
 * This can be address byte + RW flag or data byte
 *
 *	@param b Byte to write
 *	@return Status code
 */
uint8_t i2c_writebyte(uint8_t b)
{
	TWDR = b;
	TWCR = _BV(TWINT) | _BV(TWEN);
	while(!(TWCR & _BV(TWINT)));
	return TW_STATUS;
}

/*
 * \brief Read a single byte from I2C in master-receive mode
 *
 * @param *b Pointer to byte to read in
 * @param autoack Auto acknowledge the received byte

 *	@return Status code
 */
uint8_t i2c_readbyte(uint8_t *b, uint8_t autoack)
{
	if(autoack){
		TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	}else{
		TWCR = _BV(TWINT) | _BV(TWEN);
	}
	while(!(TWCR & _BV(TWINT)));
	*b = TWDR;
	return TW_STATUS;
}
#endif /* HAS_I2C == 1 */
/* EOF */
