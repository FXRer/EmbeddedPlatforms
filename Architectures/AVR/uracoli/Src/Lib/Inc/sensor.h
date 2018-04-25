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

#ifndef SENSOR_H
#define SENSOR_H
/**
 * @file
 * @brief Interface for @ref grpSensor.
 * @addtogroup grpSensor
 * @{
 */

/* === includes ============================================================ */
#include "board.h"
#include "i2c.h"
#include "ow.h"
#include "sensor_defs.h"
#if defined(HAS_SENSOR_LM73)
#    include "sensors/lm73.h"
#endif
#if defined(HAS_SENSOR_MCU_T)
#    include "sensors/mcu_temp.h"
#endif
#if defined(HAS_SENSOR_TSL2550)
#    include "sensors/tsl2550.h"
#endif
#if defined(HAS_SENSOR_LEDPS)
#    include "sensors/ledps.h"
#endif
#if defined(HAS_SENSOR_ISL29020)
#include "sensors/isl29020.h"
#endif
#if defined(HAS_SENSOR_HMC5883L)
#include "sensors/hmc5883l.h"
#endif
#if defined(HAS_SENSOR_TRXVTG)
#include "sensors/trxvtg.h"
#endif

#if defined(HAS_SENSOR_DS18B20)
#include "sensors/ds18b20.h"
#endif

/* === macros ============================================================== */
#define ALL_SENSORS (-1)

/* === types =============================================================== */

/** board defined sensor data structure
 */
typedef struct
{
    #if defined(HAS_SENSOR_LM73) || defined(HAS_SENSOR_LM73_RAW)
        lm73_ctx_t lm73;
    #endif
    #if defined(HAS_SENSOR_MCU_T)
        mcu_temp_ctx_t mcu_temp;
    #endif
    #if defined(HAS_SENSOR_TSL2550)
        tsl2550_ctx_t mcu_temp;
    #endif
    #if defined(HAS_SENSOR_LEDPS)
         ledps_ctx_t led_photosensor;
    #endif
    #if defined(HAS_SENSOR_HMC5883L)
        hmc5883l_ctx_t hmc5883l;
    #endif
    #if defined(HAS_SENSOR_TRXVTG) || defined(HAS_SENSOR_TRXVTG_RAW)
        trxvtg_ctx_t trxvtg;
    #endif
    #if defined(HAS_SENSOR_DS18B20)
        ds18b20_ctx_t ds18b20;
    #endif
    #if NB_BOARD_SENSORS < 1
        uint8_t dummy;
    #endif
} board_sensor_ctx_t;


/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
    /**
     * This function creates the data structure for all sensors that are
     * defined per board.
     */
    uint8_t create_board_sensors(void);

    /**
     * Trigger a sensor measurment.
     *  @param idx number of the sensor or ALL_SENSORS (-1) for all.
     *  @param one_shot if one_shot is true, a single measurement is
     *                  triggered, otherwise a continous measurement.
     *                  It depends on the sensor if this parameter is
     *                  really supported in hardware.
     */
    void sensor_trigger(int idx, bool one_shot);

    /**
     * Query sensor values
     *  @param idx number of the sensor or ALL_SENSORS (-1) for all
     *  @param pdata databuffer to store the sensor data.
     *  @return size in bytes of the queried data block
     *          (size depends on data availability, e.g. if the sensor has data).
     *
     *  Note: To estimate the max. size for pdata, call sensor_get(-1, NULL).
     *        This retrieves the max. block length for all sensors.
     */
    uint8_t sensor_get(int idx, uint8_t *pdata);

    /**
     * Set sensor into sleep mode.
     *  @param idx number of the sensor or ALL_SENSORS (-1) for all
     */
    void sensor_sleep(int idx);

   /**
     * Decode sensor data buffer to string.
     *
     * @param buf sensor data buffer
     * @param line line buffer for decoded text.
     * @param size maximum size of the line buffer
     *
     * @return a string pointer to line or NULL.
     */
    char * sensor_decode(uint8_t *buf, char * line, uint16_t size);

    /**
     * Return the number of sensors for this board.
     */
    uint8_t sensor_get_number(void);
    /**
     * Get sensor ID value for the sensor indexed by idx.
     * @param idx number of the sensor.
     */
    uint8_t sensor_get_id(uint8_t idx);
    /**
     * Get sensor type for the adressed sensor.
     * @param idx number of the sensor.
     */
    char* sensor_get_type(uint8_t idx);
    /**
     * get sensor type for the addressed sensor.
     * @param idx number of the sensor.
     */
    char* sensor_get_name(uint8_t idx);

    /**
     * Get last error code of the addressed sensor.
     * @param idx number of the sensor.
     */
    uint8_t sensor_get_error(uint8_t idx);
#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef SENSOR_H */
