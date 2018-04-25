/* Copyright (c) 2013 - 2014 Axel Wachtler
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
/**
 * @defgroup grpSensorHMC5883l HMC5883L - 3-Axis Digital Compass IC
 * @ingroup grpSensorDefs
 * @{
 *
 * The Honeywell HMC5883L is a surface-mount, multi-chip module designed
 * for low-field magnetic sensing.
 *
 * Datasheet: <a href="http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/HMC5883L_3-Axis_Digital_Compass_IC.pdf">HMC5883L_3-Axis_Digital_Compass_IC.pdf</a>
 */
/* === includes ============================================================ */
#include "sensor_defs.h"

/* === macros ============================================================== */

#define HMC5883L_ADDR (0x1E)

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

#define HMC5883L_GAIN (1090)     /* LSB/Gauss, default after reset */


/* === types =============================================================== */
typedef struct
{
    sensor_driver_t g;
    uint8_t addr; /* I2C address */
} hmc5883l_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/* === low level functions ================================================== */

/* === high level sensor functions ========================================= */

static inline void hmc5883l_trigger(void *pctx, bool one_shot)
{
    uint8_t buf[2] = {RG_HMC5883L_MODE, 0x01}; /* Mode 1: single measurement mode */
    i2c_master_writeread(((hmc5883l_ctx_t*)pctx)->addr, buf, sizeof(buf)/sizeof(buf[0]), NULL, 0);
}

static inline uint8_t hmc5883l_get_val(void *pctx, uint8_t *pdata)
{
    const uint8_t nbbytes = 6;
    uint8_t reg = {RG_HMC5883L_DATA_X};
    uint8_t buf[nbbytes];
    sensor_magnetic_t *p = (sensor_magnetic_t*)pdata;

    i2c_master_writeread(((hmc5883l_ctx_t*)pctx)->addr, &reg, 1, buf, nbbytes);

    p->type = SENSOR_DATA_MAGNETIC;
    p->sensor = SENSOR_HMC5883L;
    p->x = (float)((buf[0] << 8) | buf[1]) / HMC5883L_GAIN;
    p->y = (float)((buf[2] << 8) | buf[3]) / HMC5883L_GAIN;
    p->z = (float)((buf[4] << 8) | buf[5]) / HMC5883L_GAIN;

    return sizeof(sensor_magnetic_t);
}

static inline uint8_t hmc5883l_get_raw(void *pctx, uint8_t *pdata)
{
    /* not implemented */
    return 0;
}

static inline void hmc5883l_sleep(void *pctx)
{
    /* not implemented
     *
     * Sensor goes to sleep when measurement is finished (started in single-measurement mode)
     * Should another mode be implemented in the future, sleep must be implemented explicitely
     */
}

/**
 * Create an instance of a hmc5883l sensor and initialize the sensor.
 *
 * @return sizeof(hmc5883l_ctx_t)
 *
 */
static inline uint8_t sensor_create_hmc5883l(void *pdata, bool raw, uint8_t addr)
{
    uint8_t rv = sizeof(hmc5883l_ctx_t);
    hmc5883l_ctx_t *pcfg;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (hmc5883l_ctx_t *)pdata;
        pcfg->g.id = SENSOR_HMC5883L;
        pcfg->g.f_trigger = hmc5883l_trigger;
        pcfg->g.f_get_val = raw ?  hmc5883l_get_raw : hmc5883l_get_val;
        pcfg->g.f_sleep = hmc5883l_sleep;

        pcfg->addr = addr;

        /* TODO: exec calibration and store in calibration structure */
    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef HMC5883L_H */
