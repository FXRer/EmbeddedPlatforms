/* Copyright (c) 2016 Axel Wachtler
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
/* Example for using the I2C ISL29020 light sensor */
#include <stdlib.h>
#include "board.h"
#include "ioutil.h"
#include "sensors/dht22.h"
#include "hif.h"
#include "xmpl.h"
/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
dht22_ctx_t dht22_ctx;
bool do_measure;
/* === prototypes ========================================================== */
bool process_command(char chr);

/* === functions =========================================================== */
int main(void)
{
const uint32_t br = HIF_DEFAULT_BAUDRATE;
int chr;
uint8_t rv;

    mcu_init();

    /* setting up UART and adjusting the baudrate */
    hif_init(br);
    LED_INIT();
    LED_SET(0);
    MCU_IRQ_ENABLE();
    PRINTF(EOL"DHT22 Sensor Example : %s : %ld bit/s"EOL, BOARD_NAME, br);

    while (1)
    {
        chr = hif_getc();
        if (chr != -1)
        {
            PRINTF("CMD: %c"EOL, chr);
        }
    }
}
