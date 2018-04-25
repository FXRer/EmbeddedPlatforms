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

#ifndef HMC5883L_H
#define HMC5883L_H

/* === includes ============================================================ */
#include "sensors.h"

/* === macros ============================================================== */

/* Register Map */
#define RG_HMC5883L_CONFIG_A (0x00)
#define RG_HMC5883L_CONFIG_B (0x01)
#define RG_HMC5883L_MODE (0x02)
#define RG_HMC5883L_DATA_X (0x03)
#define RG_HMC5883L_DATA_Y (0x05)
#define RG_HMC5883L_DATA_Z (0x07)
#define RG_HMC5883L_STATUS (0x09)
#define RG_HMC5883L_IDENT_A (0x0A)
#define RG_HMC5883L_IDENT_B (0x0B)
#define RG_HMC5883L_IDENT_C (0x0C)

#define HMC5883L_GAINCAL (3) /* Gain setting for calibration */

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void hmc5883l_init(uint8_t addr);
    void hmc5883l_identification(uint8_t *buf);
    void hmc5883l_trigger(void);
    void hmc5883l_readresult(sensors_xyz_result_t *result);

    float sensors_hmc5883l_scale(int16_t raw, int16_t calpos, int16_t calneg, uint8_t gain, char axis);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef HMC5883L_H */
