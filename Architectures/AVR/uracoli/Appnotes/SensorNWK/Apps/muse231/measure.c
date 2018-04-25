/* Copyright (c) 2011 Joerg WUnsch, Daniel Thiele
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
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include <board.h>

#include "sensors.h"
#include "sht21.h"
#include <mma7455.h>
#include <adxl345.h>

#include "ost.h"
#include "measure.h"

#define SHT_TIMER_US (20000)

static volatile uint8_t adcfinished = 0;
static volatile uint8_t shtfinished = 0;
static volatile uint16_t sht_result;
static volatile uint8_t sht_retries;

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
static inline uint16_t adc_measure(void)
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

/**
 * \brief Supply voltage measurement
 * Method: set 1.1V reference as input and AVCC as reference
 * 	this returns a negative result: AVCC = 1.1V - result
 *
 * \return The MCU internal voltage in [mV]
 *
 * \author Daniel Thiele
 */
uint16_t measure_vmcu(void)
{
    uint16_t val;

    PRR &= ~(1<<PRADC);
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); /* PS 64 */

    ADMUX = (1<<REFS0) | (0x0E); /* reference: AVCC, input Bandgap 1.1V */
    _delay_us(200); /* some time to settle */

    ADCSRA |= (1<<ADIF); /* clear flag */
    ADCSRA |= (1<<ADIE);

    /* dummy cycle after REF change (suggested by datasheet) */
    adc_measure();

    _delay_us(100); /* some time to settle */

    val = adc_measure();

    ADCSRA &= ~((1<<ADEN) | (1<<ADIE));
    PRR |= (1<<PRADC);

    return ((1100UL * 1023UL) / val);
}

/*
 * \brief LED used as light sensor
 */
uint16_t measure_light(void)
{
    uint32_t t_us = 0;

    PRR &= ~(1<<PRTIM0);

    TCCR0A = 0;
    TCNT0 = 0;
    TIMSK0 = 0; /* no IRQs */
    TIFR0 |= (1<<TOV0); /* clear all IRQ flags */

    /* both output */
    LED_DDR |= (1<<LED_ANODE_bp) | (1<<LED_CATHODE_bp);

    /* reverse charge */
    LED_PORT &= ~(1<<LED_ANODE_bp);
    LED_PORT |= (1<<LED_CATHODE_bp);

    _delay_us(10);
    /* charge time, t.b.d. */

    LED_DDR &= ~(1<<LED_CATHODE_bp); /* cathode as input */
    LED_PORT &= ~(1<<LED_CATHODE_bp); /* immediately switch off pullup */

    TCCR0B = (1<<CS01); /* 1MHz */

    while (LED_PIN & (1<<LED_CATHODE_bp))
    {
        if (TIFR0 & (1<<TOV0))
        {
            t_us += 256;
            TIFR0 |= (1<<TOV0);

            if (t_us > 1000000)
            {
                break;
            }
        }
    }
    TCCR0B = 0;

    t_us += TCNT0;

    /* both output */
    LED_DDR |= (1<<LED_ANODE_bp) | (1<<LED_CATHODE_bp);

    return t_us;
}

/*
 * \brief Callback function for polling of SHT is finished
 *
 * @param arg Optional arguments to give (not used)
 * @return 0 when result is valid (do not restart timer), 1 else (continue polling)
 */
void sht_timer(void)
{
    shtfinished = sht21_readMeas((uint16_t*) &sht_result);

    if (shtfinished)
    {
        ost_stop();
    }
    else
    {
        /* keep running (periodically) */
    }
}

/**
 * \brief Measure SHT21 Humidity
 *
 * \return The humidity/temperature unscaled
 *
 * \author Daniel Thiele
 */
static inline uint16_t measure_sht(uint8_t cmd)
{
    sht21_triggerMeas(cmd);

    ost_start(sht_timer, SHT_TIMER_US, OST_PERIODIC); /* poll every 20ms */

    set_sleep_mode(SLEEP_MODE_IDLE);
    sht_retries = 10;
    shtfinished = 0;
    do
    {
        sleep_mode();
    }
    while(0 == shtfinished);

    return (sht_retries > 0)?sht_result:0;
}

/*
 * \brief Wrapper to start SHT temperature measurement
 *
 * @return The raw SHT result value
 */
uint16_t measure_sht_temperature(void)
{
    return measure_sht(STH21_CMD_TMEAS_NOHOLD);
}

/*
 * \brief Wrapper to start SHT humidity measurement
 *
 * @return The raw SHT result value
 */
uint16_t measure_sht_humidity(void)
{
    return measure_sht(STH21_CMD_RHMEAS_NOHOLD);
}

/**
 * \brief Measure acceleration
 * All 3 axes are measured simultaneously
 *
 * \param Pointer to structure where results are written
 */
void measure_acceleration(sensors_xyz_result_t *acc, uint8_t type)
{
    volatile uint16_t timeout = 0xFFFF;

    void (*trigger)(void) = adxl345_trigger;
    void (*read)(sensors_xyz_result_t*) = adxl345_readresult;
    void (*sleep)(void) = adxl345_powerdown;

    if(1 == type)  /* MMA7455, otherwise ADXL345 */
    {
        trigger = mma7455_trigger;
        read = mma7455_readresult;
        sleep = mma7455_powerdown;
    }

    trigger();

    while (!((ACC_IRQ_PIN & (1<<ACC_IRQ_bp))) && --timeout)
        ;

    if(timeout)
    {
        read(acc);
    }
    else
    {
        acc->x = -1234;
        acc->y = -1234;
        acc->z = -1234;
    }

    sleep();
}

/* EOF */
