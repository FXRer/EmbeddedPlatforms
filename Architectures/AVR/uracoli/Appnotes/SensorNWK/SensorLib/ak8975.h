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

#ifndef AK8975_H
#define AK8975_H

/* === includes ============================================================ */
#include "sensors.h"

/* === macros ============================================================== */

/* Register Map */
#define RG_AK8975_WIA  (0x00)
#define RG_AK8975_INFO (0x01)
#define RG_AK8975_ST1  (0x02)
#define RG_AK8975_HXL  (0x03)
#define RG_AK8975_HXH  (0x04)
#define RG_AK8975_HYL  (0x05)
#define RG_AK8975_HYH  (0x06)
#define RG_AK8975_HZL  (0x07)
#define RG_AK8975_HZH  (0x08)
#define RG_AK8975_ST2  (0x09)
#define RG_AK8975_CNTL (0x0A)
#define RG_AK8975_RSV  (0x0B)
#define RG_AK8975_ASTC (0x0C)
#define RG_AK8975_TS1  (0x0D)
#define RG_AK8975_TS2  (0x0E)
#define RG_AK8975_I2CDIS (0x0F)
#define RG_AK8975_ASAX (0x10) /* Fuse ROM */
#define RG_AK8975_ASAY (0x11) /* Fuse ROM */
#define RG_AK8975_ASAZ (0x11) /* Fuse ROM */

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void ak8975_init(uint8_t addr);
    void ak8975_trigger(void);
    void ak8975_readresult(sensors_xyz_result_t *result);

    float sensors_ak8975_scale(int16_t raw, int16_t calpos, int16_t calneg, uint8_t gain, char axis);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef AK8975_H */
