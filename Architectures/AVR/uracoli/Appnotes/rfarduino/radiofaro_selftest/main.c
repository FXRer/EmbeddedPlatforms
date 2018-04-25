/* Copyright (c) 2011 - 2012 Daniel Thiele, Axel Wachtler
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

/**
 * @file Radiofaro Selftest Software
 *       HW Requirements: Radiofaro 1v4, Selftest shield plugged on
 *
 *
 * @brief
 * @ingroup
 */

/* === includes ============================================================ */

/* avr-libc inclusions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>

/* uracoli inclusions */
#include "board.h"
#include "ioutil.h"
#include "transceiver.h"

/* === macros ============================================================== */

#define EOL "\n"

/* === types =============================================================== */

typedef struct{
	uint8_t ieee_addr[8];
}ee_content_t;

/* === globals ============================================================= */

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

ee_content_t ee_content;

/* === prototypes ========================================================== */

/* === functions =========================================================== */


/*
 * \brief Save MCUSR to variable and disable watchdog
 * This must be done very early after reset, placing to .init3 section is done
 * in the forward declaration above
 *
 */
void __attribute__((naked)) __attribute__((section(".init3"))) get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

int main()
{
	static FILE uart_stream = FDEV_SETUP_STREAM(hif_putc, hif_getc, _FDEV_SETUP_RW);

	LED_INIT();
	hif_init(HIF_DEFAULT_BAUDRATE);

	stdin = stdout = &uart_stream;

	sei();

	printf(EOL""EOL"Radiofaro Selftest vXX"EOL);


	eeprom_read_block((void*)&ee_content.ieee_addr, (const void*)0x0000, sizeof(ee_content_t));

	printf("IEEE_ADDR ");
	for(int8_t i=7;i>=0;i--){
		printf("%02X", ee_content.ieee_addr[i]);
		if(i>0) printf("-");
	}
	printf(EOL);

	printf("RFA Id %02X.%02X"EOL, PART_NUM, VERSION_NUM);

	set_sleep_mode(SLEEP_MODE_IDLE);
	for (;;)
	{
		sleep_mode();
	} /* for(;;); */

	/* never reaches this point */

}
/* EOF */
