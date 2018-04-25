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

#ifndef ITG3200_H
#define ITG3200_H

/* === includes ============================================================ */
#include "sensors.h"

/* === macros ============================================================== */

/* Register Map */
#define RG_ITG3200_WHO_AM_I    (0x00)
#define RG_ITG3200_SMPLRT_DIV  (0x15)
#define RG_ITG3200_DLPF_FS     (0x16)
#define RG_ITG3200_INT_CFG     (0x17)
#define RG_ITG3200_INT_STATUS  (0x1A)
#define RG_ITG3200_TEMP_OUT_H  (0x1B)
#define RG_ITG3200_TEMP_OUT_L  (0x1C)
#define RG_ITG3200_GYRO_XOUT_H (0x1D)
#define RG_ITG3200_GYRO_XOUT_L (0x1E)
#define RG_ITG3200_GYRO_YOUT_H (0x1F)
#define RG_ITG3200_GYRO_YOUT_L (0x20)
#define RG_ITG3200_GYRO_ZOUT_H (0x21)
#define RG_ITG3200_GYRO_ZOUT_L (0x22)
#define RG_ITG3200_PWR_MGM     (0x3E)


/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void itg3200_init(uint8_t addr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef ITG3200_H */
