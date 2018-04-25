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
 */

/* === includes ============================================================ */
#include <stdio.h>
#include <avr/io.h>
#include "usart.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */

void usart_init(uint32_t baudrate)
{
	uint16_t brr;
	UCSR0A |= (1<<RXC0) | (1<<TXC0) | (1<<UDRE0); /* clear flags */
	UCSR0A = (1<<U2X0);

	brr = F_CPU / (8UL * baudrate) - 1;
	UBRR0L = (uint8_t) (brr & 0x00FF);
	UBRR0H = (uint8_t) (brr >> 8);

	/* Set frame format: 8 data 1stop */
	UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);

	/* Enable UART receiver and transmitter */
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

int usart_getc(void)
{
	/* Wait for incoming data */
	if (bit_is_set(UCSR0A, RXC0)) {
		return UDR0;
	}
	else {
		return EOF;
	}
}

int usart_putc(char c)
{
	/* Wait for empty transmit buffer */
	while (!bit_is_set(UCSR0A,UDRE0))
		;

	/* Start transmission */
	UDR0 = c;
	return c;
}

/* EOF */
