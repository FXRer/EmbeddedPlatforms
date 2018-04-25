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

/* $Id$ */
/**
 * @file
 * @brief ....
 * @_addtogroup grpApp...
 *
 * ADC channel assignment
 *  ADC0 : Microphone
 *  ADC1 : Joystick
 *  ADC2 : External supply voltage
 *  ADC3 : MCU voltage
 *  ADC4 : Temperature sensor
 *
 *
 * Temperature Sensor: Murata ncp18wf104j03rb
 *
 */

/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>	

#include "analog.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

#if 0 /* not used yet */
/*
 * sinus table, 8-bit
 *
 *
 * >>> [int(127*math.sin(math.radians(90.0/255*i))) for i in range(0,256)]
 */
static uint8_t sine_table[255] = { 0, 0, 1, 2, 3, 3, 4, 5, 6, 7, 7, 8, 9, 10,
		10, 11, 12, 13, 14, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24,
		24, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 33, 34, 34, 35, 36, 37, 37,
		38, 39, 39, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 48, 48, 49, 50, 50,
		51, 52, 53, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 60, 61, 62, 62, 63,
		64, 64, 65, 66, 66, 67, 68, 68, 69, 70, 70, 71, 72, 72, 73, 74, 74, 75,
		75, 76, 77, 77, 78, 79, 79, 80, 80, 81, 82, 82, 83, 83, 84, 84, 85, 86,
		86, 87, 87, 88, 88, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94, 95, 95,
		96, 96, 97, 97, 98, 98, 99, 99, 100, 100, 101, 101, 102, 102, 103, 103,
		104, 104, 104, 105, 105, 106, 106, 107, 107, 107, 108, 108, 109, 109,
		109, 110, 110, 111, 111, 111, 112, 112, 112, 113, 113, 114, 114, 114,
		115, 115, 115, 116, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118,
		119, 119, 119, 120, 120, 120, 120, 121, 121, 121, 121, 121, 122, 122,
		122, 122, 122, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124,
		124, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126,
		126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126,
		126 };
static uint8_t sine_idx;
#endif

/* === prototypes ========================================================== */

/* === functions =========================================================== */

ISR(TIMER1_OVF_vect, ISR_BLOCK)
{
}

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
}

ISR(TIMER1_COMPB_vect, ISR_BLOCK)
{
}

ISR(TIMER0_OVF_vect, ISR_BLOCK)
{

}

ISR(TIMER0_COMP_vect, ISR_BLOCK)
{
	/* triggering ADC
	 *
	 * ISR would not be necessary since ADC is triggered even if OCIE1A is disabled,
	 * but clearing the flag is executed automatically if ISR is executed, we make
	 * use of this
	 *
	 */
	PORTE ^= (1 << PE3); /* UserIO 2 */
}

ISR(ADC_vect, ISR_BLOCK)
{
	if ((ADMUX & 0x1F) == 1) {
		joystick_sample_cb(ADCH);
	} else if((ADMUX & 0x1F) == 0) {
		microphone_sample_cb(ADCH);		
	} else if((ADMUX & 0x1F) == 4) {
		PORTF |= (1<<PF6); /* power down sensor */
		temperature_sample_cb(ADC);
	}
}

void audio_powerup(void)
{
	PORTE &= ~(1 << PE7);
}

void audio_powerdown(void)
{
	PORTE |= (1 << PE7);
}

void audio_init(void)
{
	DIDR0 |= (1 << ADC0D);

	/*
	 * Prescaler fixed 16
	 *
	 * Auto trigger source Timer0 Compare Match
	 */
	ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2);
	ADCSRB = (1 << ADTS1) | (1 << ADTS0); /* Timer0 CompA as trigger source */

	DDRB |= (1 << PB5); /* speaker PWM output */

	PORTE |= (1 << PE7); /* initially off */
	DDRE |= (1 << PE7);

	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	OCR1A = 0;
	OCR1B = 0;
	ICR1 = 0;
	TIMSK1 = 0;
	TIFR1 = 0xFF;

	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM10);
	TCCR1B = (1 << CS10);
}

void joystick_sampling_start(uint16_t freq_Hz)
{
	uint8_t ps; /* prescaler value */
	uint8_t cs; /* prescaler register setting */
	if (freq_Hz > 4000) {
		ps = 8;
		cs = (1 << CS01);
	}
	else {
		ps = 64;
		cs = (1 << CS01) | (1 << CS00);
	}

	ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << MUX0); /* reference AVCC, ADC1 as input */
	OCR0A = (F_CPU / (2UL * ps * freq_Hz)) - 1;
	TCCR0A = (1 << WGM01) | cs;
	TIMSK0 |= (1 << OCIE0A);
}

void joystick_sampling_stop(void)
{
	TCCR0A = 0; /* stop timer */
	TIMSK0 &= (1 << OCIE0A);
}

/*
 * \brief Start Sampling
 *
 * @param freq_Hz Sampling frequency [Hz]
 *
 */
void audio_sampling_start(uint16_t freq_Hz)
{
	uint8_t ps; /* prescaler value */
	uint8_t cs; /* prescaler register setting */
	if (freq_Hz > 4000) {
		ps = 8;
		cs = (1 << CS01);
	}
	else {
		ps = 64;
		cs = (1 << CS01) | (1 << CS00);
	}

	ADMUX = (1 << REFS0) | (1 << ADLAR); /* reference AVCC, ADC0 as input */

	OCR0A = (F_CPU / (2UL * ps * freq_Hz)) - 1;
	TCCR0A = (1 << WGM01) | cs;
	TIMSK0 |= (1 << OCIE0A);
}

/*
 * \brief Stop Sampling
 */
void audio_sampling_stop(void)
{
	TCCR0A = 0; /* stop timer */
	TIMSK0 &= (1 << OCIE0A);
}

/*
 * \brief Start Temperature measurement
 *        Single measurement, no timer
 *
 */
void temperature_start(void)
{
		/* Power up sensor */
	DDRF |= (1<<PF6);
	PORTF |= (1<<PF6);

	DIDR0 |= (1 << ADC4D);

	/* stop timer */
	TCCR0A = 0;
	TIMSK0 = 0;

	/* Use AVCC as reference you can eliminate dependency on MCU voltage
	 * because the resistor divider is powered from IO pin (equal to AVCC)
	 */
	ADMUX = (1 << REFS0) | (1<<MUX2); /* reference AVCC, ADC4 as input */
	/*
	 * Prescaler fixed 16
	 *
	 * Auto trigger source Timer0 Compare Match
	 */
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADIF) | (1 << ADPS2);	
	ADCSRB = 0;
	
	ADCSRA |= (1 << ADSC); /* start conversion */
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
void vmcu_start(void)
{
	uint16_t val;

	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); /* PS 64 */

	ADMUX = (1<<REFS0) | (0x0E); /* reference: AVCC, input Bandgap 1.1V */
	_delay_us(200); /* some time to settle */

	ADCSRA |= (1<<ADIF); /* clear flag */

	/* dummy cycle after REF change (suggested by datasheet) */
	ADCSRA |= (1 << ADSC); /* start conversion */
	while(0 == (ADCSRA & (1<<ADIF)));

	_delay_us(100); /* some time to settle */

	ADCSRA |= (1<<ADIF); /* clear flag */
	ADCSRA |= (1 << ADSC); /* start conversion */
	while(0 == (ADCSRA & (1<<ADIF)));
	val = ADC;

	ADCSRA &= ~((1<<ADEN) | (1<<ADIE));

	vmcu_sample_cb(val);
}



/* EOF */
