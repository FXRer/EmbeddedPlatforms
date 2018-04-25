/* Copyright (c) 2014 Daniel Thiele, Axel Wachtler
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

#ifndef OW_H_
#define OW_H_

/* === includes ============================================================ */

#include <stdbool.h>
#include <avr/io.h>

/* === macros ============================================================== */

#define PINHI(pin)   pin##_PORT |= (1<<pin)
#define PINLO(pin)   pin##_PORT &= ~(1<<pin)
#define PINOUTP(pin) pin##_DDR |= (1<<pin)
#define PININP(pin)  pin##_DDR &= ~(1<<pin)
#define PINGET(pin) (0 != (pin##_PIN & (1<<pin)))

#define DQ_PIN	PIND
#define DQ_DDR	DDRD
#define DQ_PORT	PORTD
#define DQ		(PD0)

//#define DBG_SLOT_PIN	PINB
//#define DBG_SLOT_DDR	DDRB
//#define DBG_SLOT_PORT	PORTB
//#define DBG_SLOT		(PB7)

#define OW_ROMCMD_READ (0x33)
#define OW_ROMCMD_MATCH (0x55)
#define OW_ROMCMD_SEARCH (0xF0)
#define OW_ROMCMD_ALARM (0xEC)
#define OW_ROMCMD_SKIP (0xCC)

/* === types =============================================================== */

typedef uint64_t ow_serial_t;

/* === prototypes ========================================================== */

void ow_init(void);
uint8_t ow_reset(void);
void ow_byte_write(uint8_t byte);
uint8_t ow_byte_read(void);
bool ow_master_searchrom(ow_serial_t *ser, bool first);
void ow_master_matchrom(ow_serial_t ser);

#endif /* OW_H_ */
