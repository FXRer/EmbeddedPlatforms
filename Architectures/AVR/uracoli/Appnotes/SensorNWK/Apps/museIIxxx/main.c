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

/*
 * sensorproto_museII_data_t: 31 Bytes
 *
 * ATmega128RFA1 Pagesize 256 Bytes
 *
 */

/*
 *
 *           | MuseIIRFA  | MuseII232
 * -----------------------------------------
 * ACC_INT1    PE5:INT5     PD2:PCINT17
 * ACC_INT2    PD3:INT3     PD1:INT0
 * COMP_DRDY   PD2:INT2     PD0:PCINT16
 *
 *
 *
 */

/* avr-libc */
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/* uracoli */
#include <ioutil.h>
#include <timer.h>
#include <rtc.h>

/* SensorProto */
#include <sensornwk.h>
#include <sensorproto.h>
#include <xyzstream.h>

/* SensorLib */
#include <i2c.h>

#include <bma250.h>
#include <hmc5883l.h>
#include <sht21.h>
#include <bmp085.h>

/* Project */
#include "flashstore.h"

/* === Macros ========================================================== */

/* === Types =========================================================== */
typedef enum
{
    APPMODE_NONE,
    APPMODE_SENSALL,
    APPMODE_ACCSTREAM, /* acceleration sensor streaming */
    APPMODE_MAGSTREAM, /* magnetic sensor streaming */
    APPMODE_PRESSURE_FLASHLOG,
} appmode_t;

typedef enum
{
    APPSTATE_IDLE,
    APPSTATE_MEAS,
    APPSTATE_TRX,
} appstate_t;

typedef enum
{
    ACCSTREAMMODE_NOFIFO,
    ACCSTREAMMODE_FIFO
} accstream_mode_t;

/* === Globals ========================================================= */
static sensorproto_museII_info_rep_t info_rep;
static sensorproto_museII_data_t result_data;

static volatile meastask_t meastasks;
static volatile appmode_t appmode;
static volatile appstate_t state;
static volatile appstate_t nextstate;

static volatile uint8_t adcfinished = 0;
static int8_t adc_offset = 0;

/* === Implementation ================================================== */

void irq_compdrdy_cb(void);


void rtc_cb_tick()
{
    if(APPMODE_SENSALL == appmode)
    {
        nextstate = APPSTATE_MEAS;
    }
}

ISR(ADC_vect, ISR_BLOCK)
{
    adcfinished = 1;
}

/*
 * \brief Trigger sleep based ADC measurement
 * Function is blocking until flag "adcfinished" is set by ISR
 *
 * @return ADC register value
 */
static inline int16_t adc_measure(void)
{
    set_sleep_mode(SLEEP_MODE_ADC);
    /* dummy cycle */
    adcfinished = 0;
    do
    {
        sleep_mode();
        /* sleep, wake up by ADC IRQ */

        /* check here for ADC IRQ because sleep mode could wake up from
         * another source too
         */
    }
    while (0 == adcfinished);   /* set by ISR */
    return ADC;
}

#if defined(museIIrfa)
/**
 * \brief Supply voltage measurement
 * Method: set 1.1V reference as input and AVCC as reference
 * 	this returns a negative result: AVCC = 1.1V - result
 *
 * \return The MCU internal voltage in [mV]
 *
 * \author Daniel Thiele
 */
int16_t measure_tmcu(void)
{
    int16_t val;

    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); /* PS 64 */
    ADCSRB = (1<<MUX5);

    ADMUX = (1<<REFS1) | (1<<REFS0) | (1<<MUX3) | (1<<MUX0); /* reference: 1.6V, input Temp Sensor */
    _delay_us(200); /* some time to settle */

    ADCSRA |= (1<<ADIF); /* clear flag */
    ADCSRA |= (1<<ADIE);

    /* dummy cycle after REF change (suggested by datasheet) */
    adc_measure();

    _delay_us(100); /* some time to settle */

    val = adc_measure();

    ADCSRA &= ~((1<<ADEN) | (1<<ADIE));

    return (val-adc_offset);
}
#endif /* defined(museIIrfa) */

int8_t measure_adc_offset(void)
{
    uint16_t val;

    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); /* PS 64 */
    ADCSRB = 0;
    ADMUX = (1<<REFS1) | (1<<REFS0) | (1<<MUX3); /* reference: 1.6V, differential ADC0-ADC0 10x */

    _delay_us(200); /* some time to settle */

    ADCSRA |= (1<<ADIF); /* clear flag */
    ADCSRA |= (1<<ADIE);

    /* dummy cycle after REF change (suggested by datasheet) */
    adc_measure();

    _delay_us(100); /* some time to settle */

    val = adc_measure();

    ADCSRA &= ~((1<<ADEN) | (1<<ADIE));

    return (val);
}


/*
 * \brief Execute measurements of all sensors and transmit
 *
 */
void meas(meastask_t tasks, sensorproto_museII_data_t *meas_result)
{
    meas_result->meas_valid &= ~tasks; /* clear out tasks */

    if (tasks & MEASTASK_VOLTAGE_AVR_bm)
    {
    	/* TODO */
        meas_result->meas_valid |= MEASTASK_VOLTAGE_AVR_bm;
    }

#if defined(museIIrfa)
    if(tasks & MEASTASK_TEMPERATURE_AVR_bm)
    {
        meas_result->tmcu = measure_tmcu();
        meas_result->meas_valid |= MEASTASK_TEMPERATURE_AVR_bm;
    }
#endif /* defined(museIIrfa) */

    if (tasks & MEASTASK_TEMPERATURE_SHT21_bm)
    {
        sht21_triggerMeas(STH21_CMD_TMEAS_NOHOLD);
        _delay_ms(100);
        sht21_readMeas(&meas_result->sht_t);
        meas_result->meas_valid |= MEASTASK_TEMPERATURE_SHT21_bm;
    }

    if (tasks & MEASTASK_HUMIDITY_SHT21_bm)
    {
        sht21_triggerMeas(STH21_CMD_RHMEAS_NOHOLD);
        _delay_ms(100);
        sht21_readMeas(&meas_result->sht_rh);
        meas_result->meas_valid |= MEASTASK_HUMIDITY_SHT21_bm;
    }

    if(tasks & MEASTASK_PRESSURE_BMP085_bm)
    {
        bmp085_up_trigger();
        _delay_ms(30);
        meas_result->up = bmp085_up_read();
        meas_result->meas_valid |= MEASTASK_PRESSURE_BMP085_bm;
    }

    if(tasks & MEASTASK_TEMPERATURE_BMP085_bm)
    {
        bmp085_ut_trigger();
        _delay_ms(30);
        meas_result->ut = bmp085_ut_read();
        meas_result->meas_valid |= MEASTASK_TEMPERATURE_BMP085_bm;
    }

    if(tasks & MEASTASK_ACCELERATION_BMA250_bm)
    {
        bma250_readresult(&meas_result->acc);
        meas_result->meas_valid |= MEASTASK_ACCELERATION_BMA250_bm;
    }

    if(tasks & MEASTASK_TEMPERATURE_BMA250_bm)
    {
        meas_result->bma_t = bma250_read_temperature();
        meas_result->meas_valid |= MEASTASK_TEMPERATURE_BMA250_bm;
    }

    if(tasks & MEASTASK_MAGNETIC_HMC5883_bm)
    {
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_B, 0x60);
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x10);
        hmc5883l_trigger();
        _delay_ms(50);
        hmc5883l_readresult(&meas_result->compass);
        meas_result->meas_valid |= MEASTASK_MAGNETIC_HMC5883_bm;
    }

    if (tasks & MEASTASK_LIGHT_LED_bm)
    {
    	/* TODO */
        meas_result->meas_valid |= MEASTASK_LIGHT_LED_bm;
    }
}

/*
 * \brief
 *
 * COMP_DRDY, ACC_INT1
 */
#if defined(museII232)
ISR(PCINT2_vect, ISR_BLOCK)
{
	/* empty */
}
#endif

/*
 * \brief
 *
 * COMP_DRDY
 *
 */
#if defined(museIIrfa)
ISR(INT2_vect, ISR_BLOCK)
{
    irq_compdrdy_cb();
}
#endif

/*
 * \brief
 *
 * ACC_INT1
 *
 */
#if defined(museIIrfa)
ISR(INT5_vect, ISR_BLOCK)
{
	/* empty */
}
#endif

/*
 * \brief
 *
 * ACC_INT2
 *
 */
#if defined(museIIrfa)
ISR(INT3_vect, ISR_BLOCK)
#elif defined(museII232)
ISR(INT0_vect, ISR_BLOCK)
#else
#error "Board not supported"
#endif
{
	/* empty */
}

void irq_compdrdy_cb(void)
{
    sensors_xyz_result_t res;

    hmc5883l_readresult(&res);
    xyzstream_put(SENSORNWK_FRAMETYPE_MUSEII_MAGSTREAM, &res);
    xyzstream_flush(SENSORNWK_FRAMETYPE_MUSEII_MAGSTREAM); /* only one sample */
}

/*
 * \brief Callback when a frame is received
 *
 */
uint8_t * cb_sensornwk_rx(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed)
{
    sensornwk_hdr_t *hdr = (sensornwk_hdr_t*) frm;
    sensorproto_museII_cfg_t *cfg =
        (sensorproto_museII_cfg_t*) &frm[sizeof(sensornwk_hdr_t)];

    switch (hdr->frametype)
    {
    case SENSORNWK_FRAMETYPE_MUSEII_CFG:
    	/* TODO: modify RTC */
        break;
    case SENSORNWK_FRAMETYPE_INFO_REQ:
        sensornwk_tx((uint8_t*) &info_rep,
                     sizeof(sensorproto_museII_info_rep_t),
                     SENSORNWK_FRAMETYPE_MUSEII_INFO_REP, 1);
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
    /* do nothing */
}


void irq_enable_compdrdy(void)
{
#if defined(museIIrfa) /* DRDY connects to PD2:INT2 */
    EIFR |= (1<<INT2); /* clear flag */
    EICRA |= (1<<ISC21) | (1<<ISC20); /* rising edge */
    EIMSK |= (1<<INT2);
#elif defined(museII232)

#else
#error "Board unknown"
#endif
}


/*
 * \brief Set application mode
 */
static inline void app_setmode(appmode_t mode)
{
    switch (mode)
    {
    case APPMODE_SENSALL:
    	/* TODO: setup RTC
        	wdt_reset();
        	wdt_timersetup(WDTO_8S);
        */
        meastasks = info_rep.sensor_available;

        appmode = APPMODE_SENSALL;
        break;

    case APPMODE_ACCSTREAM:
    	/* TODO: setup RTC
        	wdt_reset();
        */
        set_sleep_mode(SLEEP_MODE_IDLE);
        break;

    case APPMODE_MAGSTREAM:
    	/* TODO: setup RTC
        	wdt_reset();
        */
        set_sleep_mode(SLEEP_MODE_IDLE);

        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_MODE, 0x01); /* stop */
        /* write to data register to clear DRDY pin */
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_DATA_X, 0x00);

        irq_enable_compdrdy();

        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_B, 0x60);
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x08); /* 3Hz */
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_MODE, 0x00); /* continuous measurement mode */

        break;

    case APPMODE_PRESSURE_FLASHLOG:
    	/* TODO: setup RTC
			wdt_reset();
			wdt_timersetup(WDTO_2S);
        */

        meastasks = MEASTASK_TEMPERATURE_BMP085_bm | MEASTASK_PRESSURE_BMP085_bm;

        flashstore_enter(0x7000);
        appmode = APPMODE_PRESSURE_FLASHLOG;
        break;
    default:
        break;
    } /* switch(mode) */
}

void meas_printresult(sensorproto_museII_data_t *data)
{
    if(APPMODE_PRESSURE_FLASHLOG == appmode)
    {
        struct
        {
            uint32_t up;
            uint16_t ut;
            uint16_t foo;
        } chunk;
        chunk.up = data->up;
        chunk.ut = data->ut;
        chunk.foo = 0xB5;
        //flashstore_feed(&chunk, 32);
        nextstate = APPSTATE_IDLE;
    }
    else
    {
        sensornwk_tx((uint8_t*) data,
                     sizeof(sensorproto_museII_data_t),
                     SENSORNWK_FRAMETYPE_MUSEII_DATA, 1);
    }
}

/*
 * \brief Initialize sensors
 *
 */
void sensors_init(void)
{
    // info_rep.mcusr = mcusr_mirror;

    /* always available */
    info_rep.sensor_available |= MEASTASK_LIGHT_LED_bm;

#if defined(museIIrfa)
    info_rep.sensor_available |= MEASTASK_TEMPERATURE_AVR_bm;
#endif /* defined(museIIrfa) */

    /* RFA cannot measure its voltage due to 1.8V AVCC */
#if defined(museII232)
    info_rep.sensor_available  |= MEASTASK_VOLTAGE_AVR_bm;
#endif

    sht21_init(SENSORS_I2CADDR_SHT21);
    sht21_softreset();
    info_rep.sensor_available |= MEASTASK_HUMIDITY_SHT21_bm | MEASTASK_TEMPERATURE_SHT21_bm;

    bmp085_init(SENSORS_I2CADDR_BMP085);
    info_rep.sensor_available |= MEASTASK_PRESSURE_BMP085_bm | MEASTASK_TEMPERATURE_BMP085_bm;

    hmc5883l_init(SENSORS_I2CADDR_HMC5883);
    info_rep.sensor_available |= MEASTASK_MAGNETIC_HMC5883_bm;

    bma250_init(SENSORS_I2CADDR_BMA250);
    info_rep.sensor_available |= MEASTASK_ACCELERATION_BMA250_bm;
    info_rep.sensor_available |= MEASTASK_TEMPERATURE_BMA250_bm;

}

void sensors_calibrate(void)
{
    adc_offset = measure_adc_offset();

    sht21_identification((uint8_t*) &info_rep.sht_identification,
                         SHT21_IDENTIFICATION_WIDTH);

    bmp085_readee(&info_rep.bmpcal);

    /* HMC5883L calibration values
     * Self-test Measurement of HMC5883L is used, one structure for positive
     * bias (X=+1.16 Ga, Y=+1.16 Ga, Z=+1.08 Ga), one for negative bias
     * (X=-1.16 Ga, Y=-1.16 Ga, Z=-1.08 Ga)
     *
     */
    sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_B, HMC5883L_GAINCAL << 5);
    sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x11); /* positive bias */
    hmc5883l_trigger();
    _delay_ms(200);
    hmc5883l_readresult(&info_rep.hmc5883l_cal_positive);

    sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x12); /* negative bias */
    hmc5883l_trigger();
    _delay_ms(200);
    hmc5883l_readresult(&info_rep.hmc5883l_cal_negative);

    info_rep.bma250_sensitivity = 3.91; /* mg/LSB, reset value of BMA250 */
}

/*
 * \brief Main
 *
 */
int main(void)
{
	wdt_reset();
	wdt_disable();

    /* enable pullups for unused pins */
#if defined(museII232)
    DDRB &= ~(1<<PB0);
    PORTB |= (1<<PB0);

    DDRC &= ~(1<<PC3);
    PORTC |= (1<<PC3);

    DDRD &= ~((1<<PD7) | (1<<PD6) | (1<<PD5));
    PORTD |= (1<<PD7) | (1<<PD6) | (1<<PD5); /* PD5 in Rev1 */
#elif defined(museIIrfa)

    /* PORTB7:0 */
    DDRB &= ~0xFF;
    PORTB = 0xFF;

    /* PORTD7:4 */
    DDRD &= ~0xF0;
    PORTD = 0xF0;

    /* PORTE7:6 */
    DDRE &= ~0xC0;
    PORTE = 0xC0;

    /* PORTF3:0 */
    DDRF &= ~0x0F;
    PORTF |= 0x0F;

    /* PORTG5:0 */
    DDRG &= ~0x3F;
    PORTG |= 0x3F;

    DIDR0 = 0x0F; /* disable ADC3..0 pins */
#endif
    LED_INIT();
    i2c_init(400000);
    timer_init();

    rtc_init(rtc_cb_tick);
    LED_SET(0);
    _delay_ms(2000);
    LED_CLR(0);

    sensornwk_init(SENSORNWK_DEVTYPE_NODE, BOARD_TYPE);
    sensors_init();

    state = nextstate = APPSTATE_IDLE;
    sei();
    rtc_start();

    /* DEBUG */
    set_sleep_mode(SLEEP_MODE_IDLE);
    for(;;){
    	LED_SET(0);
    	_delay_ms(2);
    	LED_CLR(0);
    	sleep_mode();
    }
    /*********/

    sensors_calibrate();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sensornwk_tx((void*) 0x0000, 0, SENSORNWK_FRAMETYPE_HEREAMI, 1);
    state = nextstate = APPSTATE_TRX;

    while(APPSTATE_IDLE != nextstate);
    state = nextstate = APPSTATE_IDLE;

    sensornwk_tx((uint8_t*) &info_rep, sizeof(sensorproto_museII_info_rep_t), SENSORNWK_FRAMETYPE_MUSEII_INFO_REP, 1);
    state = nextstate = APPSTATE_TRX;

    while(APPSTATE_IDLE != nextstate);
    state = nextstate = APPSTATE_IDLE;

    app_setmode(APPMODE_SENSALL);
    //app_setmode(APPMODE_MAGSTREAM);
    //app_setmode(APPMODE_PRESSURE_FLASHLOG);

    for (;;)
    {
        if (nextstate != state)
        {
            state = nextstate;

            switch (nextstate)
            {
            case APPSTATE_IDLE:
                set_sleep_mode(SLEEP_MODE_PWR_SAVE);
                LED_CLR(0);
                break;
            case APPSTATE_MEAS:
                set_sleep_mode(SLEEP_MODE_IDLE);
                //meas(meastasks, &result_data);
                LED_SET(0);

                nextstate = APPSTATE_TRX;
                break;
            case APPSTATE_TRX:
                set_sleep_mode(SLEEP_MODE_IDLE);
                //sensornwk_tx((uint8_t*) &info_rep, sizeof(sensorproto_museII_info_rep_t), SENSORNWK_FRAMETYPE_MUSEII_INFO_REP, 1);
                _delay_ms(2);
                nextstate = APPSTATE_IDLE;
                //sensornwk_tx((uint8_t*)&result_data, sizeof(sensorproto_muse_data_t), SENSORNWK_FRAMETYPE_MUSE_DATA, 1);
                break;
            default:
                break;
            } /* switch(nextstate) */

        } else { /* if(nextstate != state) */
        	sleep_mode();
        }
    } /* for (;;) */

    /* this point is never reached */
}

/* EOF */
