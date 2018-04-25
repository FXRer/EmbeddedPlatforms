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
#include <util/twi.h>

/* uracoli inclusions */
#include <board.h>

/* project inclusions */
#include "sensorapp.h"
#include "i2c.h"

#if HAS_MMA == 1

/*
 * \brief Read multiple bytes
 *
 * @param addr Start address
 * @param nbbytes Number of bytes to read
 * @param *buf Buffer to read bytes in
 *
 * @return Number of bytes that were read
 */
uint8_t mma7455_regrd(const uint8_t addr, uint8_t nbbytes, uint8_t *buf)
{
	uint8_t stat;

	stat = i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_ACC_MMA7455_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(addr);
			if(TW_MT_DATA_ACK == stat){
				/* repeated Start condition */
				stat=i2c_startcondition();
				if(TW_REP_START == stat){
					stat=i2c_writebyte(BOARD_ACC_MMA7455_I2CADDR | TW_READ);
					if(TW_MR_SLA_ACK == stat){
						do{
							/* NAK for last byte, ACK else */
							stat=i2c_readbyte(buf++, (nbbytes>1)?1:0);
						}while(--nbbytes && (TW_MR_DATA_ACK == stat));
					}
				}
			}
		}
	}
	i2c_stopcondition();
	return nbbytes;
}

void mma7455_regwr(const uint8_t addr, const uint8_t val)
{
	uint8_t stat;
	
	stat = i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_ACC_MMA7455_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(addr);
			if(TW_MT_DATA_ACK == stat){
				stat = i2c_writebyte(val);
				if(TW_MT_DATA_ACK == stat){
						/* do nothing */
				}
			}
		}
	}

	i2c_stopcondition();
}
#endif /* HAS_MMA == 1 */
/* EOF */
