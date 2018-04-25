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

#if 0
---8<----------8<----------8<----------8<----------8<----------8<----------8<---
How to use this template:

1) copy this file and name it like your sensor:
   cp Templates/mysensor.h Src/Lib/Inc/sensors/lmaa0815.h

2) Rename in lmaa0815.h all occurences of "mysensor" with "lmaa0815"
   and "MYSENSOR" with "LMAA0815"

3) add "lmaa0815" sensor to a board in file Config/board.cfg in "sensors" key.

4) add a sensor ID in Src/Lib/Inc/const.h
   #define SENSOR_LMAA0815 (...)

5) add the sensor in Src/Lib/Inc/sensor.h
   just redo the entries like for other sensors, e.g. search for LM73

6) add the sensor in Src/Lib/Ioutil/sensor.c
   just redo the entries like for other sensors, e.g. search for LM73

7) compile the board supporting the new sensor
   scons board_with_lmaa0815

8) implement the sensor driver code

9) remove this description from lmaa0815.h

10) commit your changes
---8<----------8<----------8<----------8<----------8<----------8<----------8<---
#endif

#ifndef MYSENSOR_H
#define MYSENSOR_H
/**
 * Implementation for mysensor
 */
/* === includes ============================================================ */
#include "sensor_defs.h"

/* === macros ============================================================== */

/* === types =============================================================== */
typedef struct
{
    sensor_driver_t g;
} mysensor_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/* === low level functions ================================================== */

/* === high level sensor functions ========================================= */

static inline void mysensor_trigger(void *pctx, bool one_shot)
{
}

static inline uint8_t mysensor_get_val(void *pctx, uint8_t *pdata)
{
    return 0;
}

static inline uint8_t mysensor_get_raw(void *pctx, uint8_t *pdata)
{
    return 0;
}

static inline void mysensor_sleep(void *pctx)
{
}

/**
 * Create an instance of a mysensor sensor and initialize the sensor.
 *
 * @return sizeof(mysensor_ctx_t)
 *
 */
static inline uint8_t sensor_create_mysensor(void *pdata,
                                         bool raw)
{
    uint8_t rv = sizeof(mysensor_ctx_t);
    mysensor_ctx_t *pcfg;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (mysensor_ctx_t *)pdata;
        pcfg->g.id = SENSOR_MYSENSOR;
        pcfg->g.f_trigger = mysensor_trigger;
        pcfg->g.f_get_val = raw ?  mysensor_get_raw : mysensor_get_val;
        pcfg->g.f_sleep = mysensor_sleep;
        // ... more functions to setup

        /* init private sensor data */

        /* initialize sensor hardware */

    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef MYSENSOR_H */
