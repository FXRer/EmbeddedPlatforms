/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/


/*
  This is the Radiofaro pin mapping


--    J1            --       (Digital)
Arduino         type         ATmega128RFA1              deRFMega128
1    PD0/RxD     d0          PE0/RXD0/PCINT8               26
2    PD1/TxD     d1          PE1/TXD0                      28
3    PD2/INT0    d2          PE4/OC3B/INT4 (PWM)           33
4    PD3/INT1    d3 (PWM)    PE5/OC3C/INT5 (PWM)           35
5    PD4/T0      d4          PE6/INT6                      37
6    PD5/T1      d5 (PWM)    PE7/INT7                      39
7    PD6/AIN0    d6 (PWM)    PE2/AIN0                      30
8    PD7/AIN1    d7          PE3/OC3A/AIN1 (PWM)           31

--    J3            --    (SPI)
Arduino                      ATmega128RFA1              deRFMega128
1    PB0/ICP      d8  (ICP)  PB4/OC2A/PCINT4 (PWM)          5
2    PB1/OC1      d9  (PWM)  PB5/OC1A/PCINT5 (PWM)          3
3    PB2/SS       d10 (PWM)  PB0/PCINT0/SS                  8
4    PB3/MOSI     d11 (PWM)  PB2/PCINT2/MOSI                9
5    PB4/MISO     d12        PB3/PCINT3/MISO                7
6    PB5/SCK      d13        PB1/PCINT1/SCK                11
7    GND
8    AREF         AREF                                     21

--    J2            --    (AIN)
Arduino                      ATmega128RFA1              deRFMega128
1    PC0/ADC0    a0/d14      PF0/ADC0                      34
2    PC1/ADC1    a1/d15      PF1/ADC1                      36
3    PC2/ADC2    a2/d16      PF4/ADC4                      38
4    PC3/ADC3    a3/d17      PF5/ADC5                      40
5    PC4/ADC4    a4/d18      PF6/ADC6                      42
6    PC5/ADC5    a5/d19      PF7/ADC7                      44

-- ONBAORD
                 d20 (LED1)  PG1/DIG1                      todo
                 d21 (LED2)  PG2/AMR                       todo

*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#define radiofaro (1)
#define Serial  Serial1

#define NUM_DIGITAL_PINS           (20)
#define NUM_ANALOG_INPUTS          (6)
#define analogInputToDigitalPin(p) ((p < 6) ? (p) + 14 : -1)


/* PWM @ D2, D3, D7, D8, D9 */
#define digitalPinHasPWM(p)        ((p) == 2 || (p) == 3 || ((p) > 6 && (p) < 10))


const static uint8_t SS   = 10;
const static uint8_t MOSI = 11;
const static uint8_t MISO = 12;
const static uint8_t SCK  = 13;

/* todo: check for PD0/PD1 ... left over
const static uint8_t SDA = 20;
const static uint8_t SCL = 21;
*/
const static uint8_t LED_BUILTIN = 13;
const static uint8_t LED1_BUILTIN = 20;
const static uint8_t LED2_BUILTIN = 21;

const static uint8_t A0 = 14;
const static uint8_t A1 = 15;
const static uint8_t A2 = 16;
const static uint8_t A3 = 17;
const static uint8_t A4 = 18;
const static uint8_t A5 = 19;


/*
    pin change IRQs:
    d0         PE0/PCINT8
    d8  (ICP)  PB4/PCINT4
    d9  (PWM)  PB5/PCINT5
    d10 (PWM)  PB0/PCINT0
    d11 (PWM)  PB2/PCINT2
    d12        PB3/PCINT3
    d13        PB1/PCINT1
 */

#define digitalPinToPCICR(p)    ( ((p) == 0) ||\
                                  (((p) >= 8) && ((p) <= 13)) ? \
                                  (&PCICR) : ((uint8_t *)0) )

#define digitalPinToPCICRbit(p) ( ((p) == 0 ) ? 1 : \
                                  (((p) >= 8) && ((p) <= 10)) ? 0 : -1 \
                                )

#define digitalPinToPCMSK(p)    ( ((p) == 0 ) ? (&PCMSK1) : \
                                  (((p) >= 8) && ((p) <= 10)) ? (&PCMSK0) : 0\
                                )

#define digitalPinToPCMSKbit(p) ( ((p) == 0)  ? 0 : \
                                  ((p) == 8)  ? 4 : \
                                  ((p) == 9)  ? 5 : \
                                  ((p) == 10) ? 0 : \
                                  ((p) == 11) ? 2 : \
                                  ((p) == 12) ? 3 : \
                                  ((p) == 13) ? 1 : -1 \
                                )

/*
    external Interrupts:
    d2          PE4/INT4
    d3 (PWM)    PE5/INT5
    d4          PE6/INT6
    d5 (PWM)    PE7/INT7
    --> EICRB, see cores/arduino/WInterrupts.c:106 ff.
*/
#define digitalPinToInterrupt(p) ( ((p) == 2) ? 0 : \
                                   ((p) == 3) ? 1 : \
                                   ((p) == 4) ? 6 : \
                                   ((p) == 5) ? 7 : NOT_AN_INTERRUPT )

#ifdef ARDUINO_MAIN

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	(uint16_t) &DDRG,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTE,
	(uint16_t) &PORTF,
	(uint16_t) &PORTG,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PIN,
	NOT_A_PIN,
	(uint16_t) &PINB,
	NOT_A_PIN,
	NOT_A_PIN,
	(uint16_t) &PINE,
	(uint16_t) &PINF,
	(uint16_t) &PING,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
	NOT_A_PIN,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
        /* port  */
	PE,   /* D0 */
	PE,   /* D1 */
	PE,   /* D2 */
	PE,   /* D3 */
	PE,   /* D4 */
	PE,   /* D5 */
	PE,   /* D6 */
	PE,   /* D7 */

	PB,   /* D8 */
	PB,   /* D9 */
	PB,   /* D10 */

	PB,   /* D11 */
	PB,   /* D12 */
	PB,   /* D13 LED */

        PF,   /* D14 */
        PF,   /* D15 */
        PF,   /* D16 */
        PF,   /* D17 */
        PF,   /* D18 */
        PF,   /* D19 */

	PG,   /* D20, LED1 */
	PG,   /* D21, LED2 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	// PIN IN PORT
	// -------------------------------------------
	_BV(PE0), /* D0  */
	_BV(PE1), /* D1  */
	_BV(PE4), /* D2  */
	_BV(PE5), /* D3  */
	_BV(PE6), /* D4  */
	_BV(PE7), /* D5  */
	_BV(PE2), /* D6  */
	_BV(PE3), /* D7  */

	_BV(PB4), /* D8  */
	_BV(PB5), /* D9  */
	_BV(PB0), /* D10 */
	_BV(PB2), /* D11 */
	_BV(PB3), /* D12 */
	_BV(PB1), /* D13, LED */

	_BV(PF0), /* D14, A0 */
	_BV(PF1), /* D15, A1 */
	_BV(PF4), /* D16, A2 */
	_BV(PF5), /* D17, A3 */
	_BV(PF6), /* D18, A4 */
	_BV(PF7), /* D19, A5 */

	_BV(PG1), /* D20, LED1 */
	_BV(PG2), /* D21, LED1 */
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	// TIMERS
	// -------------------------------------------
	TIMER1A,        /* D0  */
	TIMER1B,        /* D1  */
	TIMER0A,        /* D2  */
	TIMER3C,        /* D3  */
	NOT_ON_TIMER,   /* D4  */
	NOT_ON_TIMER,   /* D5  */
	NOT_ON_TIMER,   /* D6  */
	NOT_ON_TIMER,   /* D7  */

	NOT_ON_TIMER,   /* D8  */
	NOT_ON_TIMER,   /* D9  */
	NOT_ON_TIMER,   /* D10 */

	NOT_ON_TIMER,   /* D11 */
	NOT_ON_TIMER,   /* D12 */
	NOT_ON_TIMER,   /* D13, LED */

	NOT_ON_TIMER,   /* D14, A0 */
	NOT_ON_TIMER,   /* D15, A1 */
	NOT_ON_TIMER,   /* D16, A2 */
	NOT_ON_TIMER,   /* D17, A3 */
	NOT_ON_TIMER,   /* D18, A4 */
	NOT_ON_TIMER,   /* D19, A5 */

	NOT_ON_TIMER,   /* D20, LED1 */
	NOT_ON_TIMER,   /* D21, LED1 */
};

#endif

#endif
