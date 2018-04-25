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

#ifndef MCU_TEMP_H
#define MCU_TEMP_H
/**
 * @defgroup grpSensorMCUTEMP MCU Temperature Sensor
 * @ingroup grpSensorDefs
 * @{
 *
 * Measuring environment temperature with the MCU internal ADC.
 *
 */

/* === includes ============================================================ */
#include "sensor_defs.h"

/* === macros ============================================================== */

/* === types =============================================================== */
typedef struct {
    sensor_driver_t g;
    int16_t adc_offset;
} mcu_temp_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/* === high level sensor functions ========================================= */

static inline void mcu_temp_trigger(void *pctx, bool one_shot)
{
    ADCSRC = 10<<ADSUT0; // set start-up time
    ADCSRB = 1<<MUX5; // set MUX5 first
    ADMUX = (3<<REFS0) + (9<<MUX0); // store new ADMUX, 1.6V AREF
    // switch ADC on, set prescaler, start conversion
    ADCSRA = (1<<ADEN) + (1<<ADSC) + (4<<ADPS0);
}

/**
 * @ref grpSensor Sensor API get functions
 */
static inline uint8_t mcu_temp_get_val(void *pctx, uint8_t *pdata)
{
    uint8_t rv, i;
    int16_t t_accu = 0;
    #define NB_AVG (5)

    rv = sizeof(sensor_temperature_t);

    if (pdata != NULL)
    {
        for (i = 0; i < NB_AVG; i++)
        {
            while( (ADCSRA & (1<<ADSC))); // wait for conversion end
            t_accu += (ADC - ((mcu_temp_ctx_t*)pctx)->adc_offset);
        }
        t_accu -= 1;
        ADCSRA = 0; // disable the ADC
        ((sensor_temperature_t*)pdata)->type = SENSOR_DATA_TEMPERATURE;
        ((sensor_temperature_t*)pdata)->sensor = SENSOR_MCU_T;
        ((sensor_temperature_t*)pdata)->temp = (1.13 * (t_accu/(float)(NB_AVG)) - 272.8);
    }
    else
    {
        rv = 0;
    }
    return rv;
}

/** not implemented, returns noting */
static inline uint8_t mcu_temp_get_raw(void *pctx, uint8_t *pdata)
{
    uint8_t rv = 0;
    return rv;
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
static inline uint8_t sensor_create_mcu_temp(void *pdata, bool raw)
{
    uint8_t rv = sizeof(mcu_temp_ctx_t);
    mcu_temp_ctx_t *pcfg;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (mcu_temp_ctx_t *)pdata;
        pcfg->g.id = SENSOR_MCU_T;
        pcfg->g.f_trigger = mcu_temp_trigger;
        pcfg->g.f_get_val = raw ?  mcu_temp_get_raw : mcu_temp_get_val;
        pcfg->g.f_sleep = NULL;

        /* initialize sensor */

        /* get ADC offset */
        ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
        ADCSRB = 0;
        ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3); /* reference: 1.6V, differential ADC0-ADC0 10x */

        _delay_us(200); /* some time to settle */
        while( (ADCSRA & (1<<ADSC))); // wait for conversion end
        _delay_us(200);
        while( (ADCSRA & (1<<ADSC)));

        ADCSRA = 0; // disable the ADC
        ((mcu_temp_ctx_t*) pcfg)->adc_offset = ADC;
    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef MCU_TEMP_H */
