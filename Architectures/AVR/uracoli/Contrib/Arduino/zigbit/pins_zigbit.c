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
#if defined(wdba1281) || defined (mnb900)

#include <avr/io.h>
#include "wiring_private.h"
#include "pins_uracoli.h"

/*
  This is the ZigBit PIN Mapping

              +----------------------------+
              | #       48 47 46 45        |
           -  |  1                      43 | D21 / IRQ6
           -  |  2                      42 | D20 / IRQ5
           -  |  3                      41 | D19 / PWM
     PWM / D0 |  4                      40 | D18
     PWM / D1 |  5                      39 | D17
     PWM / D2 |  6    ZigBit            38 | D16
   OSC32K_OUT |  7                      37 | D15 / PWM
        RESET |  8    ATZB-24-B0        36 | D14 / PWM
          GND |  9                      35 | AGND
      CPU_CLK | 10                      34 | AREF
    IRQ0 / D3 | 11                      33 | ADC0
    IRQ1 / D4 | 12                      32 | ADC1
    IRQ2 / D5 | 13                      31 | ADC2
    IRQ3 / D6 | 14                      30 | ADC3
    IRQ4 / D7 | 15                      29 | ADC4 / JTAG-TCK
           D8 | 16                      28 | ADC6 / JTAG-TDO
           D9 | 17                      27 | ADC7 / JTAG-TDI
          D10 | 18                      26 | ADC5 / JTAG-TMS
              |    19 20 21 22 23 24 25    |
              +----------------------------+
                   D  D  D  G  G  V  V
                   1  1  1  N  N  C  C
                   1  2  3  D  D  C  C




   Pin ZDM-A1281-A2  ATmega1281                Arduino    AT86RF230
---------------------------------------------------------------------
  int                PA7                          -       RST
  int                PB4                          -       SLP_TR
  int                PE5                          -       IRQ
  int                PB0                          -       /SEL

   1   SPI_CLK       PB1 (SCK/PCINT1)             -       SCLK
   2   SPI_MISO      PB3 (MISO/PCINT3)            -       MISO
   3   SPI_MOSI      PB2 (MOSI/PCINT2)            -       MOSI
   4   GPIO0         PB5 (OC1A/PCINT5)         D0  PWM    :
   5   GPIO1         PB6 (OC1B/PCINT6)         D1  PWM    :
   6   GPIO2         PB7 (OC0A/OC1C/PCINT7)    D2  PWM    :

   7   OSC32K_OUT    PG3 (TOSC2)               :          :

   8   RESET         RESET                     :          :
   9   DGND          GND                       :          :
  10   CPU_CLK       XTAL1                        -       CLKM

  11   I2C_CLK       PD0 (SCL/INT0)            D3 IRQ0    :
  12   I2C_DATA      PD1 (SDA/INT1)            D4 IRQ1    :
  13   UART_TXD      PD2 (TXD1/INT2)           D5 IRQ2    :
  14   UART_RXD      PD3 (RXD1/INT3)           D6 IRQ3    :
  15   UART_RTS      PD4 (ICP1)                D7         :
  16   UART_CTS      PD5 (XCK1)                D8         :
  17   GPIO6         PD6 (T1)                  D9         :
  18   GPIO7         PD7 (T0)                  D10        :

  19   GPIO3         PG0 (WR)                  D11        :
  20   GPIO4         PG1 (RD)                  D12        :
  21   GPIO5         PG2 (ALE)                 D13        :

  22   DGND          GND                       :          :
  23   DGND          GND                       :          :
  24   D_VCC         VCC                       :          :
  25   D_VCC         VCC                       :          :
  26   JTAG-TMS      PF5 (ADC5/JTAG-TMS)       :          :
  27   JTAG-TDI      PF7 (ADC7/JTAG-TDI)       :          :
  28   JTAG-TDO      PF6 (ADC6/JTAG-TDO)       :          :
  29   JTAG-TCK      PF4 (ADC4/JTAG-TCK)       :          :

  30   ADC_INPUT_3   PF3 (ADC3)                AI3        :
  31   ADC_INPUT_2   PF2 (ADC2)                AI2        :
  32   ADC_INPUT_1   PF1 (ADC1)                AI1        :
  33   BAT           PF0 (ADC0)                AI0        :

  34   A_VREF        AREF                      AREF       :
  35   AGND          GND                       :          :

  36   GPIO_1WIRE    PG5 (OC0B)                D14 PWM    :

  37   UART_DTR      PE4 (OC3B/INT4)           D15 PWM IRQ4
  38   USART0_RXD    PE0 (RXD0/PCINT8/PDI)     D16        :
  39   USART0_TXD    PE1 (TXD0/PD0)            D17        :
  40   USART0_EXTCLK PE2 (XCK0/AIN0)           D18        :
  41   GPIO8         PE3 (OC3A/AIN1)           D19 PWM    :
  42   IRQ7          PE7 (ICP3/CLK0/INT7)      D20 IRQ5   :
  43   IRQ6          PE6 (T3/INT6)             D21 IRQ6   :


This Table was found on
http://www.mikrocontroller.net/articles/Meshnetics_Zigbee#ZDM-A1281-A2_PinOUT_vs._embedded_ATmega1281_und_AT86RF230

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
	(uint16_t)&DDRC,
	(uint16_t)&DDRD,
	(uint16_t)&DDRE,
	(uint16_t)&DDRF,
	(uint16_t)&DDRG,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PORTB,
	(uint16_t)&PORTC,
	(uint16_t)&PORTD,
	(uint16_t)&PORTE,
	(uint16_t)&PORTF,
	(uint16_t)&PORTG,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t)&PINB,
	(uint16_t)&PINC,
	(uint16_t)&PIND,
	(uint16_t)&PINE,
	(uint16_t)&PINF,
	(uint16_t)&PING,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
          /* port  */
	PB,   /* D0 */
	PB,   /* D1 */
	PB,   /* D2 */

	PD,   /* D3 */
	PD,   /* D4 */
	PD,   /* D5 */
	PD,   /* D6 */
	PD,   /* D7 */
	PD,   /* D8 */
	PD,   /* D9 */
	PD,   /* D10 */

	PG,   /* D11 */
	PG,   /* D12 */
	PG,   /* D13 */
	PG,   /* D14 */

	PE,   /* D15 */
	PE,   /* D16 */
	PE,   /* D17 */
	PE,   /* D18 */
	PE,   /* D19 */
	PE,   /* D20 */
	PE,   /* D21 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(PB5), /* D0  */
	_BV(PB6), /* D1  */
	_BV(PB7), /* D2  */

	_BV(PD0), /* D3  */
	_BV(PD1), /* D4  */
	_BV(PD2), /* D5  */
	_BV(PD3), /* D6  */
	_BV(PD4), /* D7  */
	_BV(PD5), /* D8  */
	_BV(PD6), /* D9  */
	_BV(PD7), /* D10 */

	_BV(PG0), /* D11 */
	_BV(PG1), /* D12 */
	_BV(PG2), /* D13 */
	_BV(PG5), /* D14 */

	_BV(PE4), /* D15 */
	_BV(PE0), /* D16 */
	_BV(PE1), /* D17 */
	_BV(PE2), /* D18 */
	_BV(PE3), /* D19 */
	_BV(PE7), /* D21 */
	_BV(PE6), /* D21 */
};

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
	TIMER0B,        /* D14 */

	TIMER3B,        /* D15 */
	NOT_ON_TIMER,   /* D16 */
	NOT_ON_TIMER,   /* D17 */
	NOT_ON_TIMER,   /* D18 */
	TIMER3A,        /* D19 */
	NOT_ON_TIMER,   /* D20 */
	NOT_ON_TIMER,   /* D21 */

};

#endif /* #if defined(wdba1281) || defined (mnb900) */
