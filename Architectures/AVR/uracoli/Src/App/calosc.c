/* Copyright (c) 2007 Axel Wachtler
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
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/delay.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "board.h"
#define KEY_MASK (0x0c)
#define KEY_UP (0x08)
#define KEY_DN (0x04)

void init_hw(void);

int main(void)
{
uint16_t i ;
uint8_t key=0, ct0=0, ct1=0, tmp;
static uint8_t key_state;
static uint8_t osccal;

    init_hw();

    osccal = OSCCAL;
    LED_SET_VALUE(OSCCAL);
    while (1)
    {
        uart_putc(OSCCAL);
        uart_putc(0x01);
        uart_putc(0x02);
        uart_putc(0x04);
        uart_putc(0x08);
        uart_putc(0x10);
        uart_putc(0x20);
        uart_putc(0x40);
        uart_putc(0x80);
        uart_putc(0x00);
        uart_putc(0x55);
        uart_putc(0xAA);
        uart_putc(0xff);
        uart_putc('\n');
        uart_puts("hallo\n");
        for(i=0;i<500;i++)
        {
            _delay_us(10000);

            tmp =(key_state ^ ~PIND);   // key changed ?
            ct0 = ~(ct0 & tmp);         /* reset or count ct0 */
            ct1 =(ct0 ^ ct1) & tmp;     /* reset or count ct1 */
            tmp &= ct0 & ct1;           /* count until roll over ? */
            key_state ^= tmp;           /* then toggle debounced state */
            key |= (key_state & tmp);

            if(key & KEY_UP)
            {
                OSCCAL ++;
                osccal = OSCCAL;
                LED_SET_VALUE(osccal);
                key = 0;
                _delay_us(100);
            }
            if(key & KEY_DN)
            {
                OSCCAL --;
                osccal = OSCCAL;
                LED_SET_VALUE(osccal);
                key = 0;
                _delay_us(100);
            }
        }
    }
}

void init_hw(void)
{
    mcu_init();
    OSCCAL = TUNED_OSCCAL;
    uart_init(UART_BAUD_SELECT(9600,F_CPU));
    /* switches */
    DDRD  &= ~0x0c;
    PORTD |= 0x0c;

    /* LED state */
    LED_INIT();
    sei();

}
