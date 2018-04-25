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

/* project inclusions */
#include "i2c.h"
#include "sensors.h"
#include "sht21.h"

static uint8_t i2c_addr;

/*
 * \brief Initialize SHT21
 * Assign I2C address and use this for further accesses
 */
void sht21_init(uint8_t addr)
{
    i2c_addr = addr;
}

/*
 * \brief Soft reset
 *
 */
void sht21_softreset(void)
{
    uint8_t wbuf[1] = {STH21_CMD_SOFTRESET};

    i2c_master_writeread(i2c_addr, wbuf, sizeof(wbuf)/sizeof(wbuf[0]), (void*)0x0000, 0);
}

void sht21_write_user_reg(uint8_t value)
{
    uint8_t wbuf[2] = {0xE6, value};

    i2c_master_writeread(i2c_addr, wbuf, sizeof(wbuf)/sizeof(wbuf[0]), (void*)0x0000, 0);
}

uint8_t sht21_read_user_reg(void)
{
    uint8_t wbuf[1] = {0xE7};
    uint8_t rbuf[1];

    i2c_master_writeread(i2c_addr, wbuf, sizeof(wbuf)/sizeof(wbuf[0]),
    		rbuf, sizeof(rbuf)/sizeof(rbuf[0]));

    return rbuf[0];
}

/*
 * \brief Trigger measurement
 *
 * @param cmd Command to declare which measurement is triggerd, use SHT21_CMD_xxx definitions
 */
void sht21_triggerMeas(uint8_t cmd)
{
    uint8_t wbuf[1] = {cmd};

    i2c_master_writeread(i2c_addr, wbuf, sizeof(wbuf)/sizeof(wbuf[0]), (void*)0x0000, 0);
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
    uint8_t ret = 0;
    uint8_t rbuf[3];

    ret = i2c_master_writeread(i2c_addr, (void*)0x0000, 0, rbuf, sizeof(rbuf)/sizeof(rbuf[0]));
    if(ret > 0)
    {
        *res = (rbuf[0] << 8) | (rbuf[1] << 0);
    }

    /* could compute checksum here */

    return ret;
}

/*
 * \brief Read sensor unique identification
 *
 * @param *buf Pointer to memory to store into
 * @param maxbytes for buffer, must be >8
 */
void sht21_identification(uint8_t *buf, uint8_t maxbytes)
{
    uint8_t wbuf[2] = {0xFA, 0x0F};
    uint8_t rbuf1[8];
    i2c_master_writeread(i2c_addr,
                         wbuf, sizeof(wbuf)/sizeof(wbuf[0]),
                         rbuf1, sizeof(rbuf1)/sizeof(rbuf1[0]));

    wbuf[0] = 0xFC;
    wbuf[1] = 0xC9;
    uint8_t rbuf2[6];
    i2c_master_writeread(i2c_addr,
                         wbuf, sizeof(wbuf)/sizeof(wbuf[0]),
                         rbuf2, sizeof(rbuf2)/sizeof(rbuf2[0]));

    /* reformat */
    buf[7] = rbuf2[3];
    buf[6] = rbuf2[4];
    buf[5] = rbuf1[0];
    buf[4] = rbuf1[2];
    buf[3] = rbuf1[4];
    buf[2] = rbuf1[6];
    buf[1] = rbuf2[1];
    buf[0] = rbuf2[0];
}

/* EOF */
