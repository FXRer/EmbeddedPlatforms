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

/* uracoli inclusions */
#include <board.h>

/* project inclusions */
#include "sensorapp.h"
#include "i2c.h"
#include "sht21.h"


#if HAS_SHT == 1

#if 0
/*
 * \brief Soft reset
 *
 */
void sht21_softreset(void)
{
	uint8_t stat;
	stat = i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(STH21_CMD_SOFTRESET);
			if(TW_MT_DATA_ACK == stat){
				/* do nothing */
				asm volatile ("nop");
			}
		}
	}
	i2c_stopcondition();
}
#endif
/*
 * \brief Trigger measurement
 *
 * @param cmd Command to declare which measurement is triggerd, use SHT21_CMD_xxx definitions
 */
void sht21_triggerMeas(uint8_t cmd)
{
	uint8_t stat;
	stat = i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(cmd);
			if(TW_MT_DATA_ACK == stat){
				/* do nothing */
			}
		}
	}
	i2c_stopcondition();
}

/*
 * \brief Read the previously triggered measurement
 * This can be either temperature or humidity result, depending
 * on what was triggerd
 *
 * @param *res Pointer to result
 * @return 0 for fail (measuremnt not finished), >0 for success
 */
uint8_t sht21_readMeas(uint16_t *res)
{
	uint8_t stat;
	uint8_t tmp;
	uint8_t crc;
	uint8_t ret = 0;

	stat = i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_READ);
		if(TW_MR_SLA_ACK == stat){
			stat = i2c_readbyte(&tmp, 1);		/* Auto-Ack */
			*res = tmp << 8;	/* high byte */
			stat = i2c_readbyte(&tmp, 1);		/* Auto-Ack */
			*res += tmp;		/* low byte */
			stat = i2c_readbyte(&crc, 0);		/* No Ack */
			ret = 1;
		}else{
			ret = 0;
			/* not yet finished */
		}
	}
	i2c_stopcondition();

	/* could compute checksum here */

	return ret;	/* success */
}
#if 0
/*
 * \brief Read sensor unique identification
 *
 * @param *buf Pointer to memory to store into
 * @param maxbytes for buffer, must be >8
 */
uint8_t sht21_identification(uint8_t *buf, uint8_t maxbytes)
{
	uint8_t stat;
	uint8_t crc;
	static uint8_t tmp[8];
	uint8_t *ptmp = tmp;

	stat=i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(0xFA);
			stat = i2c_writebyte(0x0F);
			stat = i2c_startcondition();	/* Repeated Start */
			if(TW_REP_START == stat){
				stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_READ);
				if(TW_MR_SLA_ACK == stat){
					i2c_readbyte(ptmp++, 1);	/* SNB_3 */
					i2c_readbyte(&crc, 1);
					i2c_readbyte(ptmp++, 1);	/* SNB_2 */
					i2c_readbyte(&crc, 1);
					i2c_readbyte(ptmp++, 1);	/* SNB_1 */
					i2c_readbyte(&crc, 1);
					i2c_readbyte(ptmp++, 1);	/* SNB_0 */
					i2c_readbyte(&crc, 0);	/* NAK */
				}
			}
		}
	}
	i2c_stopcondition();

	stat=i2c_startcondition();
	if(TW_START == stat){
		stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_WRITE);
		if(TW_MT_SLA_ACK == stat){
			stat = i2c_writebyte(0xFC);
			stat = i2c_writebyte(0xC9);
			stat = i2c_startcondition();	/* Repeated Start */
			if(TW_REP_START == stat){
				stat = i2c_writebyte(BOARD_SHT21_I2CADDR | TW_READ);
				if(TW_MR_SLA_ACK == stat){
					i2c_readbyte(ptmp++, 1);	/* SNC_1 */
					i2c_readbyte(ptmp++, 1);	/* SNC_0 */
					i2c_readbyte(&crc, 1);
					i2c_readbyte(ptmp++, 1);	/* SNA_1 */
					i2c_readbyte(ptmp++, 1);	/* SNA_0 */
					i2c_readbyte(&crc, 0);	/* NAK */
				}
			}
		}
	}
	i2c_stopcondition();

	/* reformat */
	buf[7] = tmp[6];
	buf[6] = tmp[7];
	buf[5] = tmp[0];
	buf[4] = tmp[1];
	buf[3] = tmp[2];
	buf[2] = tmp[3];
	buf[1] = tmp[5];
	buf[0] = tmp[4];

	return 0;
}
#endif
#endif /* HAS_SHT == 1 */
/* EOF */
