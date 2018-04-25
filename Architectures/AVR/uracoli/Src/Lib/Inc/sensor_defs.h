/* Copyright (c) 2013, 2014 Axel Wachtler
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

#ifndef SENSOR_DEFS_H
#define SENSOR_DEFS_H

/* === includes ============================================================ */

/* === macros ============================================================== */

/** sensor record ends, if this value is found as data type */
#define SENSOR_DATA_EOR (0x0)

#define SENSOR_DATA_TEMPERATURE (0x01)
#define SENSOR_DATA_RELHUMIDITY (0x02)
#define SENSOR_DATA_ACCELERATION (0x03)
#define SENSOR_DATA_LIGHT (0x04)
#define SENSOR_DATA_MAGNETIC (0x05)
#define SENSOR_DATA_VOLTAGE (0x06)

/** Raw data: MSB is set and lower 7 bit give length of raw data array */
#define SENSOR_DATA_RAW (0x80)

/* === types =============================================================== */
typedef enum
{
    SENSOR_ERR_OK = 0,
    SENSOR_ERR_INIT,
    SENSOR_ERR_CRC,
    SENSOR_ERR_PWRON
} sensor_error_t;

/** sensor driver structure */
typedef struct
{
    uint8_t id;
    sensor_error_t last_error;
    void (* f_trigger)(void *ctx, bool one_shot);
    uint8_t (* f_get_val)(void *ctx, uint8_t *pdata);
    void (* f_sleep)(void * ctx);
} sensor_driver_t;

/** sensor value for temperature */
typedef struct
{
    uint8_t type;
    uint8_t sensor;
    float temp;
} sensor_temperature_t;

/** sensor value for light */
typedef struct
{
    uint8_t type;
    uint8_t sensor;
    uint16_t light;
} sensor_light_t;

/** sensor value for 3-axis magnetic, typical unit is Gauss */
typedef struct
{
    uint8_t type;
    uint8_t sensor;
    float x;
    float y;
    float z;
} sensor_magnetic_t;

/** sensor value for light */
typedef struct
{
    uint8_t type;
    uint8_t sensor;
    float voltage;
} sensor_voltage_t;


/** sensor value for raw value */
typedef struct
{
    uint8_t type;
    uint8_t sensor;
    uint8_t data[1];
} sensor_raw_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SENSOR_DEFS_H */
