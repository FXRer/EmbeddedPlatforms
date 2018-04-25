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

#ifndef SENSORS_H
#define SENSORS_H

/* === includes ============================================================ */
#include <stdint.h>
#include "i2c.h"

/* === macros ============================================================== */

/* I2C addresses (already shifted left 1 to left-align to MSB) */
#define SENSORS_I2CADDR_BMA250  (0x30)
#define SENSORS_I2CADDR_LIS3331 (0x32)
#define SENSORS_I2CADDR_HMC5883 (0x3C)
#define SENSORS_I2CADDR_BMP085  (0xEE)
#define SENSORS_I2CADDR_MMA7455 (0x3A)
#define SENSORS_I2CADDR_ADXL345 (0xA6) /* ALT_ADDRESS (pin 12) = Low (floating) */
#define SENSORS_I2CADDR_SHT21   (0x80)
#define SENSORS_I2CADDR_AK8975  (0x18) /* CAD0/1 pulled low */

/* === types =============================================================== */

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} sensors_xyz_result_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void sensors_regwr(uint8_t devaddr, uint8_t regaddr, uint8_t val);
    uint8_t sensors_regrd(uint8_t devaddr, uint8_t regaddr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SENSORS_H */
