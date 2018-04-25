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

#ifndef BMP085_H_
#define BMP085_H_

/* === includes ============================================================ */
#include <stdint.h>

/* === macros ============================================================== */

/* Register File */
#define RG_BMP085_PROM_START (0xAA)
#define RG_BMP085_CHIP_ID_REG (0xD0)
#define RG_BMP085_VERSION_REG (0xD1)
#define RG_BMP085_CTRL_MEAS_REG (0xF4)
#define RG_BMP085_ADC_OUT_MSB_REG (0xF6)
#define RG_BMP085_ADC_OUT_LSB_REG (0xF7)
#define RG_BMP085_SOFT_RESET_REG (0xE0)

/* additional constants */
#define BMP085_PROM_DATA_LEN (22)
#define BMP085_MODE_T (0x2E)	/* temperature measurent */
#define BMP085_MODE_P (0x34)	/* pressure measurement */

#define BMP085_DEFAULT_OSRS (3) /* default setting */

#define BMP085_MAX_CONVERSION_TIME_US_TEMPERATURE (4500)
#define BMP085_MAX_CONVERSION_TIME_US_OSS0 (4500)
#define BMP085_MAX_CONVERSION_TIME_US_OSS1 (7500)
#define BMP085_MAX_CONVERSION_TIME_US_OSS2 (13500)
#define BMP085_MAX_CONVERSION_TIME_US_OSS3 (25500)

/* === types =============================================================== */

typedef struct
{
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
} bmp085_calibration_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void bmp085_init(uint8_t devaddr);
    void bmp085_configure(uint8_t posrs);
    uint16_t bmp085_readresult(void);
    void bmp085_readee(bmp085_calibration_t *cal);
    void bmp085_ut_trigger(void);
    void bmp085_up_trigger(void);
    uint16_t bmp085_ut_read(void);
    uint32_t bmp085_up_read(void);

    float bmp085_scale_up(uint32_t up, uint8_t oss, bmp085_calibration_t *cal);
    float bmp085_scale_ut(uint16_t ut, bmp085_calibration_t *cal);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef BMP085_H_ */
