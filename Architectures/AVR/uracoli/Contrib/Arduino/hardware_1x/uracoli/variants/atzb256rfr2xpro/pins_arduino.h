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
Arduino         type         ATmega128RFA1     deRFMega128
1    PD0/RxD     d0          PE0/RXD0            26
2    PD1/TxD     d1          PE1/TXD0            28
3    PD2/INT0    d2          PE4/OC3B/INT4 (PWM) 33
4    PD3/INT1    d3 (PWM)    PE5/OC3C/INT5 (PWM) 35
5    PD4/T0      d4          PE6                 37
6    PD5/T1      d5 (PWM)    PE7                 39
7    PD6/AIN0    d6 (PWM)    PE2/AIN0            30
8    PD7/AIN1    d7          PE3/AIN1 (PWM)      31

--    J3            --    (SPI)
Arduino                      ATmega128RFA1     deRFMega128
1    PB0/ICP      d8  (ICP)  PB4/OC2A (PWM)      5
2    PB1/OC1      d9  (PWM)  PB5/OC1A (PWM)      3
3    PB2/SS       d10 (PWM)  PB0/SS              8
4    PB3/MOSI     d11 (PWM)  PB2/MOSI            9
5    PB4/MISO     d12        PB3/MISO            7
6    PB5/SCK      d13        PB1/SCK             11
7    GND
8    AREF         AREF                           21

--    J2            --    (AIN)
Arduino                      ATmega128RFA1     deRFMega128
1    PC0/ADC0    a0          PF0/ADC0            34
2    PC1/ADC1    a1          PF1/ADC1            36
3    PC2/ADC2    a2          PF4/ADC4            38
4    PC3/ADC3    a3          PF5/ADC5            40
5    PC4/ADC4    a4          PF6/ADC6            42
6    PC5/ADC5    a5          PF7/ADC7            44

*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#define atzb256rfr2xpro (1)
#define Serial  Serial1

#define NUM_DIGITAL_PINS           (20)
#define NUM_ANALOG_INPUTS          (6)
#define analogInputToDigitalPin(p) ((p < 6) ? (p) + 14 : -1)
#define digitalPinHasPWM(p)        ((p) == 2 || (p) == 3 || (p) == 8 || (p) == 9)


const static uint8_t SS   = 10;
const static uint8_t MOSI = 11;
const static uint8_t MISO = 12;
const static uint8_t SCK  = 13;

const static uint8_t SDA = 20;
const static uint8_t SCL = 21;
const static uint8_t LED_BUILTIN = 13;

const static uint8_t A0 = 14;
const static uint8_t A1 = 15;
const static uint8_t A2 = 16;
const static uint8_t A3 = 17;
const static uint8_t A4 = 18;
const static uint8_t A5 = 19;
//const static uint8_t A6 = 60;

// A majority of the pins are NOT PCINTs, SO BE WARNED (i.e. you cannot use them as receive pins)
// Only pins available for RECEIVE (TRANSMIT can be on any pin):
// (I've deliberately left out pin mapping to the Hardware USARTs - seems senseless to me)
// Pins: 10, 11, 12, 13,  50, 51, 52, 53,  62, 63, 64, 65, 66, 67, 68, 69

#define digitalPinToPCICR(p)    ( (((p) >= 10) && ((p) <= 13)) || \
                                  (((p) >= 50) && ((p) <= 53)) || \
                                  (((p) >= 62) && ((p) <= 69)) ? (&PCICR) : ((uint8_t *)0) )

#define digitalPinToPCICRbit(p) ( (((p) >= 10) && ((p) <= 13)) || (((p) >= 50) && ((p) <= 53)) ? 0 : \
                                ( (((p) >= 62) && ((p) <= 69)) ? 2 : \
                                0 ) )

#define digitalPinToPCMSK(p)    ( (((p) >= 10) && ((p) <= 13)) || (((p) >= 50) && ((p) <= 53)) ? (&PCMSK0) : \
                                ( (((p) >= 62) && ((p) <= 69)) ? (&PCMSK2) : \
                                ((uint8_t *)0) ) )

#define digitalPinToPCMSKbit(p) ( (((p) >= 10) && ((p) <= 13)) ? ((p) - 6) : \
                                ( ((p) == 50) ? 3 : \
                                ( ((p) == 51) ? 2 : \
                                ( ((p) == 52) ? 1 : \
                                ( ((p) == 53) ? 0 : \
                                ( (((p) >= 62) && ((p) <= 69)) ? ((p) - 62) : \
                                0 ) ) ) ) ) )

#ifdef ARDUINO_MAIN

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRE,
	(uint16_t) &DDRF,
	NOT_A_PORT,
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
	NOT_A_PORT,
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
	NOT_A_PIN,
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
	PB,   /* D13 */

        PF,   /* D14 */
        PF,   /* D15 */
        PF,   /* D16 */
        PF,   /* D17 */
        PF,   /* D18 */
        PF,   /* D19 */
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
	_BV(PB1), /* D13 */

	_BV(PF0), /* D14, A0 */
	_BV(PF1), /* D15, A1 */
	_BV(PF4), /* D16, A2 */
	_BV(PF5), /* D17, A3 */
	_BV(PF6), /* D18, A4 */
	_BV(PF7), /* D19, A5 */
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
	NOT_ON_TIMER,   /* D13 */

	NOT_ON_TIMER,   /* D14, A0 */
	NOT_ON_TIMER,   /* D15, A1 */
	NOT_ON_TIMER,   /* D16, A2 */
	NOT_ON_TIMER,   /* D17, A3 */
	NOT_ON_TIMER,   /* D18, A4 */
	NOT_ON_TIMER,   /* D19, A5 */
};

#endif

#endif
