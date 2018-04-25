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
 * @file
 * @brief ....
 * @_addtogroup grpApp...
 */

#include <stdint.h>

/* project inclusions */
#include "i2c.h"
#include "sensors.h"
#include "bma250.h"

static uint8_t i2caddr = 0;

void bma250_init(uint8_t devaddr)
{
    i2caddr = devaddr;
}

void bma250_trigger(void)
{

}

void bma250_powerdown(void)
{
	/* enter suspend mode */
	sensors_regwr(i2caddr, RG_BMA250_POWERMODES, 0x80);
}

void bma250_readresult(sensors_xyz_result_t *result)
{
    uint8_t wbuf[1] = {RG_BMA250_ACC_X_LSB};
    uint8_t rbuf[6];

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, 6);

    result->x = (int16_t) ((rbuf[1] << 8) | (rbuf[0] >> 6));
    result->y = (int16_t) ((rbuf[3] << 8) | (rbuf[2] >> 6));
    result->z = (int16_t) ((rbuf[5] << 8) | (rbuf[4] >> 6));
}

/*
 * \brief Read raw temperature value
 */
uint8_t bma250_read_temperature(void)
{
    return sensors_regrd(i2caddr, RG_BMA250_TEMP);
}

void bma250_eeprom_write(uint8_t *data, uint8_t nbbytes)
{
    if(8 >= nbbytes)  /* hard coded: 8 bytes for EEPROM */
    {

        /* Step1: write to image registers */
        i2c_master_writeread(i2caddr, data, nbbytes, (void*)0x0000, 0);

        /* if someone already writing */
        while(0 == (sensors_regrd(i2caddr, RG_BMA250_EEPROM_CONTROL) & (1<<2)))
        {
            /* wait for nvm_rdy == 1 */
        }

        /* Step2: unlock */
        sensors_regwr(i2caddr, RG_BMA250_EEPROM_CONTROL, 0x01);

        /* Step3: initiate write */
        sensors_regwr(i2caddr, RG_BMA250_EEPROM_CONTROL, 0x03);

        while(0 == (sensors_regrd(i2caddr, RG_BMA250_EEPROM_CONTROL) & (1<<2)))
        {
            /* wait for nvm_rdy == 1 */
        }
    }
    else
    {

    }
}

/* EOF */
