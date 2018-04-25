/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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

/**
 * @file
 * @brief ....
 * @_addtogroup grpApp...
 */

/* avr-libc inclusions */
#include <string.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/twi.h>

/* uracoli inclusions */
#include <board.h>
#include <transceiver.h>
#include <radio.h>
#include <hif.h>
#include <timer.h>

/* project inclusions */
#include <sensornwk.h>
#include <sensorproto.h>
#include <adxl345.h>
#include <bma250.h>
#include <hmc5883l.h>
#include <atmega128rfa.h>
#include "cmdif.h"

#define MEASTASK_FORMATTER "%08lX" /* depends on type of "meastask_t" */

#define EOL "\n"

typedef struct
{
    uint8_t phr; /**< PHY header (frame length) */
    uint8_t psdu[MAX_FRAME_SIZE]; /**< PHY payload */
    int8_t ed; /**< ED level */
    time_t tstamp; /**< Rx timestamp */
    uint8_t lqi; /**< LQI */
} rxfrm_t;

typedef struct
{
    sensornwk_frametype_t type;
    void (*output)(uint8_t*);
    char *name;
} frametype_map_t;

typedef enum
{
    OUTPUT_UART_ASCII, /**< output ascii lines to uart set in hif_ */
    OUTPUT_RAVEN /**< output to IO of Raven */
} output_t;

static rxfrm_t rxfrm;

static bmp085_calibration_t bmp085_cal; /* read once */
static sensors_xyz_result_t hmc5883l_cal_positive;
static sensors_xyz_result_t hmc5883l_cal_negative;
static float adxl345_sensitivity = 3.9; /* mg/LSB */
static float bma250_sensitivity = 0; /* mg/LSB */

static const uint8_t i2c_hostaddr = 0xb8;

static const struct
{
    uint8_t type;
    int8_t rssi_base_val;
    char *name;
} boardmap[] =
{
    {.type=BOARD_ROSE231, .rssi_base_val=-91, .name="rose231"},
    {.type=BOARD_MUSE231, .rssi_base_val=-91, .name="muse231"},
    {.type=BOARD_LGEE231, .rssi_base_val=-91, .name="lgee231"},
    {.type=BOARD_MUSEII232, .rssi_base_val=-91, .name="museII232"},
    {.type=BOARD_MUSEIIRFA, .rssi_base_val=-91, .name="museIIrfa"},
    {.type=BOARD_RDK230B, .rssi_base_val=-91, .name="rdk230b"},
    {.type=BOARD_RADIOFARO, .rssi_base_val=-91, .name="radiofaro"},
    {.type=BOARD_PINOCCIO, .rssi_base_val=-91, .name="pinoccio"},
};


ISR(TWI_vect, ISR_BLOCK)
{
    uint8_t status = TWSR & TW_STATUS_MASK;
    switch (status)
    {
    case TW_START:
        break;
    case TW_REP_START:
        break;
    case TW_MT_SLA_ACK:
        break;
    case TW_MT_DATA_ACK:
        break;
    case TW_MT_SLA_NACK:
        break;
    case TW_MT_DATA_NACK:
        break;
    case TW_MT_ARB_LOST:
        break;
    case TW_MR_SLA_ACK:
        break;
    case TW_MR_SLA_NACK:
        break;
    case TW_MR_DATA_ACK:
        break;
    case TW_MR_DATA_NACK:
        break;
    case TW_ST_SLA_ACK:
        break;
    case TW_ST_ARB_LOST_SLA_ACK:
        break;
    case TW_ST_DATA_ACK:
        break;
    case TW_ST_DATA_NACK:
        break;
    case TW_ST_LAST_DATA:
        break;
    case TW_SR_SLA_ACK:
        break;
    case TW_SR_ARB_LOST_SLA_ACK:
        break;
    case TW_SR_GCALL_ACK:
        break;
    case TW_SR_ARB_LOST_GCALL_ACK:
        break;
    case TW_SR_DATA_ACK:
        putchar(TWDR);
        break;
    case TW_SR_DATA_NACK:
        break;
    case TW_SR_GCALL_DATA_ACK:
        break;
    case TW_SR_GCALL_DATA_NACK:
        break;
    case TW_SR_STOP:
        break;
    case TW_NO_INFO:
        break;
    case TW_BUS_ERROR:
        break;
    default:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
        break;
    }
    TWCR |= (1<<TWINT); /* manually clear */
}

static void output_ascii_muse231_data(uint8_t *d)
{
    sensorproto_muse_data_t *data = (sensorproto_muse_data_t*)d;

    printf(", MASK=0x"MEASTASK_FORMATTER, data->meas_valid);

    if(data->meas_valid & MEASTASK_VOLTAGE_AVR_bm)
    {
        printf(", VMCU[mV]=%u", data->vmcu);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_SHT21_bm)
    {
        printf(", SHT_T[degC]=%.2f", (double)sht21_scale_temperature(data->sht_t));
    }

    if(data->meas_valid & MEASTASK_HUMIDITY_SHT21_bm)
    {
        printf(",  SHT_RH[vH]=%.2f", (double)sht21_scale_humidity(data->sht_rh));
    }

    if(data->meas_valid & MEASTASK_ACCELERATION_ADXL345_bm)
    {
        printf(", AX=%f, AY=%f, AZ=%f",
               (double)adxl345_scale(data->acc.x, adxl345_sensitivity),
               (double)adxl345_scale(data->acc.y, adxl345_sensitivity),
               (double)adxl345_scale(data->acc.z, adxl345_sensitivity)
              );
    }

    if(data->meas_valid & MEASTASK_LIGHT_LED_bm)
    {
        printf(",  LIGHT=%u", data->light);
    }
}

static void output_ascii_muse231_info_rep(uint8_t *d)
{
    sensorproto_muse_info_rep_t *info = (sensorproto_muse_info_rep_t*)d;

    printf(", RST=0x%02X", info->mcusr);
    printf(", AVAILABLE=0x"MEASTASK_FORMATTER, info->sensor_available);

    printf(", SHT_ID=[");
    for(uint8_t i=0; i<sizeof(info->sht_identification)/sizeof(info->sht_identification[0]); i++)
    {
        printf("%02X", info->sht_identification[i]);
    }
    printf("]");
}

static void output_ascii_muse231_accstream_scaled(uint8_t *d)
{
    sensorproto_generic_xyzstream_t *data = (sensorproto_generic_xyzstream_t*)d;

    for(uint8_t ax=0; ax<3; ax++)
    {
        printf(", %c=[", 'X'+ax);
        for(uint8_t i=0; i<sizeof(data->xyz)/sizeof(data->xyz[0]); i++)
        {
            if(i)
            {
                putchar(' ');
            }
            printf("%.3f", (double)adxl345_scale(*(&data->xyz[i].x+ax), adxl345_sensitivity));
        }
        printf("]");
    }
}

static void output_ascii_museII232_accstream(uint8_t *d)
{
    sensorproto_generic_xyzstream_t *data = (sensorproto_generic_xyzstream_t*)d;

    for(uint8_t ax=0; ax<3; ax++)
    {
        printf(", %c=[", 'X'+ax);
        for(uint8_t i=0; i<sizeof(data->xyz)/sizeof(data->xyz[0]); i++)
        {
            if(i)
            {
                putchar(' ');
            }
            printf("%.3f", (double)0); /* TODO */
        }
        printf("]");
    }
}

static void output_ascii_museII232_magstream(uint8_t *d)
{
    sensorproto_generic_xyzstream_t *data = (sensorproto_generic_xyzstream_t*)d;

    printf(", N=%u ", data->nbsamples);
    for(uint8_t ax=0; ax<3; ax++)
    {
        printf(", %c=[", 'X'+ax);
        for(uint8_t i=0; i<data->nbsamples; i++)
        {
            if(i)
            {
                putchar(' ');
            }
            printf("%.3f", (double)sensors_hmc5883l_scale(*(&data->xyz[i].x+(int16_t)ax),
                    *(&hmc5883l_cal_positive.x+(int16_t)ax),
                    *(&hmc5883l_cal_negative.x+(int16_t)ax),
                    3,
                    'X'+ax ));
        }
        printf("]");
    }
}

static void output_ascii_muse231_accstream_unscaled(uint8_t *d)
{
    sensorproto_generic_xyzstream_t *data = (sensorproto_generic_xyzstream_t*)d;

    for(uint8_t ax=0; ax<3; ax++)
    {
        printf(", %c=[", 'X'+ax);
        for(uint8_t i=0; i<sizeof(data->xyz)/sizeof(data->xyz[0]); i++)
        {
            if(i)
            {
                putchar(' ');
            }
            printf("%d", (int16_t)(*(&data->xyz[i].x+ax)));
        }
        printf("]");
    }
}

static void output_ascii_rose231_data(uint8_t *d)
{
    sensorproto_rose_data_t *data = (sensorproto_rose_data_t*)d;

    printf(", AX=%d, AY=%d, AZ=%d, CX=%d, CY=%d, CZ=%d, "
           "UP=%lu, VMCU[mV]=%u, UT=%u",
           data->acc.x, data->acc.y, data->acc.z,
           data->comp.x, data->comp.y, data->comp.z,
           data->up,
           data->vmcu,
           data->ut
          );

    printf(", T=%f, P=%f",
           (double)bmp085_scale_ut(data->ut, &bmp085_cal),
           (double)bmp085_scale_up(data->up, 3, &bmp085_cal)
          );
    printf(", CX[Ga]=%f, CY[Ga]=%f, CZ[Ga]=%f",
           (double)sensors_hmc5883l_scale(data->comp.x,
                                          hmc5883l_cal_positive.x,
                                          hmc5883l_cal_negative.x,
                                          3,
                                          'X' ),
           (double)sensors_hmc5883l_scale(data->comp.y,
                                          hmc5883l_cal_positive.y,
                                          hmc5883l_cal_negative.y,
                                          3,
                                          'Y' ),
           (double)sensors_hmc5883l_scale(data->comp.z,
                                          hmc5883l_cal_positive.z,
                                          hmc5883l_cal_negative.z,
                                          3,
                                          'Z' )
          );
}

static void output_ascii_rose231_info_rep(uint8_t *d)
{
    sensorproto_rose_info_rep_t *info = (sensorproto_rose_info_rep_t*)d;
    bmp085_calibration_t *cal = &info->bmpcal;

    memcpy(&bmp085_cal, &info->bmpcal, sizeof(bmp085_calibration_t));
    printf(", AC1=%d, AC2=%d, AC3=%d, AC4=%u, AC5=%u, AC6=%u"
           ", B1=%d, B2=%d, MB=%d, MC=%d, MD=%d",
           cal->ac1, cal->ac2, cal->ac3,
           cal->ac4, cal->ac5, cal->ac6,
           cal->b1, cal->b2,
           cal->mb, cal->mc, cal->md
          );

    memcpy(&hmc5883l_cal_positive, &info->hmc5883l_cal_positive, sizeof(sensors_xyz_result_t));
    memcpy(&hmc5883l_cal_negative, &info->hmc5883l_cal_negative, sizeof(sensors_xyz_result_t));
    printf(", CXP=%d, CXN=%d, CYP=%d, CYN=%d, CZP=%d, CZN=%d",
           hmc5883l_cal_positive.x, hmc5883l_cal_negative.x,
           hmc5883l_cal_positive.y, hmc5883l_cal_negative.y,
           hmc5883l_cal_positive.z, hmc5883l_cal_negative.z
          );
}

static void output_ascii_museII_data_unscaled(uint8_t *d)
{
    sensorproto_museII_data_t *data = (sensorproto_museII_data_t*)d;
    printf(", MASK=0x"MEASTASK_FORMATTER, data->meas_valid);

    if(data->meas_valid & MEASTASK_VOLTAGE_AVR_bm)
    {
        printf(", VMCU=%u", data->vmcu);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_AVR_bm)
    {
        printf(", TMCU=%u", data->tmcu);
    }

    if(data->meas_valid & MEASTASK_LIGHT_LED_bm)
    {
        printf(",  LIGHT=%u", 0);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_SHT21_bm)
    {
        printf(", SHT_T=%u", data->sht_t);
    }

    if(data->meas_valid & MEASTASK_HUMIDITY_SHT21_bm)
    {
        printf(",  SHT_RH=%u", data->sht_rh);
    }

    if(data->meas_valid & MEASTASK_ACCELERATION_BMA250_bm)
    {
        printf(", AX=%d, AY=%d, AZ=%d", data->acc.x, data->acc.y, data->acc.z
              );
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_BMA250_bm)
    {
        printf(", BMA_T=%d", data->bma_t);
    }

    if(data->meas_valid & MEASTASK_MAGNETIC_HMC5883_bm)
    {
        printf(", CX=%d, CY=%d, CZ=%d", data->compass.x, data->compass.y, data->compass.z);
    }

    if(data->meas_valid & MEASTASK_PRESSURE_BMP085_bm)
    {
        printf(", UP=%lu",	data->up);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_BMP085_bm)
    {
        printf(", UT=%u", data->ut);
    }

}

static void output_ascii_museII_data_scaled(uint8_t *d)
{
    sensorproto_museII_data_t *data = (sensorproto_museII_data_t*)d;

    printf(", MASK=0x"MEASTASK_FORMATTER, data->meas_valid);

    if(data->meas_valid & MEASTASK_VOLTAGE_AVR_bm)
    {
        printf(", VMCU[mV]=%u", data->vmcu);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_AVR_bm)
    {
        printf(", TMCU[degC]=%.2f", (double)atmega128rfa_scale_temperature(data->tmcu));
    }

    if(data->meas_valid & MEASTASK_LIGHT_LED_bm)
    {
        printf(", LIGHT=%u", 0);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_SHT21_bm)
    {
        printf(", SHT_T[degC]=%.2f", (double)sht21_scale_temperature(data->sht_t));
    }

    if(data->meas_valid & MEASTASK_HUMIDITY_SHT21_bm)
    {
        printf(", SHT_RH[vH]=%.2f", (double)sht21_scale_humidity(data->sht_rh));
    }

    if(data->meas_valid & MEASTASK_ACCELERATION_BMA250_bm)
    {
        printf(", AX=%f, AY=%f, AZ=%f",
               (double)bma250_scale_acceleration(data->acc.x, bma250_sensitivity),
               (double)bma250_scale_acceleration(data->acc.y, bma250_sensitivity),
               (double)bma250_scale_acceleration(data->acc.z, bma250_sensitivity)
              );
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_BMA250_bm)
    {
        printf(", BMA_T[degC]=%.1f", (double)bma250_scale_temperature(data->bma_t));
    }

    if(data->meas_valid & MEASTASK_MAGNETIC_HMC5883_bm)
    {
        printf(", CX[Ga]=%f, CY[Ga]=%f, CZ[Ga]=%f",
               (double)sensors_hmc5883l_scale(data->compass.x,
                                              hmc5883l_cal_positive.x,
                                              hmc5883l_cal_negative.x,
                                              3,
                                              'X' ),
               (double)sensors_hmc5883l_scale(data->compass.y,
                                              hmc5883l_cal_positive.y,
                                              hmc5883l_cal_negative.y,
                                              3,
                                              'Y' ),
               (double)sensors_hmc5883l_scale(data->compass.z,
                                              hmc5883l_cal_positive.z,
                                              hmc5883l_cal_negative.z,
                                              3,
                                              'Z' )
              );
    }

    if(data->meas_valid & MEASTASK_PRESSURE_BMP085_bm)
    {
        printf(", BMP_P[Pa]=%f", (double)bmp085_scale_up(data->up, 3, &bmp085_cal));
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_BMP085_bm)
    {
        printf(", BMP_T[degC]=%f", (double)bmp085_scale_ut(data->ut, &bmp085_cal));
    }
}

static void output_ascii_museII_info_rep(uint8_t *d)
{
    sensorproto_museII_info_rep_t *info = (sensorproto_museII_info_rep_t*)d;
    bmp085_calibration_t *cal = &info->bmpcal;

    printf(", SHT_ID=[");
    for(uint8_t i=0; i<sizeof(info->sht_identification)/sizeof(info->sht_identification[0]); i++)
    {
        printf("%02X", info->sht_identification[i]);
    }
    printf("]");

    memcpy(&bmp085_cal, &info->bmpcal, sizeof(bmp085_calibration_t));
    printf(", AC1=%d, AC2=%d, AC3=%d, AC4=%u, AC5=%u, AC6=%u"
           ", B1=%d, B2=%d, MB=%d, MC=%d, MD=%d",
           cal->ac1, cal->ac2, cal->ac3,
           cal->ac4, cal->ac5, cal->ac6,
           cal->b1, cal->b2,
           cal->mb, cal->mc, cal->md
          );

    memcpy(&hmc5883l_cal_positive, &info->hmc5883l_cal_positive, sizeof(sensors_xyz_result_t));
    memcpy(&hmc5883l_cal_negative, &info->hmc5883l_cal_negative, sizeof(sensors_xyz_result_t));
    printf(", CXP=%d, CXN=%d, CYP=%d, CYN=%d, CZP=%d, CZN=%d",
           hmc5883l_cal_positive.x, hmc5883l_cal_negative.x,
           hmc5883l_cal_positive.y, hmc5883l_cal_negative.y,
           hmc5883l_cal_positive.z, hmc5883l_cal_negative.z
          );

    bma250_sensitivity = info->bma250_sensitivity;
    printf(", BMA250_SENS=%f", (double)bma250_sensitivity);
}

static void output_ascii_radiofaro_data(uint8_t *d)
{
    sensorproto_radiofaro_data_t *data = (sensorproto_radiofaro_data_t*)d;

    printf(", MASK=0x"MEASTASK_FORMATTER, data->meas_valid);

    if(data->meas_valid & MEASTASK_VOLTAGE_AVR_bm)
    {
        printf(", VMCU[mV]=%u", data->vmcu);
    }

    if(data->meas_valid & MEASTASK_TEMPERATURE_AVR_bm)
    {
        printf(", TMCU=%u", data->tmcu);
    }
}


static const frametype_map_t framemap[] =
{
    {
        .type = SENSORNWK_FRAMETYPE_NOP,
        .name = "NOP",
        .output = NULL,
    },
    {
        .type = SENSORNWK_FRAMETYPE_HEREAMI,
        .name = "HEREAMI",
        .output = NULL,
    },
    {
        .type = SENSORNWK_FRAMETYPE_ASSOCIATION_REQ,
        .name = "ASSOCIATION_REQ",
        .output =NULL,
    },
    {
        .type = SENSORNWK_FRAMETYPE_ASSOCIATION_REP,
        .name = "ASSOCIATION_REP",
        .output = NULL,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_ROSE_INFO_REP,
        .name = "INFO",
        .output = output_ascii_rose231_info_rep,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_ROSE_DATA,
        .name = "DATA",
        .output = output_ascii_rose231_data,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSE_INFO_REP,
        .name = "INFO",
        .output = output_ascii_muse231_info_rep,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSE_DATA,
        .name = "DATA",
        .output = output_ascii_muse231_data,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSE_TAP,
        .name = "TAP",
        .output = NULL,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSE_ACCSTREAM,
        .name = "ACC",
        .output = output_ascii_muse231_accstream_unscaled,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSEII_DATA,
        .name = "DATA",
        .output = output_ascii_museII_data_scaled,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSEII_INFO_REP,
        .name = "INFO",
        .output = output_ascii_museII_info_rep,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSEII_ACCSTREAM,
        .name = "ACC",
        .output = output_ascii_museII232_accstream,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_MUSEII_MAGSTREAM,
        .name = "MAG",
        .output = output_ascii_museII232_magstream,
    },
    {
        .type =  SENSORNWK_FRAMETYPE_RADIOFARO_DATA,
        .name = "DATA",
        .output = output_ascii_radiofaro_data,
    },
};

static inline void rx_decode(rxfrm_t *rxfrm)
{
    sensornwk_hdr_t *hdrfrm;
    uint8_t *data = &rxfrm->psdu[sizeof(sensornwk_hdr_t)];
    uint8_t idx;

    hdrfrm = (sensornwk_hdr_t*)rxfrm->psdu;

    printf("BOARD=");
    for(idx=0; idx<sizeof(boardmap)/sizeof(boardmap[0]); idx++)
    {
        if(boardmap[idx].type == hdrfrm->boardtype)
        {
            break;
        }
    }
    if(sizeof(boardmap)/sizeof(boardmap[0]) == idx) /* boardtype has not been found in list boardmap */
    {
        printf("INVALID(%02X)", hdrfrm->boardtype);
        return;
    }

    /* Fields that are common to all boards */
    printf("'%s', ADDR=%04X, RSSI[dBm]=%d, TSTAMP=%lu",
           boardmap[idx].name,
           hdrfrm->srcaddr,
           boardmap[idx].rssi_base_val + rxfrm->ed,
           timer_systime()
          );

    printf(", FRAMETYPE=");
    for(idx=0; idx<sizeof(framemap)/sizeof(framemap[0]); idx++)
    {
        if(framemap[idx].type == hdrfrm->frametype)
        {
            printf("'%s'", framemap[idx].name);
            if(NULL != framemap[idx].output)
            {
                framemap[idx].output(data);
            }
            break;
        }
    }

    if(sizeof(framemap)/sizeof(framemap[0]) == idx)  /* frametype has not been found in list framemap */
    {
        printf("INVALID(%02X)", hdrfrm->frametype);
        return;
    }

    putchar('\n');
}

/*
 * \brief Callback for uracoli lib
 */
uint8_t * cb_sensornwk_rx(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed)
{
    rxfrm.phr = len;
    rxfrm.ed = ed;
    rxfrm.lqi = lqi;
    rxfrm.tstamp = timer_systime();
    memcpy(rxfrm.psdu, frm, len);

    rx_decode((rxfrm_t*)&rxfrm);

    return (uint8_t*)rxfrm.psdu;
}

/*
 * \brief Callback for sensorproto module
 */
void cb_sensornwk_done(void)
{
    printf("Tx done"EOL);
}

/*
 * \brief Callback for sensorproto module
 *
 * Not useful for host (does not associate) but required for comipiling
 */
void cb_sensornwk_association_cnf(uint16_t addr)
{
    printf("Associated 0x%04X"EOL, addr);
}


/* setup stream for UART communication */
static FILE hifstream = FDEV_SETUP_STREAM(hif_putc, hif_getc, _FDEV_SETUP_RW);


int main()
{
    timer_init();
    timer_set_systime(0);
    i2c_init(4000000UL);
    TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT) | (1<<TWIE); /* enable auto acknowledge */
    TWAR = i2c_hostaddr;

#if defined(wdba1281) /* enable I2C driver U4 */
    DDRD |= (1<<PD7);
    PORTD |= (1<<PD7);
#endif

    sensornwk_init(SENSORNWK_DEVTYPE_COORD, BOARD_TYPE);

    hif_init(HIF_DEFAULT_BAUDRATE);
    stdin = stdout = stderr = &hifstream;

    sei();

	printf("Sensor Host <BUILD %s %s>"EOL, __DATE__, __TIME__);

	node_config_t *cfg = sensornwk_get_config();
	printf("PAN_ID=x%04X, SHORT_ADDR=x%04X, CHANNEL=x%02X"EOL, cfg->pan_id, cfg->short_addr, cfg->channel);
	printf("I2C Host at addr 0x%02X"EOL, i2c_hostaddr);
	printf("Supported Sensors:"EOL);
	for(uint8_t i=0; i<sizeof(boardmap)/sizeof(boardmap[0]); i++)
	{
		printf("  %02X %s"EOL, boardmap[i].type, boardmap[i].name);
	}

    set_sleep_mode(SLEEP_MODE_IDLE);
    for(;;)
    {
        cmdif_task();
    }
}

/* EOF */

