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

/* $Id$ */
/**
 * @file Driver for Bosch Sensortec BMP085 and BMP180
 *
 *
 * @brief ....
 * @_addtogroup grpApp...
 */

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

/* project inclusions */
#include "i2c.h"
#include "sensors.h"
#include "bmp085.h"

static uint8_t i2caddr = 0;
static uint8_t osrs = BMP085_DEFAULT_OSRS;	/* OSRS */

/*
 * \brief Initialize
 *
 */
void bmp085_init(uint8_t devaddr)
{
    i2caddr = devaddr;
}

/*
 * \brief Configure Sensor
 *
 * @param posrs OSRS setting
 */
void bmp085_configure(uint8_t posrs)
{
    /* TODO: check for ongoing conversion, otherwise result is
     * misinterpreted
     */
    osrs = posrs; /* applied to sensor on next trigger */
}

/*
 * \brief Trigger a measurement
 * Common for pressure and temperature measurement
 *
 * @param mode 0x2E: temperature, 0x34: pressure (osrs=0), ...
 */
static inline void sensors_bmp085_trigger(uint8_t mode)
{
    sensors_regwr(i2caddr, 0xF4, mode);
}

/*
 * \brief Read uncompensated temperature value
 */
uint16_t bmp085_ut_read(void)
{
    uint8_t wbuf[1] = {0xF6};
    uint8_t rbuf[2];

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, 2);

    return (rbuf[0] << 8 ) | (rbuf[1] << 0);
}

/*
 * \brief Read uncompensated pressure value
 */
uint32_t bmp085_up_read(void)
{
    uint8_t wbuf[1] = {0xF6};
    uint8_t rbuf[3];

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, 3);

    return ((((uint32_t)rbuf[0] << 16 ) | ((uint32_t)rbuf[1] << 8) | ((uint32_t)rbuf[2] << 0)) >> (8-osrs));
}

/*
 * \brief Read internal calibration EEPROM
 *
 * @param *buf Pointer to buffer to read into, must be of size 22
 */
void bmp085_readee(bmp085_calibration_t *cal)
{
    uint8_t wbuf[1] = {RG_BMP085_PROM_START};
    uint8_t rbuf[BMP085_PROM_DATA_LEN];

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, BMP085_PROM_DATA_LEN);

    cal->ac1 = (rbuf[0] << 8) | (rbuf[1] << 0);
    cal->ac2 = (rbuf[2] << 8) | (rbuf[3] << 0);
    cal->ac3 = (rbuf[4] << 8) | (rbuf[5] << 0);

    cal->ac4 = (rbuf[6] << 8) | (rbuf[7] << 0);
    cal->ac5 = (rbuf[8] << 8) | (rbuf[9] << 0);
    cal->ac6 = (rbuf[10] << 8) | (rbuf[11] << 0);

    cal->b1 = (rbuf[12] << 8) | (rbuf[13] << 0);
    cal->b2 = (rbuf[14] << 8) | (rbuf[15] << 0);
    cal->mb = (rbuf[16] << 8) | (rbuf[17] << 0);
    cal->mc = (rbuf[18] << 8) | (rbuf[19] << 0);
    cal->md = (rbuf[20] << 8) | (rbuf[21] << 0);
}

/*
 * \brief Trigger temperature measurement
 */
void bmp085_ut_trigger(void)
{
    sensors_bmp085_trigger(BMP085_MODE_T);
}

/*
 * \brief Trigger pressure measurement
 */
void bmp085_up_trigger(void)
{
    sensors_bmp085_trigger(BMP085_MODE_P + (osrs << 6));
}

/* EOF */
