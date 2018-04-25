/* Copyright (c) 2016 Axel Wachtler
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

#ifndef DHT22_H
#define DHT22_H

/* === includes ============================================================ */
#include "sensor_defs.h"

/* === macros ============================================================== */

/* === types =============================================================== */
typedef struct
{
    sensor_driver_t g;
} dht22_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
static inline uint8_t dht22_get_val(void *pctx, uint8_t *pdata)
{
    return 0;
}

static inline uint8_t dht22_get_raw(void *pctx, uint8_t *pdata)
{
    return 0;
}


/**
 * Create an instance of a DHT22 sensor and initialize the sensor.
 *
 * @return sizeof(dht22_ctx_t)
 *
 */
static inline uint8_t sensor_create_dht22(void *pdata, bool raw)
{
    uint8_t rv = sizeof(dht22_ctx_t);
    dht22_ctx_t *pcfg;
    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (dht22_ctx_t *)pdata;
        pcfg->g.id = SENSOR_DHT22;
        pcfg->g.f_trigger = NULL;
        pcfg->g.f_get_val = raw ?  dht22_get_raw : dht22_get_val;
        pcfg->g.f_sleep = NULL;
        // ... more functions to setup

        /* init private sensor data */
        //memcpy(pcfg->addr, tmp_addr, sizeof(tmp_addr));
        /* TODO: scan bus for DS18B20 devices and store addresses sorted */

        /* initialize sensor hardware */
    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef XXX_H */
