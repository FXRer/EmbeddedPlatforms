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

#ifndef BMA250_H_
#define BMA250_H_

/* === includes ============================================================ */
#include <stdint.h>
#include <sensors.h>

/* === macros ============================================================== */

/* Register File */
#define RG_BMA250_ACC_X_LSB      (0x02)
#define RG_BMA250_ACC_X_MSB      (0x03)
#define RG_BMA250_ACC_Y_LSB      (0x04)
#define RG_BMA250_ACC_Y_MSB      (0x05)
#define RG_BMA250_ACC_Z_LSB      (0x06)
#define RG_BMA250_ACC_Z_MSB      (0x07)
#define RG_BMA250_TEMP           (0x08)
#define RG_BMA250_SELFTEST       (0x32)
#define RG_BMA250_EEPROM_CONTROL (0x33)
#define RG_BMA250_POWERMODES	 (0x11)

#define RG_BMA250_RANGE (0x0F)
#define RG_BMA250_BW (0x10)

/* Bits sleep_dur in Register 0x11 */
#define BMA250_SLEEP_DUR_0MS5 (0x0)
#define BMA250_SLEEP_DUR_1MS  (0x06)
#define BMA250_SLEEP_DUR_2MS  (0x07)
#define BMA250_SLEEP_DUR_4MS  (0x08)
#define BMA250_SLEEP_DUR_6MS  (0x09)
#define BMA250_SLEEP_DUR_10MS  (0x0A)
#define BMA250_SLEEP_DUR_25MS  (0x0B)
#define BMA250_SLEEP_DUR_50MS  (0x0C)
#define BMA250_SLEEP_DUR_100MS  (0x0D)
#define BMA250_SLEEP_DUR_500MS  (0x0E)
#define BMA250_SLEEP_DUR_1000MS  (0x0F)

/* Bits bw in Register 0x10 */
#define BMA250_BW_8HZ (0x08)
#define BMA250_BW_16HZ (0x09)
#define BMA250_BW_32HZ (0x0A)
#define BMA250_BW_63HZ (0x0B)
#define BMA250_BW_125HZ (0x0C)
#define BMA250_BW_250HZ (0x0D)
#define BMA250_BW_500HZ (0x0E)
#define BMA250_BW_1000HZ (0x0F)

/* Bits range in Register 0x0F */
#define BMA250_RANGE_2G (0x03)
#define BMA250_RANGE_4G (0x05)
#define BMA250_RANGE_8G (0x08)
#define BMA250_RANGE_16G (0x0C)

/* === types =============================================================== */


/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void bma250_init(uint8_t devaddr);
    void bma250_trigger(void);
    void bma250_powerdown(void);
    void bma250_readresult(sensors_xyz_result_t *result);
    uint8_t bma250_read_temperature(void);
    void bma250_eeprom_write(uint8_t *data, uint8_t nbbytes);

    float bma250_scale_acceleration(int16_t raw, float sensitivity);
    float bma250_scale_temperature(int8_t raw);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef BMA250_H_ */
