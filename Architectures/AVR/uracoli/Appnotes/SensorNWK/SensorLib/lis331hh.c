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


/* === includes ============================================================ */
#include <stdint.h>

#include "i2c.h"
#include "sensors.h"
#include "lis331hh.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

static uint8_t i2caddr = 0;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

void lis331_init(uint8_t addr)
{
    i2caddr = addr;
}

/*
 * \brief
 *
 */
void lis331_trigger(void)
{
    /* Mode: 400 Hz, all axes enabled */
    sensors_regwr(i2caddr, RG_LIS331HH_CTRL_REG1, 0x2F);
}

/*
 * \brief
 *
 */
void lis331_powerdown(void)
{
    sensors_regwr(i2caddr, RG_LIS331HH_CTRL_REG1, 0x00);
}

/*
 * \brief
 *
 */
void lis331hh_readresult(sensors_xyz_result_t *result)
{
    uint8_t wbuf[1] = {RG_LIS331HH_OUT_X_L}; /* start address of data */
    uint8_t rbuf[6];

    /* Block read of LIS331 requires MSB to be set to 1 */
    i2c_master_writeread(i2caddr | 0x80, wbuf, 1, rbuf, 6);

    /* 3mg per digit/fullscale */
    result->x = (int16_t)((rbuf[1] << 8) | rbuf[0]);
    result->y = (int16_t)((rbuf[3] << 8) | rbuf[2]);
    result->z = (int16_t)((rbuf[5] << 8) | rbuf[4]);
}

/* EOF */
