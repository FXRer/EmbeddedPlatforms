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
#include "sensorapp.h"
#include "sensors.h"
#include "mma7455.h"
#include "sht21.h"

static volatile uint8_t adcfinished = 0;
static volatile uint8_t shtfinished = 0;
static volatile uint16_t sht_result;

#if 0
double measure_vcc(void)
{
  uint16_t adcval;

  /*
   * Turn off ADC right now, so the new reference selection can take
   * place.
   */
  ADCSRA = 0;
  /*
   * Reconfigure ADC to measure Vbg [internal bandgap],
   * using Vcc as reference.
   */
  ADMUX = /* !_BV(REFS1) | */ _BV(REFS0) | 0b11110;
  /*
   * Enable ADC, let the reference voltage settle, and start...
   */
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
  _delay_us(100);
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADPS2) | _BV(ADPS1);

  /*
   * ...and wait until it completed.
   */
  while ((ADCSRA & _BV(ADIF)) == 0)
  {
    /* wait */
  }

  /*
   * Fetch the conversion result, and re-enable the ADC for LCD
   * voltage tracking.
   */
  adcval = ADC;

  ADCSRA = 0;
  ADMUX = _BV(REFS1) /* | !_BV(REFS0) */ |      /* Internal 1.1V reference */
    _BV(ADLAR) |                                /* left-adjust result in ADCH */
    7;                                          /* Select channel ADC7 */
  /*
   * Changing the reference selection vom Vcc (3.5 ... 4 V) down to
   * the internal bandgap (~ 1.1 V) takes quite some time to discharge
   * the Varef bypass capacitor.
   */
  ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1);
  _delay_ms(1);
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1);

  while ((ADCSRA & _BV(ADIF)) == 0)
  {
    /* wait */
  }

  #if 0
   ADCSRA = 0;   
   ADMUX = 0;
    #endif

  /*
   * Calculate Vcc
   */
  #define VAREF (1.1)
  return VAREF * 1023.0 / (double)adcval;
}
#endif

#if defined(muse231) || defined(tiny230) || defined(lgee231)
ISR(ADC_vect)
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
	do{
		sleep_mode();
		/* sleep, wake up by ADC IRQ */

		/* check here for ADC IRQ because sleep mode could wake up from
		 * another source too
		 */
	}while (0 == adcfinished);	/* set by ISR */
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

	PRR &= ~_BV(PRADC);
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);	/* PS 64 */

#if defined(muse231) || defined(lgee231)
	ADMUX  = _BV(REFS0) | (0x0E); /* reference: AVCC, input Bandgap 1.1V */
#elif defined(tiny230)
	ADMUX = 0b100001;  /* reference: AVCC, input Bandgap 1.1V */
#endif
	_delay_us(200);			/* some time to settle */

	ADCSRA |= _BV(ADIF);		/* clear flag */
	ADCSRA |= _BV(ADIE);

	/* dummy cycle after REF change (suggested by datasheet) */
	adc_measure();

	_delay_us(100);			/* some time to settle */

	val = adc_measure();

	ADCSRA &= ~(_BV(ADEN) | _BV(ADIE));
	PRR |= _BV(PRADC);

	return ( (1100UL*1023UL) / val );
}
#endif /* defined(muse231 or tiny230) || defined(lgee231) */

#if defined(tiny230)
uint16_t measure_t_kty(void)
{
	uint16_t val;

	/* enable KTY13, PA3=>0 + wait */
	DDRA |= _BV(PA3);
	PORTA |= _BV(PA3);
	_delay_us(200);
	
	PRR &= ~_BV(PRADC);
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);	/* PS 64 */

	ADMUX = 0b000010;  /* reference: AVCC, input ADC2 */
	
	_delay_us(200);			/* some time to settle */

	ADCSRA |= _BV(ADIF);		/* clear flag */
	ADCSRA |= _BV(ADIE);

	/* dummy cycle after REF change (suggested by datasheet) */
	adc_measure();

	_delay_us(100);			/* some time to settle */

	val = adc_measure();

	ADCSRA &= ~(_BV(ADEN) | _BV(ADIE));
	PRR |= _BV(PRADC);
	/* turn off KTY13 */
	DDRA &= ~_BV(PA3);
	PORTA &= ~_BV(PA3);
	return ( val );
}
#endif /* defined(tiny230) */

#if defined(muse231)
/*
 * \brief LED used as light sensor
 */
uint16_t measure_light(void)
{
	uint16_t i;

	/* both output */
	LED_DDR |= _BV(LED_ANODE_bp) | _BV(LED_CATHODE_bp);

	/* reverse charge */
	LED_PORT &= ~_BV(LED_ANODE_bp);
	LED_PORT |= _BV(LED_CATHODE_bp);

	_delay_us(10);
	/* charge time, t.b.d. */

	LED_DDR &= ~_BV(LED_CATHODE_bp);	/* cathode as input */
	LED_PORT &= ~_BV(LED_CATHODE_bp);	/* immediately switch off pullup */

	i=0x4FFF;
	while( (LED_PIN & _BV(LED_CATHODE_bp)) && --i);

	/* both output */
	LED_DDR |= _BV(LED_ANODE_bp) | _BV(LED_CATHODE_bp);

	return i;
}

#endif /* defined(muse231) */

#if HAS_SHT == 1
/*
 * \brief Callback function for polling of SHT is finished
 *
 * @param arg Optional arguments to give (not used)
 * @return 0 when result is valid (do not restart timer), 1 else (continue polling)
 */
time_t sht_timer(timer_arg_t arg)
{
	shtfinished = sht21_readMeas((uint16_t*)&sht_result);
	if(shtfinished){
		return 0;	/* stop timer */
	}else{
		return 1;	/* restart timer */
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
	shtfinished = 0;
	sht21_triggerMeas(cmd);

	/* do not store the timer handle, returned by this function
	 * timer is stopped inside polling (callback "sht_timer") and nowhere else
	 */
	timer_start(sht_timer, MSEC(20), 0);	/* poll every 20ms */

	set_sleep_mode(SLEEP_MODE_IDLE);	/* to keep Timer1 running */
	do{
		sleep_mode();
	}while(0 == shtfinished);

	/* TODO scale to relative humidity [%] according to datasheet */

	return sht_result;
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
#endif


#if HAS_MMA == 1
/**
  * \brief Measure acceleration
  * All 3 axes are measured simultaneously
  * 
  * \param Pointer to structure where results are written
  */
void measure_acceleration(mma7455_result_t *acc)
{
	mma7455_regwr(0x16, ((ACC_SENSITIVITY << 2) & 0x0C)| 0x01);	/* measurement mode */

#if 0	/* sleep mode based implementation */
	MMA7455_INT1_EN();
	set_sleep_mode( SLEEP_MODE_IDLE);
	sleep_mode();
	/* sleeping, wake up by ACC_INT1 */

#else	/* polling based implementation */
	while(!((ACC_IRQ_PIN & _BV(ACC_IRQ_bp))));
#endif

	uint8_t buf[6];
	mma7455_regrd(0x00, 6, buf);

	acc->x = (int16_t)((buf[1] << 14) | (buf[0] << 6)) / 64;
	acc->y = (int16_t)((buf[3] << 14) | (buf[2] << 6)) / 64;
	acc->z = (int16_t)((buf[5] << 14) | (buf[4] << 6)) / 64;

	mma7455_regwr(0x16, 0x00);	/* standby mode */
	MMA7455_INT1_DIS();
}
#endif /* HAS_MMA == 1*/

#if ! defined(muse231) && !defined(tiny231)
float scale_sht_temperature(uint16_t raw)
{
	return 175.72*(float)raw/65536-46.85;
}

float scale_sht_humidity(uint16_t raw)
{
	return 125*(float)raw/65536-6;
}

inline float scale_acc(int16_t raw)
{
	const uint8_t sensitivity = 64; /* 2g-range, 64LSB/g */
	return (float)raw/sensitivity;
}
#endif /*! defined(muse231)*/

/* EOF */
