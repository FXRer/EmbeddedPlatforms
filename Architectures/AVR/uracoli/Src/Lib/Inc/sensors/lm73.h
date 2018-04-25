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

#ifndef LM73_H
#define LM73_H
/**
 * @defgroup grpSensorLM73 LM73 - Temperature Sensor
 * @ingroup grpSensorDefs
 * @{
 *
 * ±1°C Temperature Sensor with I2C/SMBus Interface
 *
 * - Product: http://www.ti.com/product/lm73
 * - Datasheet: http://www.ti.com/lit/ds/symlink/lm73.pdf
 *
 */

/* === includes ============================================================ */
#include "sensor_defs.h"

/* === macros ============================================================== */
#define LM73_DEBUG (0)
#if LM73_DEBUG == 1
# include "hif.h"
# define DBG_LM73(...) PRINTF(__VA_ARGS__)
#else
# define DBG_LM73(...)
#endif

#define LM73_ADDR_0 (0x48) /**< LM3-0, ADDR pin is floating */
#define LM73_ADDR_1 (0x49) /**< LM3-0, ADDR pin is pulled to GND */
#define LM73_ADDR_2 (0x4A) /**< LM3-0, ADDR pin is pulled to VDD */

#define LM73_ADDR_3 (0x4C) /**< LM3-1, ADDR pin is floating */
#define LM73_ADDR_4 (0x4D) /**< LM3-1, ADDR pin is pulled to GND */
#define LM73_ADDR_5 (0x4E) /**< LM3-1, ADDR pin is pulled to VDD */

#define LM73_PTR_TEMP   (0x00)
#define LM73_PTR_CFG    (0x01)
#define LM73_PTR_THIGH  (0x02)
#define LM73_PTR_TLOW   (0x03)
#define LM73_PTR_STATUS (0x04)
#define LM73_PTR_ID     (0x07)

#define LM73_CFG_RESERVED  (0x03)
#define LM73_CFG_ONE_SHOT  (0x04)
#define LM73_CFG_ALERT_RST (0x08)
#define LM73_CFG_ALERT_POL (0x10)
#define LM73_CFG_ALERT_EN  (0x20)
#define LM73_CFG_PWR_DOWN  (0x80)

#define LM73_STATUS_DAV   (0x01)
#define LM73_STATUS_TLOW  (0x02)
#define LM73_STATUS_THIGH (0x04)
#define LM73_STATUS_ALERT (0x08)
#define LM73_STATUS_RES   (0x60)

/**
 * This value is the time between a stop-start cycle of the LM73.
 * It is speced as tBuf = 1.2us min, but on mesh bean it was found
 * that 8us ensure stable behaviour.
 */
#define LM73_T_BUS_FREE_US (8)

// LM73_TEMP_SCALE = 1/128
#define LM73_REG_TO_TEMP_SCALE (0.0078125)
#define LM73_TEMP_TO_REG_SCALE (128)
/* === types =============================================================== */
typedef struct
{
    sensor_driver_t g;
    uint8_t addr;
    int8_t temp_offset;
} lm73_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/* === low level functions ================================================== */

static inline uint8_t lm73_byte_read(lm73_ctx_t *pctx, uint8_t addr)
{
    uint8_t buf[1] = {addr,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 1);
    DBG_LM73("d: lm73_byte_read: addr=0x%x val=0x%x\n", addr, buf[0]);
    return buf[0];
}

static inline void lm73_byte_write(lm73_ctx_t *pctx, uint8_t addr, uint8_t val)
{
    uint8_t buf[2] = {addr, val};
    i2c_master_writeread(pctx->addr, buf, 2, NULL, 0);
    DBG_LM73("d: lm73_byte_write: addr=0x%x val=0x%x\n", addr, val);
}

static inline uint16_t lm73_word_read(lm73_ctx_t *pctx, uint8_t addr)
{
    uint16_t rv;
    uint8_t buf[2] = {addr,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 2);
    rv = (buf[0]<<8) | buf[1];
    DBG_LM73("d: lm73_word_read: addr=0x%x val=0x%x\n", addr, rv);
    return rv;
}

static inline void lm73_word_write(lm73_ctx_t *pctx, uint8_t addr, uint16_t val)
{
    uint8_t buf[3] = {addr, ((val>>8) &0xff), (val & 0xff)};
    i2c_master_writeread(pctx->addr, buf, 3, NULL, 0);
    DBG_LM73("d: lm73_word_write: addr=0x%x val=0x%x\n", addr, val);
}

static inline int16_t lm73_get(lm73_ctx_t *pctx)
{
    int16_t rv;
    rv = lm73_word_read(pctx, LM73_PTR_TEMP);
    DBG_LM73("d: lm73_get: val=0x%x\n", rv);
    return rv;
}

static inline void lm73_set_upper_limit(lm73_ctx_t *pctx, int16_t val)
{
    lm73_word_write(pctx, LM73_PTR_THIGH, val);
}

static inline void lm73_set_lower_limit(lm73_ctx_t *pctx, int16_t val)
{
    lm73_word_write(pctx, LM73_PTR_TLOW, val);
}

static inline uint16_t lm73_get_id(lm73_ctx_t *pctx)
{
    uint8_t buf[2] = {7};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 2);
    return buf[0] | buf[1]*256;
}

static inline float lm73_scale(int16_t val)
{
    return  LM73_REG_TO_TEMP_SCALE * (val);
}


/* === high level sensor functions ========================================= */

static inline void lm73_trigger(void *pctx, bool one_shot)
{
    uint8_t tmp;
    tmp = lm73_byte_read(((lm73_ctx_t *)pctx), LM73_PTR_CFG);
    if (one_shot)
    {
        tmp |= (LM73_CFG_PWR_DOWN | LM73_CFG_ONE_SHOT);
    }
    else
    {
        tmp &= ~LM73_CFG_PWR_DOWN;
    }
    tmp &= ~LM73_CFG_RESERVED;
    tmp |= 1<<6;
    DELAY_US(LM73_T_BUS_FREE_US);
    lm73_byte_write(((lm73_ctx_t *)pctx), LM73_PTR_CFG, tmp);
    DBG_LM73("d: lm73_trigger: cfg=0x%x, one_shot=%d\n", tmp, one_shot);
}

static inline uint8_t lm73_get_val(void *pctx, uint8_t *pdata)
{
    uint8_t rv, status;
    uint16_t temp;
    rv = sizeof(sensor_temperature_t);
    if (pdata != NULL)
    {
        temp = lm73_get((lm73_ctx_t *)pctx);
        status = lm73_byte_read(((lm73_ctx_t *)pctx), LM73_PTR_STATUS);
        if (status & LM73_STATUS_DAV)
        {
            ((sensor_temperature_t*)pdata)->type = SENSOR_DATA_TEMPERATURE;
            ((sensor_temperature_t*)pdata)->sensor = SENSOR_LM73;
            ((sensor_temperature_t*)pdata)->temp = lm73_scale(temp);
        }
        else
        {
            rv = 0;
        }
        DBG_LM73("d: lm73_get_val: temp=%d, status=0x%x\n", temp, status);

    }
    return rv;

}
static inline uint8_t lm73_get_raw(void *pctx, uint8_t *pdata)
{
    uint8_t rv, status;
    uint16_t tempi;
    rv = sizeof(sensor_raw_t) - 1 + sizeof(tempi);
    if (pdata != NULL)
    {
        status = lm73_byte_read(((lm73_ctx_t *)pctx), LM73_PTR_STATUS);
        if (status & LM73_STATUS_DAV)
        {
            ((sensor_raw_t*)pdata)->type = SENSOR_DATA_RAW | sizeof(tempi);
            ((sensor_raw_t*)pdata)->sensor = SENSOR_LM73;
            tempi = lm73_get((lm73_ctx_t *)pctx);
            ((sensor_raw_t*)pdata)->data[0] = tempi & 0xff;
            ((sensor_raw_t*)pdata)->data[1] = (tempi >> 8) & 0xff;
        }
        else
        {
            rv = 0;
        }
    }
    return rv;
}

static inline void lm73_sleep(void *pctx)
{
    uint8_t tmp;
    tmp = lm73_byte_read(((lm73_ctx_t *)pctx), LM73_PTR_CFG);
    tmp |= LM73_CFG_PWR_DOWN;
    DELAY_US(LM73_T_BUS_FREE_US);
    lm73_byte_write(((lm73_ctx_t *)pctx), LM73_PTR_CFG, tmp);
    DBG_LM73("d: lm73_sleep: val=0x%x\n", tmp);
}

/**
 * Create an instance of a LM73 sensor and initialize the sensor.
 *
 * If the parameter pdata != NULL, the sensor is also initialized.
 * In case that the initialization fails, the variable drv.last_error is
 * set to SENSOR_ERR_INIT.
 *
 * @param pdata
 *        pointer to memory, where the sensor data is stored.
 * @param raw
 *        if true the get function will return the register values, otherwise
 *        a physical value is returned (temperature in degC a float)
 * @param addr
 *        i2c address of the sensor
 * @param temp_offset
 *        offset of the measurement value.
 *
 * @return sizeof(lm73_ctx_t)
 *
 */
static inline uint8_t sensor_create_lm73(void *pdata,
                                         bool raw,
                                         uint8_t addr,
                                         int8_t temp_offset)
{
    uint8_t rv = sizeof(lm73_ctx_t);
    lm73_ctx_t *pcfg;
    uint8_t tst;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (lm73_ctx_t *)pdata;
        pcfg->g.id = SENSOR_LM73;
        pcfg->g.f_trigger = lm73_trigger;
        pcfg->g.f_get_val = raw ?  lm73_get_raw : lm73_get_val;
        pcfg->g.f_sleep = lm73_sleep;
        // ... more functions to setup

        /* init private sensor data */
        pcfg->addr = addr;
        pcfg->temp_offset = temp_offset;

        /* initialize sensor */
        tst = i2c_probe(pcfg->addr);
        pcfg->g.last_error = tst ? SENSOR_ERR_OK : SENSOR_ERR_INIT;
        DBG_LM73("d: lm73_init: addr=0x%x\n", addr);
    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef LM73_H */
