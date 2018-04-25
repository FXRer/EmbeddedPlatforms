/* Copyright (c) 2013 Axel Wachtler
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
/* $Id$ */
/**
 * @file
 * @brief Sensor abstraction functions.
 */

/* === includes ============================================================ */
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "sensor.h"
/* === macros ============================================================== */

/* === types =============================================================== */
typedef struct
{
    char sensor_name[16];
    char data_name[16];
} sensor_description_t;

/* === globals ============================================================= */
/**
 * This structure holds the context memory for each sensor.
 */
sensor_description_t SensorDescription[] = {
    #if defined(HAS_SENSOR_LM73)
        {
            .sensor_name = "lm73\0",
            .data_name = "temp\0"
        },
    #endif
    #if defined(HAS_SENSOR_LM73_RAW)
        {
            .sensor_name = "lm73\0",
            .data_name = "rawtemp\0"
        },
    #endif
    #if defined(HAS_SENSOR_MCU_T)
        {
            .sensor_name = "mcu_t\0",
            .data_name = "temp\0",
        },
    #endif
    #if defined(HAS_SENSOR_TSL2550)
        {
            .sensor_name = "tsl2550\0",
            .data_name = "light\0",
        },
    #endif
    #if defined(HAS_SENSOR_HMC5883L)
        {
            .sensor_name = "hmc5883l\0",
            .data_name = "magnetic\0",
        },
    #endif
    #if defined(HAS_SENSOR_TRXVTG)
        {
            .sensor_name = "trxvtg\0",
            .data_name = "voltage\0",
        },
    #endif
    #if defined(HAS_SENSOR_DS18B20)
        {
            .sensor_name = "ds18b20\0",
            .data_name = "temperature\0",
        }
    #endif
};


/** array of pointers to */
static sensor_driver_t* BoardSensors[NB_BOARD_SENSORS];
/*block of memory that stores the sensor context data */
static uint8_t BoardSensorCtx[sizeof(board_sensor_ctx_t)];
static uint8_t LastBoardSensor = 0;
/* === prototypes ========================================================== */

/* === functions =========================================================== */
uint8_t create_board_sensors(void)
{
    uint8_t cnt = 0;
#if NB_BOARD_SENSORS > 0
    uint8_t *psensor, sz;
    memset(BoardSensors, sizeof(BoardSensors), 0);
    memset(BoardSensorCtx, sizeof(BoardSensorCtx), 0);
    LastBoardSensor = 0;
    psensor = BoardSensorCtx;
    #if defined(HAS_SENSOR_LM73) || defined(HAS_SENSOR_LM73_RAW)
        sz = sensor_create_lm73(psensor, 0, LM73_ADDR, 0);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_MCU_T)
        sz = sensor_create_mcu_temp(psensor, 0);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_TSL2550)
        sz = sensor_create_tsl2550(psensor, 0);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_LEDPS)
        sz = sensor_create_ledps(psensor, 0, LEDPS_PORT, LEDPS_ANODE, LEDPS_CATHODE);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_HMC5883L)
        sz = sensor_create_hmc5883l(psensor, 0, HMC5883L_ADDR);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_TRXVTG) || defined(HAS_SENSOR_TRXVTG_RAW)
        sz = sensor_create_trxvtg(psensor);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
    #if defined(HAS_SENSOR_DS18B20)
        sz = sensor_create_ds18b20(psensor, 0);
        BoardSensors[cnt++] = (sensor_driver_t*) psensor;
        psensor += sz;
    #endif
#else
    cnt = 0;
#endif /* NB_BOARD_SENSORS > 0 */
    LastBoardSensor = cnt;
    return cnt;
}

void sensor_trigger(int idx, bool one_shot)
{
    uint8_t i, idstart, idend;
    sensor_driver_t * ps;

    if (idx < 0)
    {
        idstart = 0;
        idend = LastBoardSensor;
    }
    else
    {
        idstart = idx;
        idend = idx + 1;
    }
    for (i = idstart; (i < idend) && (i < LastBoardSensor); i++)
    {
        ps = BoardSensors[i];
        if ((ps != NULL) && (ps->f_trigger != NULL))
        {
            ps->f_trigger(ps, one_shot);
        }
    }
}

uint8_t sensor_get(int idx, uint8_t *pdata)
{
    uint8_t rv, idstart, idend, i;
    sensor_driver_t * ps;
    uint8_t csize = 0;

    rv = 0;

    if (idx < 0)
    {
        idstart = 0;
        idend = LastBoardSensor;
    }
    else
    {
        idstart = idend = idx;
    }

    for (i = idstart; (i < idend) && (i < LastBoardSensor) ; i++)
    {
        ps = BoardSensors[i];
        if ((ps != NULL) &&(ps->f_get_val != NULL))
        {
            csize = ps->f_get_val(ps, pdata);
        }
        if (pdata)
        {
            if (csize == 0)
            {
                *pdata++ = 0x80; /*skipped value */
                *pdata++ = ps->last_error;
            }
            else
            {
                pdata += csize;
            }
        }
        rv += csize;
    }
    if (pdata)
    {
        pdata[0] = SENSOR_DATA_EOR;
        pdata[1] = SENSOR_DATA_EOR;
    }
    /* one byte more for end of record */
    rv += 1;

    return rv;
}

void sensor_sleep(int idx)
{
    uint8_t idstart, idend, i;
    sensor_driver_t * ps;

    if (idx < 0)
    {
        idstart = 0;
        idend = LastBoardSensor;
    }
    else
    {
        idstart = idend = idx;
    }

    for (i = idstart; (i < idend) && (i < LastBoardSensor); i++)
    {
        ps = BoardSensors[i];
        if ((ps != NULL) && (ps->f_sleep != NULL))
        {
            ps->f_sleep(ps);
        }
    }
}

char * sensor_decode(uint8_t *buf, char * line, uint16_t size)
{
    char *pline;
    int sz, nsz, i, nb_raw;
    pline = line;
    sz = size;
    do
    {
        switch (*buf)
        {
            case SENSOR_DATA_TEMPERATURE:
                nsz = snprintf(pline, sz, " temp: %.2f,",
                              (double)((sensor_temperature_t*)buf)->temp );
                pline += nsz;
                sz = sz - nsz;
                buf += sizeof(sensor_temperature_t);
                break;

            case SENSOR_DATA_LIGHT:
                nsz = snprintf(pline, sz, " light: %d,",
                               ((sensor_light_t*)buf)->light );
                pline += nsz;
                sz = sz - nsz;
                buf += sizeof(sensor_light_t);
                break;

            case SENSOR_DATA_MAGNETIC:
                nsz = snprintf(pline, sz, " magnetic[x,y,z]: [%f,%f,%f],",
                                ((sensor_magnetic_t*)buf)->x,
                                ((sensor_magnetic_t*)buf)->y,
                                ((sensor_magnetic_t*)buf)->z
                                );
                pline += nsz;
                sz = sz - nsz;
                buf += sizeof(sensor_magnetic_t);
                break;

            case SENSOR_DATA_VOLTAGE:
                nsz = snprintf(pline, sz, " vtg: %.2f,",
                               (double)((sensor_voltage_t*)buf)->voltage );
                pline += nsz;
                sz = sz - nsz;
                buf += sizeof(sensor_voltage_t);
                break;

            case 0x80:
                nsz = snprintf(pline, sz, " err:%d,", buf[1]);
                pline += nsz;
                sz = sz - nsz;
                buf += 2;
                break;

            case SENSOR_DATA_RELHUMIDITY:
                // todo implement, for now fall through
            case SENSOR_DATA_ACCELERATION:
                // todo implement, for now fall through
            case SENSOR_DATA_EOR:
                *pline = 0;
                break;

            default:
                if (*buf > 127)
                {
                    nsz = snprintf(pline, sz, " raw: ");
                    pline += nsz;
                    sz = sz - nsz;
                    nb_raw = *buf & 0x7f;
                    buf += 1;
                    for (i = 0; i < nb_raw+1; i++)
                    {
                        nsz = snprintf(pline, sz, "%02x ", *buf);
                        pline += nsz;
                        sz = sz - nsz;
                        buf++;
                    }
                    pline[-1] = ',';
                }
                else
                {
                    /* unknown ID, skip rest of buffer */
                    *buf = SENSOR_DATA_EOR;
                }
                break;
        }
    }
    while (*buf != SENSOR_DATA_EOR);
    line[size-1] = 0;
    return line;
}

uint8_t sensor_get_number(void)
{
    return LastBoardSensor;
}

uint8_t sensor_get_id(uint8_t idx)
{
    if (idx < LastBoardSensor)
    {
        return BoardSensors[idx]->id;
    }
    return 0;
}

char* sensor_get_name(uint8_t idx)
{
    if (idx < LastBoardSensor)
    {
        return (char*)SensorDescription[idx].sensor_name;
    }
    return (char*)"?";
}

char* sensor_get_type(uint8_t idx)
{
    if (idx < LastBoardSensor)
    {
        return (char*)SensorDescription[idx].data_name;
    }
    return (char*)"?";
}

uint8_t sensor_get_error(uint8_t idx)
{
    if (idx < LastBoardSensor)
    {
        return BoardSensors[idx]->last_error;
    }
    return 0xff;
}
