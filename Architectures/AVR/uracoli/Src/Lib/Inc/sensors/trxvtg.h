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

#ifndef TRXVTG_H
#define TRXVTG_H
/**
 * @defgroup grpSensorMCUVTG MCU Operating Voltage Sensor
 * @ingroup grpSensorDefs
 * @{
 *
 * Measuring operating voltage with the transceiver internal BATMON block.
 *
 */

/* === includes ============================================================ */
#include "sensor_defs.h"
#include "transceiver.h"


/* === macros ============================================================== */
#define TRXVTG_DEBUG (0)
#if TRXVTG_DEBUG == 1
# include "hif.h"
# define DBG_TRXVTG(...) PRINTF(__VA_ARGS__)
#else
# define DBG_TRXVTG(...)
#endif

/* === types =============================================================== */
typedef struct
{
    sensor_driver_t g;
} trxvtg_ctx_t;

/* === prototypes ========================================================== */
static inline uint8_t trxvtg_get_raw_internal(void);

#ifdef __cplusplus
extern "C" {
#endif
/* === low level functions ================================================== */

/* === high level sensor functions ========================================= */

static inline void trxvtg_trigger(void *pctx, bool one_shot)
{
    /* cannot trigger since BATMON is always running */
}

static inline uint8_t trxvtg_get_val(void *pctx, uint8_t *pdata)
{
    sensor_voltage_t *p = (sensor_voltage_t*)pdata;
    if (p)
    {
        uint8_t bm = trxvtg_get_raw_internal();
        p->type = SENSOR_DATA_VOLTAGE;
        p->sensor = SENSOR_TRXVTG;
        if (bm > 15)
        {
            p->voltage = 2.555 + 0.075 * (bm & 0xf);
        }
        else
        {
            p->voltage = 1.7 + 0.05 * (bm & 0xf);
        }
    }
    return sizeof(sensor_voltage_t);
}

static inline uint8_t trxvtg_get_raw(void *pctx, uint8_t *pdata)
{
    uint8_t rv;
    rv = sizeof(sensor_raw_t) - 1 + sizeof(uint8_t); /* +1 for Sensor type */
    if (pdata != NULL)
    {
        ((sensor_raw_t*)pdata)->type = SENSOR_DATA_RAW | sizeof(uint8_t);
        ((sensor_raw_t*)pdata)->sensor = SENSOR_TRXVTG;
        ((sensor_raw_t*)pdata)->data[0] = trxvtg_get_raw_internal();
    }
    return rv;
}

static inline uint8_t trxvtg_get_raw_internal(void)
{
    uint8_t bmok, bm;
    /* this we could do more clever with a tree search */
    for(bm = 0; bm < 32; bm++)
    {
        if (bm == 0)
        {
            trx_bit_write(SR_BATMON_HR, 0);
        }
        if (bm == 16)
        {
            trx_bit_write(SR_BATMON_HR, 1);
        }
        trx_bit_write(SR_BATMON_VTH, bm & 0xf);
        bmok = trx_bit_read(SR_BATMON_OK);
        DBG_TRXVTG("bm=%x bmok=%d\n\r", bm, bmok);
        if (bmok == 0)
        {
            break;
        }
    }
    return bm;
}

static inline void trxvtg_sleep(void *pctx)
{
    /* empty */
}

/**
 * Create an instance of a trxvtg sensor and initialize the sensor.
 *
 * @return sizeof(trxvtg_ctx_t)
 *
 */
static inline uint8_t sensor_create_trxvtg(void *pdata)
{
    uint8_t rv = sizeof(trxvtg_ctx_t);
    trxvtg_ctx_t *pcfg;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (trxvtg_ctx_t *)pdata;
        pcfg->g.id = SENSOR_TRXVTG;
        pcfg->g.f_trigger = trxvtg_trigger;
        pcfg->g.f_get_val = trxvtg_get_val;
        pcfg->g.f_sleep = NULL;

        /* init private sensor data */

        /* initialize sensor hardware */

    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef TRXVTG_H */
