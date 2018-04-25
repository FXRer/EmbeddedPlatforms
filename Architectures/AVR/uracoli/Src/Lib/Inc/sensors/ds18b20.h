/* Copyright (c) 2014 Daniel Thiele, Axel Wachtler
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

#ifndef DS18B20_H
#define DS18B20_H
/**
 * @defgroup grpSensorDS1820 DS18B20 - One Wire Temperature Sensor.
 * @ingroup grpSensorDefs
 * @{
 *
 * The DS18B20 digital thermometer provides 9-bit to 12-bit Celsius temperature
 * measurements and has an alarm function with nonvolatile user-programmable
 * upper and lower trigger points.
 *
 * - Product Page: http://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html
 * - Datasheet: http://pdfserv.maximintegrated.com/en/ds/DS18B20.pdf
 *
 */

/* === includes ============================================================ */
#include <util/crc16.h>
#include <string.h>
#include "ow.h"
#include "sensor_defs.h"

/* === macros ============================================================== */
/**
 *
 * With the definition of the following macros, multiple ds18b20 sensors can
 * be configured, per default the driver assumes just one (anonymous) sensor.
 *
 * @code
 * #define DS18B20_NB (3)
 * #define DS18B20_ADDR { 0xfc000005eab97828, \
 *                        0x90000005eb460e28, \
 *                        0xc5000005ea2dc128 }
 * @endcode
 *
 * This macros can either be manually in the apropriate board_xyz.h or they can
 * added in the board.cfg file in the form:
 *
 * @code
 * sensors: ... ds18b20:0xfc000005eab97828,0x90000005eb460e28,0xc5000005ea2dc128
 * @endcode
 *
 * When using the second method, the macros appear in the generated board_cfg.h.
 *
 */

#if !defined(DS18B20_NB)
# define DS18B20_NB (1)
# define DS18B20_ADDR {0}
#endif


/* undefine if DS18B20 is self-powered */
#define DS18B20_PARASITE_POWER (1)

#define DS18B20_DEBUG (0)
#if DS18B20_DEBUG == 1
# include "hif.h"
# define DBG(...) PRINTF(__VA_ARGS__)
# define DBGP(x) PRINT(x)
#else
# define DBG(...)
# define DBGP(x)
#endif


/* depends on selected resolution */
#define DS18B20_CONV_TIME_MS (188) /* 10-bit resolution, 187.5ms */
#define DS18B20_COPYSCRATCH_TIME_MS (20) /* at least 10ms, acc. to datasheet */
#define DS18B20_SIZE_SCRATCHPAD (9)

#define DS18B20_FUNCCMD_CONVT (0x44)
#define DS18B20_FUNCCMD_READSCRATCH (0xBE)
#define DS18B20_FUNCCMD_WRITESCRATCH (0x4E)
#define DS18B20_FUNCCMD_COPYSCRATCH (0x48)
#define DS18B20_FUNCCMD_RECALLEE (0xB8)
#define DS18B20_FUNCCMD_READPOWER (0xB4)

/* === types =============================================================== */
/** DS18B20 context structure */
typedef struct
{
    sensor_driver_t g;
    ow_serial_t addr[DS18B20_NB];
} ds18b20_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/* === low level functions ================================================== */
/**
 * Send command to 1 wire sensor.
 *
 * @param command command code
 * @param *buf argument buffer
 * @return error code
 */
static inline uint8_t ds18b20_command(uint8_t command, uint8_t *buf)
{
    uint8_t i;
    uint8_t crc = 0;
    uint8_t rv = SENSOR_ERR_OK;

    switch(command)
    {
        case DS18B20_FUNCCMD_CONVT :
            break;
        case DS18B20_FUNCCMD_READSCRATCH:
            ow_byte_write(DS18B20_FUNCCMD_READSCRATCH);
            DBGP("read scratch:");
            for(i=0; i<9; i++)
            {
                *buf = ow_byte_read();
                DBG("0x%02x ", *buf);
                crc = _crc_ibutton_update(crc, *buf++);
            }
            DBG("crc = 0x%x\n", crc);
            if (crc != 0)
            {
                rv = SENSOR_ERR_CRC;
            }
            break;
        case DS18B20_FUNCCMD_WRITESCRATCH:
            break;
        case DS18B20_FUNCCMD_COPYSCRATCH:
#if defined(DS18B20_PARASITE_POWER)
            PINHI(DQ);
            PINOUTP(DQ);
#endif
            break;
        case DS18B20_FUNCCMD_RECALLEE:
            break;
        case DS18B20_FUNCCMD_READPOWER:
            break;
        default:
            /* unknown */
            break;
    }

    return rv;
}
/**
 * Read the temperature value
 *
 * @param ser serial number of the device or 0 if skip addressing
 *            (in case if just one sensor is present)
 * @param *temp pointer that holds the singned integer temperature value.
 * @return error code of conversion
 */
static inline uint8_t ds18b20_read_temperature(ow_serial_t ser, int16_t *temp)
{
    uint8_t buf[DS18B20_SIZE_SCRATCHPAD];
    uint8_t rv = SENSOR_ERR_OK;
    /* send convert to all */
    ow_reset();
    ow_byte_write(OW_ROMCMD_SKIP);
    ow_byte_write(DS18B20_FUNCCMD_CONVT);
    _delay_ms(DS18B20_CONV_TIME_MS);

    if (ser != 0)
    {
        ow_master_matchrom(ser);
        DBGP("match rom");

    }
    else
    {
        ow_reset();
        ow_byte_write(OW_ROMCMD_SKIP);
        DBGP("skip rom");
    }


#if defined(DS18B20_PARASITE_POWER)
    PINHI(DQ);
    PINOUTP(DQ);
#endif


#if defined(DS18B20_PARASITE_POWER)
    PININP(DQ);
    PINLO(DQ);
#endif

    rv = ds18b20_command(DS18B20_FUNCCMD_READSCRATCH, buf);
    if (rv == SENSOR_ERR_OK)
    {
        *temp = ( ((int16_t)buf[1] << 8)| ((int16_t)buf[0] << 0) )/16;
    }
    /* TODO: evaluate CRC */
    return rv;
}

/* === high level sensor functions ========================================= */
/** start converion not implemented */
static inline void ds18b20_trigger(void *pctx, bool one_shot)
{
}

/** return temperature value */
static inline uint8_t ds18b20_get_val(void *pctx, uint8_t *pdata)
{
    uint8_t rv, err, i;
    int16_t temp;
    ds18b20_ctx_t *p = (ds18b20_ctx_t*) pctx;

    rv = sizeof(sensor_temperature_t)*DS18B20_NB;
    if (pdata != NULL)
    {
        rv = 0;
        for (i = 0; i < DS18B20_NB; i++)
        {
            err = ds18b20_read_temperature(p->addr[i], &temp);
            if (temp == 85 && err == SENSOR_ERR_OK)
            {
                /* TODO: add some quirks or heuristics,
                   If temp is const 85°C we will have a problem here.
                   What a silly chip design - to signal PWRON with a value
                   just from the middle of the valid data range!
                 */
                err = SENSOR_ERR_PWRON;
            }
            if (err == SENSOR_ERR_OK)
            {
                ((sensor_temperature_t*)pdata)->type = SENSOR_DATA_TEMPERATURE;
                ((sensor_temperature_t*)pdata)->sensor = SENSOR_DS18B20;
                ((sensor_temperature_t*)pdata)->temp = (float)temp;
                rv += sizeof(sensor_temperature_t);
                pdata += sizeof(sensor_temperature_t);
            }
            else
            {
                *pdata++ = 0x80;
                *pdata++ = err;
                rv += 2;
            }
            ((ds18b20_ctx_t*)pctx)->g.last_error = err;
            DBG("ds18b20_get_val: temp=%d, rv=%d, err=%d\n\r", temp, rv, err);
        }
    }

    return rv;
}

static inline uint8_t ds18b20_get_raw(void *pctx, uint8_t *pdata)
{
    return 0;
}

static inline void ds18b20_sleep(void *pctx)
{
}

/**
 * Create an instance of a ds18b20 sensor and initialize the sensor.
 *
 * @return sizeof(ds18b20_ctx_t)
 *
 */
static inline uint8_t sensor_create_ds18b20(void *pdata, bool raw)
{
    uint8_t rv = sizeof(ds18b20_ctx_t);
    ds18b20_ctx_t *pcfg;
    ow_serial_t tmp_addr[DS18B20_NB] = DS18B20_ADDR;
    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (ds18b20_ctx_t *)pdata;
        pcfg->g.id = SENSOR_DS18B20;
        pcfg->g.f_trigger = NULL;
        pcfg->g.f_get_val = raw ?  ds18b20_get_raw : ds18b20_get_val;
        pcfg->g.f_sleep = NULL;
        // ... more functions to setup

        /* init private sensor data */
        memcpy(pcfg->addr, tmp_addr, sizeof(tmp_addr));
        /* TODO: scan bus for DS18B20 devices and store addresses sorted */

        /* initialize sensor hardware */
    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif /* DS18B20_H */
