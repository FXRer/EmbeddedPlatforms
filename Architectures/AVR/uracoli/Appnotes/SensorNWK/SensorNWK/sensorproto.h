/* Copyright (c) 2011 - 2013 Daniel Thiele
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

#ifndef SENSORPROTO_H
#define SENSORPROTO_H

/* === includes ============================================================ */
#include <stdint.h>

#include "../SensorLib/sensors.h"
#include "../SensorLib/mma7455.h"
#include "../SensorLib/sht21.h"
#include "../SensorLib/bmp085.h"

/* === macros ============================================================== */

/* Physical sensor data codes */

#define MEASTASK_NONE					 (0x00)

#define MEASTASK_HUMIDITY_SHT21_bm       (0x10)
                                          
#define MEASTASK_TEMPERATURE_AVR_bm      (0x20)
#define MEASTASK_TEMPERATURE_SHT21_bm    (0x21)
#define MEASTASK_TEMPERATURE_BMP085_bm   (0x22)
#define MEASTASK_TEMPERATURE_BMA250_bm   (0x23)
                                          
#define MEASTASK_ACCELERATION_MMA7455_bm (0x30)
#define MEASTASK_ACCELERATION_LIS3331_bm (0x31)
#define MEASTASK_ACCELERATION_ADXL345_bm (0x32)
#define MEASTASK_ACCELERATION_BMA250_bm  (0x33)
                                          
#define MEASTASK_MAGNETIC_HMC5883_bm     (0x40)
#define MEASTASK_MAGNETIC_AK8975_bm      (0x41)
                                          
#define MEASTASK_PRESSURE_BMP085_bm      (0x40)
                                          
#define MEASTASK_LIGHT_LED_bm            (0x50)

#define MEASTASK_VOLTAGE_AVR_bm          (0x60)

/* take care not to exceed max PSDU size (127 bytes)
 *
 * here: 16 acc values = 6 bytes * 16 = 96 bytes
 */
#define SENSORPROTO_GENERIC_XYZSTREAM_NBSAMPLES (16)

/* === types =============================================================== */

/* type that stores measurement bits from bitmap MEASTASK_... */
typedef uint32_t meastask_t;


typedef struct
{
    meastask_t meas_valid;
    uint16_t vmcu; /**< [mV] */
    uint16_t tmcu; /**< AVR internal temperature sensor */
    uint16_t sht_t; /**< SHT21 temperature [unscaled] */
    uint16_t sht_rh; /**< SHT21 relative humidity [unscaled] */
    sensors_xyz_result_t acc; /**< acceleration */
    uint16_t light;	/**< light sensor */
} sensorproto_muse_data_t;

typedef struct
{
    uint8_t mcusr; /**< Source of reset */
    uint32_t datamiss; /**< number of data packets that could not be delivered since reset */
    meastask_t sensor_available; /**< bitmask for sensor availability for each meas task */
    uint8_t sht_identification[SHT21_IDENTIFICATION_WIDTH];
} sensorproto_muse_info_rep_t;

typedef struct
{
    uint8_t wdto; /**< watchdog timeout, configures data cycle time */
    meastask_t meastasks; /**< bitmap for tasks to be executed */
    uint8_t appmode; /**< 0: slow sens all sensors, 1: ACC stream mode */
    uint16_t accstream_rate_hz;
    uint32_t accstream_nbsamples;
} sensorproto_muse_cfg_t;

typedef struct
{
    meastask_t meas_valid;
    sensors_xyz_result_t acc;
    sensors_xyz_result_t comp;
    uint32_t up;
    uint16_t ut;
    uint16_t vmcu;
} sensorproto_rose_data_t;

typedef struct
{
    bmp085_calibration_t bmpcal;
    sensors_xyz_result_t hmc5883l_cal_positive;
    sensors_xyz_result_t hmc5883l_cal_negative;
} sensorproto_rose_info_rep_t;

typedef struct
{
    meastask_t meas_valid;
    uint16_t vmcu; /**< [mV] */
    uint16_t tmcu; /**< AVR internal temperature sensor */
    uint16_t sht_t; /**< SHT21 temperature [unscaled] */
    uint16_t sht_rh; /**< SHT21 relative humidity [unscaled] */
    sensors_xyz_result_t acc; /**< BMA250 unscaled acceleration values */
    int8_t bma_t; /**< BMA250 temperature unscaled */
    sensors_xyz_result_t compass; /**< HMC5883L magnetic sensor */
    uint32_t up; /**< BMP180 uncompensated pressure */
    uint16_t ut; /**< BMP180 uncompensated temperature */
    uint16_t light;	/**< light sensor */
} sensorproto_museII_data_t;

typedef struct
{
    uint8_t mcusr; /**< Source of reset */
    uint32_t datamiss; /**< number of data packets that could not be delivered since reset */
    meastask_t sensor_available; /**< bitmask for sensor availability for each meas task */
    uint8_t sht_identification[SHT21_IDENTIFICATION_WIDTH];
    bmp085_calibration_t bmpcal;
    sensors_xyz_result_t hmc5883l_cal_positive;
    sensors_xyz_result_t hmc5883l_cal_negative;
    float bma250_sensitivity; /**< mg/LSB */
} sensorproto_museII_info_rep_t;

typedef struct
{
    uint8_t wdto; /**< watchdog timeout, configures data cycle time */
    meastask_t meastasks; /**< bitmap for tasks to be executed */
} sensorproto_museII_cfg_t;

typedef struct
{
    meastask_t meas_valid;
    uint16_t vmcu; /**< [mV] */
    uint16_t tmcu; /**< AVR internal temperature sensor */
} sensorproto_radiofaro_data_t;

typedef struct
{
    uint8_t nbsamples; /**< Number of samples of following array */
    sensors_xyz_result_t xyz[SENSORPROTO_GENERIC_XYZSTREAM_NBSAMPLES];
} sensorproto_generic_xyzstream_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SENSORPROTO_H */
