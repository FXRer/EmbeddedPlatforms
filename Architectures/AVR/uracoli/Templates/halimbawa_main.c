/* Copyright (c) 2015 Daniel Thiele, Axel Wachtler
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
 * @file
 * @brief
 *
 * To change to a different board:
 *   - set your MCU: Project Properties - Device - Change Device
 *   - set F_CPU: Project Properties - Toolchain - Symbols - F_CPU={yourfrequency}
 *   - set Board name: Project Properties - Toolchain - Symbols - {yourboardname}
 *
 * @_addtogroup grpApp...
 */


#include <board.h>
#include <transceiver.h>
#include <radio.h>

/* Buffer to store received frames */
static uint8_t rxbuf[MAX_FRAME_SIZE];


/* Radio callback functions */

uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
	/* Process received frame here */
	return frm;
}


void usr_radio_tx_done(radio_tx_done_t status)
{
	
}

int main(void)
{
	radio_init(rxbuf, sizeof(rxbuf)/sizeof(rxbuf[0]));
	
	sei();
		
    for(;;);
}

/* EOF */
