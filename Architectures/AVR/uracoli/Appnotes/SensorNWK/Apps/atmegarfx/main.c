/* Copyright (c) 2011 - 2012 Daniel Thiele, Axel Wachtler
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
 *
 * @brief
 * @ingroup
 */


/* === includes ============================================================ */

/* avr-libc inclusions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>

/* uracoli inclusions */
#include <board.h>
#include <ioutil.h>
#include <transceiver.h>
#include <rtc.h>

/* SensorProto */
#include <sensornwk.h>
#include <sensorproto.h>

/* === macros ============================================================== */


/* === types =============================================================== */

typedef enum
{
    APPSTATE_IDLE, APPSTATE_MEAS, APPSTATE_TRX,
} appstate_t;

/* === globals ============================================================= */

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

static volatile uint8_t adcfinished = 0;
static int8_t adc_offset = 0;

static volatile appstate_t appstate;
static volatile appstate_t nextstate;

static volatile bool associated = false;

static sensorproto_radiofaro_data_t result_data;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

static void rtc_tick(void)
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

    switch (hdr->frametype)
    {
    case SENSORNWK_FRAMETYPE_INFO_REQ:
        break;
    case SENSORNWK_FRAMETYPE_JBOOTL:
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
    return ADC ;
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
float measure_tmcu(void)
{
    int32_t val = 0;
    uint8_t nbavg = 5;

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
    ADCSRB = (1 << MUX5);

    ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX0); /* reference: 1.6V, input Temp Sensor */
    _delay_us(200); /* some time to settle */

    ADCSRA |= (1 << ADIF); /* clear flag */
    ADCSRA |= (1 << ADIE);

    /* dummy cycle after REF change (suggested by datasheet) */
    adc_measure();

    _delay_us(100); /* some time to settle */

    for(uint8_t i=0; i<nbavg; i++)
    {
        val += adc_measure() - adc_offset;
    }

    ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

    return (1.13 * val / (float)nbavg - 272.8);
}

int8_t measure_adc_offset(void)
{
    uint16_t val;

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
    ADCSRB = 0;
    ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3); /* reference: 1.6V, differential ADC0-ADC0 10x */

    _delay_us(200); /* some time to settle */

    ADCSRA |= (1 << ADIF); /* clear flag */
    ADCSRA |= (1 << ADIE);

    /* dummy cycle after REF change (suggested by datasheet) */
    adc_measure();

    _delay_us(100); /* some time to settle */

    val = adc_measure();

    ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

    return (val);
}

/*
 * \brief Measure internal voltage of ATmega128RFA1
 *
 * Cannot be measured via ADC, use Batmon of TRX part
 *
 */
uint16_t measure_vmcu(void)
{
    uint16_t v; /* voltage in [mV] */
    uint8_t vth;

    for(vth=0; vth<32; vth++)
    {
        trx_reg_write(RG_BATMON, vth & 0x1F);

        if(0 == trx_bit_read(SR_BATMON_OK))
        {
            break;
        }
    }

    if(vth & 0x10)
    {
        v = 2550 + 75*(vth&0x0F); /* high range */
    }
    else
    {
        v = 1700 + 50*(vth&0x0F); /* low range */
    }

    return v;
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

int main()
{
    LED_INIT();
    rtc_init(rtc_tick);

    sensornwk_init(SENSORNWK_DEVTYPE_NODE, BOARD_PINOCCIO);

    /* Welcome Blink */
    LED_SET(0);
    LED_SET(1);
    _delay_ms(20);
    LED_CLR(0);
    LED_CLR(1);

    result_data.meas_valid = MEASTASK_TEMPERATURE_AVR_bm | MEASTASK_VOLTAGE_AVR_bm;

    sei();

    rtc_start();

    do
    {
        sensornwk_tx_association_req();
        appstate = nextstate = APPSTATE_TRX;

        while (APPSTATE_IDLE != nextstate)
            ;
        appstate = nextstate = APPSTATE_IDLE;

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

    adc_offset = measure_adc_offset();

    set_sleep_mode(SLEEP_MODE_IDLE);
    appstate = nextstate = APPSTATE_TRX;

    for (;;)
    {

        if (appstate != nextstate)
        {
            switch (nextstate)
            {
            case APPSTATE_IDLE:
                //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                set_sleep_mode(SLEEP_MODE_IDLE);
                break;
            case APPSTATE_MEAS:
                LED_SET(0);
                DELAY_MS(5);
                LED_CLR(0);

                result_data.tmcu = measure_tmcu();
                result_data.vmcu = measure_vmcu();

                nextstate = APPSTATE_TRX;

                /* Fake,
                 *
                 * fall through
                 */
            case APPSTATE_TRX:
                set_sleep_mode(SLEEP_MODE_IDLE);

                sensornwk_tx((uint8_t*)&result_data, sizeof(sensorproto_radiofaro_data_t), SENSORNWK_FRAMETYPE_RADIOFARO_DATA, 1);

                break;
            }
            appstate = nextstate;
        }

        sleep_mode();
    } /* for(;;); */

    /* never reaches this point */

}
/* EOF */
