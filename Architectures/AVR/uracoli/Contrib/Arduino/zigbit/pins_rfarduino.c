/*
  pins_zigbit.c - pin definitions for the Arduino board
  Part of Arduino / Wiring Lite

  Copyright (c) 2005 David A. Mellis

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

  $Id$
*/
#include <avr/io.h>
#include "wiring_private.h"
#include "pins_uracoli.h"

/*
  This is the RFArduino pin mapping


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

#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7

#define REPEAT8(x) x, x, x, x, x, x, x, x
#define BV0TO7 _BV(0), _BV(1), _BV(2), _BV(3), _BV(4), _BV(5), _BV(6), _BV(7)
#define BV7TO0 _BV(7), _BV(6), _BV(5), _BV(4), _BV(3), _BV(2), _BV(1), _BV(0)


/*
 * these arrays map port names (e.g. port B) to the
 * appropriate addresses for various functions (e.g. reading
 * and writing)
 */
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT, /* idx 0 unused  */
	NOT_A_PORT, /* port A unused */
	(uint16_t)&DDRB,
    NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&DDRE,
	(uint16_t)&DDRF,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PORTB,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PORTE,
	(uint16_t)&PORTF,
	NOT_A_PORT,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PINB,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PINE,
	(uint16_t)&PINF,
	NOT_A_PORT,
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
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
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
};
#warning "continue here"
const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {

	TIMER1A,        /* D0  */
	TIMER1B,        /* D1  */
	TIMER0A,        /* D2  */
	NOT_ON_TIMER,   /* D3  */
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

};
