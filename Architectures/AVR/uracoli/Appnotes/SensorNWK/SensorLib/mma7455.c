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
#include "mma7455.h"

static uint8_t i2caddr = 0;

/*
 * \brief Initialize
 */
void mma7455_init(uint8_t addr)
{
    i2caddr = addr;
}

/*
 * \brief Trigger
 *
 */
void mma7455_trigger(void)
{
    sensors_regwr(SENSORS_I2CADDR_MMA7455, 0x16,
                  ((ACC_SENSITIVITY << 2) & 0x0C) | 0x01);
}

/*
 * \brief Power down
 *
 */
void mma7455_powerdown(void)
{
    sensors_regwr(SENSORS_I2CADDR_MMA7455, 0x16, 0x00); /* standby mode */
}

/*
 * \brief Read result
 *
 */
void mma7455_readresult(sensors_xyz_result_t *result)
{
    uint8_t buf[6];

    i2c_master_writeread(SENSORS_I2CADDR_MMA7455, (void*)0x0000, 0, buf, 6);

    result->x = (int16_t) ((buf[1] << 14) | (buf[0] << 6)) / 64;
    result->y = (int16_t) ((buf[3] << 14) | (buf[2] << 6)) / 64;
    result->z = (int16_t) ((buf[5] << 14) | (buf[4] << 6)) / 64;
}

/* EOF */
