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

/**
 * @file
 * @brief ....
 * @_addtogroup grpApp...
 */

#include "i2c.h"
#include "sensors.h"
#include "adxl345.h"

static uint8_t i2caddr = 0;

/*
 * \brief Initialize
 *
 * @param addr I2C address to use
 */
void adxl345_init(uint8_t addr)
{
    i2caddr = addr;
}

/*
 * \brief Trigger
 *
 */
void adxl345_trigger(void)
{
    sensors_regwr(i2caddr, RG_ADXL345_POWER_CTL, 0x08); /* measure mode */
}

/*
 * \brief Power down
 *
 */
void adxl345_powerdown(void)
{
    sensors_regwr(i2caddr, RG_ADXL345_POWER_CTL, 0x00); /* standby mode */
}

/*
 * \brief Read result
 *
 */
void adxl345_readresult(sensors_xyz_result_t *result)
{
    uint8_t wbuf[1] = {RG_ADXL345_DATAX0};
    uint8_t rbuf[6];

    sensors_regrd(i2caddr, RG_ADXL345_INT_SOURCE);

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, 6);

    result->x = (int16_t) ((rbuf[1] << 14) | (rbuf[0] << 6)) / 64;
    result->y = (int16_t) ((rbuf[3] << 14) | (rbuf[2] << 6)) / 64;
    result->z = (int16_t) ((rbuf[5] << 14) | (rbuf[4] << 6)) / 64;
}

/* EOF */
