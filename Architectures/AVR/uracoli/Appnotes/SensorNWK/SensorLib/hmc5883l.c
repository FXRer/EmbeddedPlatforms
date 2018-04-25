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
#include "hmc5883l.h"
#include "sensors.h" /* for the sensors_xyz_t structure */

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
static uint8_t i2caddr = 0;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

void hmc5883l_init(uint8_t addr)
{
    i2caddr = addr;
}


/*
 * \brief Read HMC5883L identification registers
 *
 * @param buf Buffer to keep identification code, must be size of 3
 */
void hmc5883l_identification(uint8_t *buf)
{
    uint8_t wbuf[1] = {RG_HMC5883L_IDENT_A}; /* start address of data */
    i2c_master_writeread(i2caddr, wbuf, 1, buf, 3);
}

/*
 * \brief Trigger measurement
 */
void hmc5883l_trigger(void)
{
    uint8_t wbuf[2] = {RG_HMC5883L_MODE, 0x01}; /* single measurement mode */
    i2c_master_writeread(i2caddr, wbuf, 2, (void*)0x0000, 0);
}

/*
 * \brief Read measurement that was triggered before
 *
 * @param *result Pointer to structure containing 3 unsigned ints for xyz axis
 * @return None
 */
void hmc5883l_readresult(sensors_xyz_result_t *result)
{
    uint8_t wbuf[1] = {RG_HMC5883L_DATA_X}; /* start address of data */
    uint8_t rbuf[6];

    i2c_master_writeread(i2caddr, wbuf, 1, rbuf, 6);

    /* order of data registers: X, Z, Y */
    result->x = (rbuf[0] << 8) | rbuf[1];
    result->z = (rbuf[2] << 8) | rbuf[3];
    result->y = (rbuf[4] << 8) | rbuf[5];
}

/* EOF */
