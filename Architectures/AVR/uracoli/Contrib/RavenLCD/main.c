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
 * @file Application for RavenLCD
 * @brief ...
 *
 * @_addtogroup grpApp...
 * @ingroup grpAppRavenLCD
 *
 *
 * Resources
 *
 * Timer0 Time base for audio sampling and
 *  - Triggers ADC (Compare Match)
 *  - Updates PWM with new sample
 *
 * Timer1 PWM output for speaker
 *
 *
 */

/* === includes ============================================================ */
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>

#include <util/delay.h>

#include "usart.h"
#include "lcd.h"
#include "cmdif.h"
#include "analog.h"
#include "rtc.h"

/* === macros ============================================================== */
#define NOP() do{ asm volatile ("nop"); }while(0)

/* === types =============================================================== */

/* === globals ============================================================= */
volatile uint8_t joystick; /* bitmask for joystick positions */

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/*
 * \brief Initialize LED that is the eye of the Raven
 */
void raveneye_init(void)
{
	DDRB |= (1<<PB7);
	TCCR2A = (1<<COM2A1) | (1<<COM2A0) | (1<<WGM20) | (1<<CS20);
}

/*
 * \brief Set LED that is the eye of the Raven
 * @param level Brightness to set, range [0..255]
 */
void raveneye_set(uint8_t level)
{
	OCR2A = level;
}

/*
 * \brief Initialize Joystick
 */
void joystick_init(void)
{
	DDRE &= ~(1<<PE2);	/* input with pull-up */
	PORTE |= (1<<PE2);
}

/*
 * \brief Status of center click button
 */
uint8_t joystick_is_pushed(void)
{
	return (0 == (PINE & (1<<PE2)));
}

void joystick_sample_cb(uint8_t sample)
{
	if((0 <= ADCH) && (30 > ADCH)){
		joystick = (1<<0);
	}else if((30 <= ADCH) && (96 > ADCH)){
		joystick = (1<<1);
	}else if((96 <= ADCH) && (166 > ADCH)){
		joystick = (1<<2);
	}else if((166 <= ADCH) && (240 > ADCH)){
		joystick = (1<<3);
	}else{ /* no button */
		joystick = 0;
	}
}

void microphone_sample_cb(uint8_t sample)
{
	raveneye_set(sample);
	putchar(sample);
}

void temperature_sample_cb(uint16_t sample)
{
	int16_t temp = log(3000.0/(float)sample);
	printf("ADC=%u, Temperature=%d\n", sample, temp);

	LCD_puti(temp);
	LCD_setsymbol(LCD_SYMBOL_degC, 1);
}

void vmcu_sample_cb(uint16_t sample)
{
	printf("VMCU=%lu mV\n", (1100UL * 1023UL) / sample);
}

void rtc_tick_cb(uint16_t systime_sec)
{
	//LCD_puti(systime_sec);
	//LCD_setsymbol(LCD_SYMBOL_COLON, systime_sec % 2);
}

int main(void)
{
	uint8_t old_joystick;

	/* UserIO 1..4 */
	DDRE |= (1<<PE3) | (1<<PE4) | (1<<PE5) | (1<<PE6); 
	PORTE &= ~( (1<<PE3) | (1<<PE4) | (1<<PE5) | (1<<PE6) ); 
	
	/* Relay driver */
	DDRB |= (1<<PB6);
	PORTB &= ~(1<<PB6);

	rtc_init();
	joystick_init();
	usart_init(9600UL);
	// raveneye_init();
	LCD_init();
	audio_init();

	/* initialize serial flash via SPI */
	/* pullup for SS of SPI unit external (R308) */
    PORTB |= 0x0F; /* Out High, In Pullup (MOSI) */
	DDRB &= ~0x0F;
    DDRB  |=  0x07; /* SS, MOSI, SCK as output */
    SPCR = ((1<<SPE) | (1<<MSTR));

	/* setup stream for UART communication */
	FILE usart_stdio = FDEV_SETUP_STREAM(usart_putc, usart_getc, _FDEV_SETUP_RW);
	stdin = stdout = stderr = &usart_stdio;

	sei();

	_delay_ms(100); /* wait for m1284P on Raven */

	printf("Raven alive\n");
	printf("Get help on commands entering \"?\"\n");

	joystick_sampling_start(100);

	old_joystick = joystick = 0;

	for (;;)
	{
		if (joystick_is_pushed())
		{
			LCD_setsymbol(LCD_SYMBOL_LOGO, 1);
			printf("Click\n"); /* use this as debounce delay */

			while (joystick_is_pushed())
				;
			LCD_setsymbol(LCD_SYMBOL_LOGO, 0);
		}

		if(old_joystick != joystick){
			switch(joystick){
			case (1<<0) :
				LCD_putchar('U', 1);
				printf("Up\n");
				break;
			case (1<<1) :
				LCD_putchar('R', 1);
				printf("Right\n");
				break;
			case (1<<2):
				LCD_putchar('L', 1);
				printf("Left\n");
				break;
			case (1<<3) :
				LCD_putchar('D', 1);
				printf("Down\n");
				break;
			default:
				LCD_putchar('X', 1);
				break;
			} /* switch(joystick) */

			old_joystick = joystick;
		}

		cmdif_task();

		// sleep_mode();
	}

	return 0;
}

/* EOF */
