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

#ifndef ADXL345_H_
#define ADXL345_H_

/* === includes ============================================================ */
#include "sensors.h"

/* === macros ============================================================== */

#define RG_ADXL345_DEVID (0x00) /* Device ID */
#define RG_ADXL345_THRESH_TAP (0x1D) /* Tap threshold */
#define RG_ADXL345_OFSX (0x1E) /* X-axis offset */
#define RG_ADXL345_OFSY (0x1F) /* Y-axis offset */
#define RG_ADXL345_OFSZ (0x20) /* Z-axis offset */
#define RG_ADXL345_DUR (0x21) /* Tap duration */
#define RG_ADXL345_LATENT (0x22) /* Tap latency */
#define RG_ADXL345_WINDOW (0x23) /* Tap window */
#define RG_ADXL345_THRESH_ACT (0x24) /* Activity threshold */
#define RG_ADXL345_THRESH_INACT (0x25) /* Inactivity threshold */
#define RG_ADXL345_TIME_INACT (0x26) /* Inactivity time */
#define RG_ADXL345_ACT_INACT_CTL (0x27) /* Axis enable control for activity and inactivity detection */
#define RG_ADXL345_THRESH_FF (0x28) /* Free-fall threshold */
#define RG_ADXL345_TIME_FF (0x29) /* Free-fall time */
#define RG_ADXL345_TAP_AXES (0x2A) /* Axis control for single tap/double tap */
#define RG_ADXL345_ACT_TAP_STATUS (0x2B) /* Source of single tap/double tap */
#define RG_ADXL345_BW_RATE (0x2C) /* Data rate and power mode control */
#define RG_ADXL345_POWER_CTL (0x2D) /* Power-saving features control */
#define RG_ADXL345_INT_ENABLE (0x2E) /* Interrupt enable control */
#define RG_ADXL345_INT_MAP (0x2F) /* Interrupt mapping control */
#define RG_ADXL345_INT_SOURCE (0x30) /* Source of interrupts */
#define RG_ADXL345_DATA_FORMAT (0x31) /* Data format control */
#define RG_ADXL345_DATAX0 (0x32) /* X-Axis Data 0 */
#define RG_ADXL345_DATAX1 (0x33) /* X-Axis Data 1 */
#define RG_ADXL345_DATAY0 (0x34) /* Y-Axis Data 0 */
#define RG_ADXL345_DATAY1 (0x35) /* Y-Axis Data 1 */
#define RG_ADXL345_DATAZ0 (0x36) /* Z-Axis Data 0 */
#define RG_ADXL345_DATAZ1 (0x37) /* Z-Axis Data 1 */
#define RG_ADXL345_FIFO_CTL (0x38) /* FIFO control */
#define RG_ADXL345_FIFO_STATUS (0x39) /* FIFO status */

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void adxl345_init(uint8_t addr);
    void adxl345_trigger(void);
    void adxl345_powerdown(void);
    void adxl345_readresult(sensors_xyz_result_t *result);

    static inline float adxl345_rate_code2hz(uint8_t code)
    {
        return 3200 / (16-code);
    }

    float adxl345_scale(int16_t raw, float sensitivity);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ADXL345_H_ */
