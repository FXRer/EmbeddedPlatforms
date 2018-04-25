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

#ifndef MMA7455_H_
#define MMA7455_H_

/* === includes ============================================================ */
#include "sensors.h"

/* === macros ============================================================== */

/** Subregister MOD[1:0], STANDBY */
#define MMA7455_MODE_STANDBY        (0x00)
/** Subregister MOD[1:0], MEASURE */
#define MMA7455_MODE_MEASURE        (0x01)
/** Subregister MOD[1:0], LEVELDETECT */
#define MMA7455_MODE_LEVELDETECT    (0x02)
/** Subregister MOD[1:0], PULSEDETECT */
#define MMA7455_MODE_PULSEDETECT    (0x03)

/** Subregister GLVL[1:0], g-Selector: 2g */
#define MMA7455_RANGE_2G            (0x01)
/** Subregister GLVL[1:0], g-Selector: 4g */
#define MMA7455_RANGE_4G            (0x02)
/** Subregister GLVL[1:0], g-Selector: 8g */
#define MMA7455_RANGE_8G            (0x00)

/* Register map */
#define RG_MMA7455_XOUTL (0x0)
#define RG_MMA7455_XOUTH (0x1)
#define RG_MMA7455_YOUTL (0x2)
#define RG_MMA7455_YOUTH (0x3)
#define RG_MMA7455_ZOUTL (0x4)
#define RG_MMA7455_ZOUTH (0x5)
#define RG_MMA7455_XOUT8 (0x6)
#define RG_MMA7455_YOUT8 (0x7)
#define RG_MMA7455_ZOUT8 (0x8)
#define RG_MMA7455_STATUS (0x9)
#define RG_MMA7455_DETSRC (0xa)
#define RG_MMA7455_TOUT (0xb)
#define RG_MMA7455_I2CAD (0xd)
#define RG_MMA7455_USRINF (0xe)
#define RG_MMA7455_WHOAMI (0xf)
#define RG_MMA7455_XOFFL (0x10)
#define RG_MMA7455_XOFFH (0x11)
#define RG_MMA7455_YOFFL (0x12)
#define RG_MMA7455_YOFFH (0x13)
#define RG_MMA7455_ZOFFL (0x14)
#define RG_MMA7455_ZOFFH (0x15)
#define RG_MMA7455_MCTL (0x16)
#define RG_MMA7455_INTRST (0x17)
#define RG_MMA7455_CTL1 (0x18)
#define RG_MMA7455_CTL2 (0x19)
#define RG_MMA7455_LDTH (0x1a)
#define RG_MMA7455_PDTH (0x1b)
#define RG_MMA7455_PW (0x1c)
#define RG_MMA7455_LT (0x1d)
#define RG_MMA7455_TW (0x1e)


/*
 * could be dynamic in the future
 */
#define ACC_SENSITIVITY (MMA7455_RANGE_2G)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void mma7455_init(uint8_t addr);
    void mma7455_trigger(void);
    void mma7455_powerdown(void);
    void mma7455_readresult(sensors_xyz_result_t *result);

    float mma7455_scale(int16_t raw, uint8_t sensitivity);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef MMA7455_H_ */
