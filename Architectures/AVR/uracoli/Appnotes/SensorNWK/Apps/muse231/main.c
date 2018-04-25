/* Copyright (c) 2011 Daniel Thiele
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

/* avr-libc */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/* uracoli */
#include <board.h>
#include <ioutil.h>
#include <p2p_protocol.h>

/* SensorProto */
#include <sensornwk.h>
#include <sensorproto.h>
#include <xyzstream.h>

/* SensorLib */
#include <i2c.h>
#include <mma7455.h>
#include <adxl345.h>

/* Project */
#include "measure.h"

/* === Macros ========================================================== */

//#define PROTO_P2P (1)

/* === Types =========================================================== */
typedef enum
{
    APPMODE_NONE, APPMODE_SLOWSENS, APPMODE_ACCSTREAM,
} appmode_t;

typedef enum
{
    APPSTATE_IDLE,
    APPSTATE_MEAS,
    APPSTATE_TRX,
} appstate_t;

typedef enum
{
    ACCSTREAMMODE_NOFIFO, ACCSTREAMMODE_FIFO
} accstream_mode_t;

typedef struct
{
    accstream_mode_t mode;
    uint8_t ratecode;
    uint32_t nbsamples;
} accstream_cfg_t;

/* === Globals ========================================================= */
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

static sensorproto_muse_info_rep_t info_rep;
static sensorproto_muse_data_t result_data;
#if defined(PROTO_P2P)
static p2p_sensor_data_t p2p_sensor_data;
#endif

static volatile bool associated = false;

static volatile accstream_cfg_t accstream_cfg;

static volatile appmode_t appmode;
static volatile appmode_t nextmode;

static volatile meastask_t meastasks;
static volatile appstate_t state;
static volatile appstate_t nextstate;

/* === Implementation ================================================== */

static inline void app_setmode(appmode_t mode);

/*
 * \brief Execute measurements of all sensors and transmit
 *
 */
void meas(meastask_t tasks, sensorproto_muse_data_t *meas_result)
{
    meas_result->meas_valid &= ~tasks; /* clear out tasks */

    if (tasks & MEASTASK_VOLTAGE_AVR_bm)
    {
        meas_result->vmcu = measure_vmcu();
        meas_result->meas_valid |= MEASTASK_VOLTAGE_AVR_bm;
    }

    if (tasks & MEASTASK_TEMPERATURE_SHT21_bm)
    {
        meas_result->sht_t = measure_sht_temperature();
        meas_result->meas_valid |= MEASTASK_TEMPERATURE_SHT21_bm;
    }

    if (tasks & MEASTASK_HUMIDITY_SHT21_bm)
    {
        meas_result->sht_rh = measure_sht_humidity();
        meas_result->meas_valid |= MEASTASK_HUMIDITY_SHT21_bm;
    }

    if (tasks & MEASTASK_ACCELERATION_MMA7455_bm)
    {
        measure_acceleration((sensors_xyz_result_t*) &meas_result->acc, 1);
        meas_result->meas_valid |= MEASTASK_ACCELERATION_MMA7455_bm;
    }

    if (tasks & MEASTASK_ACCELERATION_ADXL345_bm)
    {
        measure_acceleration((sensors_xyz_result_t*) &meas_result->acc, 0);
        meas_result->meas_valid |= MEASTASK_ACCELERATION_ADXL345_bm;
    }

    if (tasks & MEASTASK_LIGHT_LED_bm)
    {
        meas_result->light = measure_light();
        meas_result->meas_valid |= MEASTASK_LIGHT_LED_bm;
    }
}

/*
 * \brief Setup watchdog to serve as application timer
 *
 */
static inline void wdt_timersetup(uint8_t timeout)
{
    WDTCSR = (1 << WDCE) | (1 << WDE); /* Enable configuration change */
    WDTCSR = (1 << WDIF) | (1 << WDIE) | /* Enable Watchdog Interrupt Mode */
             (timeout & 0x08 ? _WD_PS3_MASK : 0x00) | (timeout & 0x07);
}

/*
 * \brief Watchdog ISR, used as application timer
 *
 */
ISR( WDT_vect, ISR_NOBLOCK)
{
    nextstate = APPSTATE_MEAS;
}

/*
 * \brief Callback when a frame is received
 *
 */
uint8_t * cb_sensornwk_rx(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed)
{
    sensornwk_hdr_t *hdr = (sensornwk_hdr_t*) frm;
    sensorproto_muse_cfg_t *cfg =
        (sensorproto_muse_cfg_t*) &frm[sizeof(sensornwk_hdr_t)];

    switch (hdr->frametype)
    {
    case SENSORNWK_FRAMETYPE_MUSE_CFG:
        if (1 == cfg->appmode)
        {
            switch (cfg->accstream_rate_hz)
            {
            case 50:
                accstream_cfg.ratecode = 0x09;
                break;
            case 100:
                accstream_cfg.ratecode = 0x0A;
                break;
            case 200:
                accstream_cfg.ratecode = 0x0B;
                break;
            case 400:
                accstream_cfg.ratecode = 0x0C;
                break;
            default:
                accstream_cfg.ratecode = 0x08; /* 25 Hz */
                break;
            } /* switch(cfg->accstream_rate_hz) */
            accstream_cfg.nbsamples = cfg->accstream_nbsamples;
            accstream_cfg.mode = ACCSTREAMMODE_FIFO;
            nextmode = APPMODE_ACCSTREAM;
        }
        else
        {
            wdt_timersetup(cfg->wdto);
            nextmode = APPMODE_SLOWSENS;
        }
        break;
    case SENSORNWK_FRAMETYPE_INFO_REQ:
        sensornwk_tx((uint8_t*) &info_rep,
                     sizeof(sensorproto_muse_info_rep_t),
                     SENSORNWK_FRAMETYPE_MUSE_INFO_REP, 1);
        break;
    case SENSORNWK_FRAMETYPE_JBOOTL:
        EIMSK &= ~(1 << INT0);
        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_POWER_CTL, 0x00); /* standby */
        jump_to_bootloader();
        break;
    default:
        break;
    } /* switch(hdr->frametype) */

    return frm;
}

/*
 * \brief Callback when frame has been transmitted
 *
 */
void cb_sensornwk_done(void)
{
    nextstate = APPSTATE_IDLE;
}

void cb_sensornwk_association_cnf(uint16_t addr)
{
    associated = true;
    nextstate = APPSTATE_IDLE;
}

/*
 * \brief Handler for ACC_INT1 interrupt connected to
 * AVR INT0 external interrupt pin
 */
static inline void acc_irq_handler(void)
{
    uint8_t nbsamples;
    sensors_xyz_result_t res;

    if (ACCSTREAMMODE_FIFO == accstream_cfg.mode)
    {
        nbsamples = sensors_regrd(SENSORS_I2CADDR_ADXL345,
                                  RG_ADXL345_FIFO_STATUS);
    }
    else if (ACCSTREAMMODE_NOFIFO == accstream_cfg.mode)
    {
        nbsamples = 1;
    }
    else
    {
        /* not handled */
        nbsamples = 0;
    }

    do
    {
        adxl345_readresult(&res);
        xyzstream_put(SENSORNWK_FRAMETYPE_MUSE_ACCSTREAM, &res);
    }
    while (--nbsamples && --accstream_cfg.nbsamples);

    /* accstream_cfg.nbsamples became zero before nbsamples became zero,
     * send the samples collected so far
     */
    if (nbsamples > 0)
    {
        xyzstream_flush(SENSORNWK_FRAMETYPE_MUSE_ACCSTREAM);
    }

    if (0 == accstream_cfg.nbsamples)
    {
        nextmode = APPMODE_SLOWSENS;
    }
}

/*
 * \brief Interrupt Service Routine INT0
 */
ISR(INT0_vect, ISR_BLOCK)
{
    if (accstream_cfg.nbsamples > 0)
    {
        acc_irq_handler();
    }
}

/*
 * \brief Set application mode
 */
static inline void app_setmode(appmode_t mode)
{
    switch (mode)
    {
    case APPMODE_SLOWSENS:
        EIMSK &= ~(1 << INT0);

        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_ENABLE, 0x00); /* all off */
        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_POWER_CTL, 0x00); /* standby */

        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_ENABLE, 0x80); /* Data ready */
        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_MAP, 0x00); /* all to INT1 pin */
        sensors_regrd(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_SOURCE); /* clear IRQ flags */

        wdt_reset();
        wdt_timersetup(WDTO_8S);
        //meastasks = info_rep.sensor_available;
        meastasks = MEASTASK_HUMIDITY_SHT21_bm | MEASTASK_TEMPERATURE_SHT21_bm
                    | MEASTASK_VOLTAGE_AVR_bm;
        break;
    case APPMODE_ACCSTREAM:
        wdt_reset();
        wdt_disable();

        EIMSK &= ~(1 << INT0);

        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_ENABLE, 0x00); /* all off */
        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_POWER_CTL, 0x00); /* standby */
        sensors_regrd(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_SOURCE); /* clear IRQ flags */

        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_BW_RATE,
                      accstream_cfg.ratecode);
        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_MAP, 0x00); /* all to INT1 pin */

        EICRA = (1 << ISC01) | (1 << ISC00); /* rising edge */
        EIFR |= (1 << INT0); /* clear flag */
        EIMSK |= (1 << INT0);

        if (ACCSTREAMMODE_FIFO == accstream_cfg.mode)
        {
            sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_FIFO_CTL,
                          0x80 | (SENSORPROTO_GENERIC_XYZSTREAM_NBSAMPLES & 0x1F)); /* stream mode, number samples */
            sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_ENABLE, 0x02); /* Watermark */
        }
        else if (ACCSTREAMMODE_NOFIFO == accstream_cfg.mode)
        {
            sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_INT_ENABLE, 0x80); /* Data ready */
        }
        else
        {
            /* not handled */
        }

        sensors_regwr(SENSORS_I2CADDR_ADXL345, RG_ADXL345_POWER_CTL, 0x08); /* measure mode */

        //meastasks = MEASTASK_ACCELERATION_ADXL345_bm;
        break;
    default:
        break;
    } /* switch(mode) */
}

/*
 * \brief Save MCUSR to variable and disable watchdog
 * This must be done very early after reset, placing to .init3 section is done
 * in the forward declaration above
 *
 */
void __attribute__((naked)) __attribute__((section(".init3"))) get_mcusr(void)
{
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

/*
 * \brief Initialize sensors
 *
 */
void sensors_init(void)
{
    info_rep.mcusr = mcusr_mirror;
    info_rep.sensor_available = MEASTASK_LIGHT_LED_bm | MEASTASK_VOLTAGE_AVR_bm;

    sht21_init(SENSORS_I2CADDR_SHT21);
    sht21_identification((uint8_t*) &info_rep.sht_identification,
                         SHT21_IDENTIFICATION_WIDTH);
    info_rep.sensor_available |= MEASTASK_HUMIDITY_SHT21_bm
                                 | MEASTASK_TEMPERATURE_SHT21_bm;

    mma7455_init(SENSORS_I2CADDR_MMA7455);
    adxl345_init(SENSORS_I2CADDR_ADXL345);

    if (sensors_regrd(SENSORS_I2CADDR_ADXL345, RG_ADXL345_DEVID) != 0)
    {
        info_rep.sensor_available |= MEASTASK_ACCELERATION_ADXL345_bm;
    }
    else
    {
        info_rep.sensor_available |= MEASTASK_ACCELERATION_MMA7455_bm;
    }
}

static inline void sensors_calibrate(void)
{
}


/*
 * \brief Main
 *
 */
int main(void)
{
    LED_INIT();
    LED_SET(0);
    _delay_ms(20);
    LED_CLR(0);

    i2c_init(400000);
    sensornwk_init(SENSORNWK_DEVTYPE_NODE, BOARD_MUSE231);
    sensors_init();

    state = nextstate = APPSTATE_IDLE;
    sei();

    sensors_calibrate();

    set_sleep_mode(SLEEP_MODE_IDLE);

    do
    {
        sensornwk_tx_association_req();
        state = nextstate = APPSTATE_TRX;

        while (APPSTATE_IDLE != nextstate)
            ;
        state = nextstate = APPSTATE_IDLE;

        _delay_ms(1000);
        LED_SET(0);
        _delay_ms(50);
        LED_CLR(0);
        _delay_ms(50);
        LED_SET(0);
        _delay_ms(50);
        LED_CLR(0);

    }
    while(associated == false);

#if defined(PROTO_P2P)
    /* do nothing */
#else
    sensornwk_tx((uint8_t*) &info_rep, sizeof(sensorproto_muse_info_rep_t),
                 SENSORNWK_FRAMETYPE_MUSE_INFO_REP, 1);
#endif
    state = APPSTATE_TRX;

    app_setmode(APPMODE_SLOWSENS);
    appmode = nextmode = APPMODE_SLOWSENS;

    for (;;)
    {
        if (nextstate != state)
        {

            switch (nextstate)
            {
            case APPSTATE_IDLE:
                if (nextmode != appmode)
                {
                    app_setmode(nextmode);
                    appmode = nextmode;
                }

                switch (appmode)
                {
                case APPMODE_ACCSTREAM:
                    set_sleep_mode(SLEEP_MODE_IDLE);
                    break;
                default:
                    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                    break;
                } /* switch(appmode) */

                LED_CLR(0);
                break;
            case APPSTATE_MEAS:
                set_sleep_mode(SLEEP_MODE_IDLE);
                meas(meastasks, &result_data);
                LED_SET(0);

#if defined(PROTO_P2P)
                uint8_t size;
				size=sizeof(p2p_sensor_data_t);

				p2p_sensor_data.data[size] = SENSOR_MCU_VTG;
				size += sizeof(sensor_const_t);
				*((uint16_t*)&p2p_sensor_data.data[size]) = result_data.vmcu;
				size += sizeof(uint16_t);

				p2p_sensor_data.data[size] = SENSOR_SHT21_T;
				size += sizeof(sensor_const_t);
				*((uint16_t*)&p2p_sensor_data.data[size]) = result_data.sht_t;
				size += sizeof(uint16_t);

				p2p_sensor_data.data[size] = SENSOR_SHT21_RH;
				size += sizeof(sensor_const_t);
				*((uint16_t*)&p2p_sensor_data.data[size]) = result_data.sht_rh;
				size += sizeof(uint16_t);

                sensornwk_tx((uint8_t*) &p2p_sensor_data, size, P2P_SENSOR_DATA, 1);
#else
                sensornwk_tx((uint8_t*) &result_data,
                             sizeof(sensorproto_muse_data_t),
                             SENSORNWK_FRAMETYPE_MUSE_DATA, 1);
#endif
                break;
            case APPSTATE_TRX:
                set_sleep_mode(SLEEP_MODE_IDLE);
                break;
            default:
                break;
            } /* switch(nextstate) */

            state = nextstate;

        } /* if(nextstate != state) */

        cli();
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
    } /* for (;;) */

    /* this point is never reached */
}

/* EOF */
