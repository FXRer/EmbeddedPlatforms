/* Copyright (c) 2008 Axel Wachtler
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
/* Example use of the buffer functions */
#include <stdint.h>
#include <stdio.h>
#include "board.h"
#include "ioutil.h"
#include "hif.h"
#include "radio.h"
#include "transceiver.h"

static inline void WAIT_MS(uint16_t t)
{
    while (t--) DELAY_MS(1);
}

#define CHANNEL (17)
static uint8_t frame_sent = 0 ;

int main(void)
{

/* buffer is smaller than a limmerick line */
#define XMPL_FRAME_SIZE (40)
/*two byte more for the CRC */
uint8_t txbuf[sizeof(buffer_t) + XMPL_FRAME_SIZE + 2];

uint8_t frame_header[] = {0x01, 0x80, 0, 0x11,0x22,0x33,0x44};
buffer_t *pbuf;
//                           1         2         3         4


uint8_t num_stations = 3 ;
uint8_t station_temp[] = { 12, 23, 41 } ; /* start temperatures */
uint16_t station_id[] = { 2, 4, 5 } ; /* start temperatures */

char sendstr[20]  ;       /* string to send temperatures */

char *psendstr;

    /* Prerequisite: Init radio */
    LED_INIT();
    radio_init(NULL, 0);
    sei();
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_state(STATE_TX);

    /* Initialize the buffer structure */
    pbuf = buffer_init(txbuf, sizeof(txbuf)-2, sizeof(frame_header));

    uint8_t i ;
    uint16_t randi = 0 ;

    while(1)
    {
        for(i = 0; i < num_stations;i++){

            /* new temp */
            /* http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=254771 */
            randi=(randi*109+89)%251;
            if ( randi > 200 )
            {
                station_temp[i]++ ;
            }
            else if ( randi <50 )
            {
                station_temp[i]-- ;
            } ;


            /* sprintf( sendstr, "%cT%02dC\n", station_id[i], station_temp[i]) ; */
            sprintf( sendstr, "n: 0x%04x, t: %02d\n\r", station_id[i], station_temp[i]) ;

            psendstr = sendstr;
            /* fill buffer until '\n' or '\0' or EOF is reached */
            do
            {
                if (buffer_append_char(pbuf, *psendstr) == EOF)
                {
                    break;
                }
                psendstr++;
            }
            while (*psendstr != 0 );

            /* finalize the buffer and transmit it */
            buffer_prepend_block(pbuf, frame_header, sizeof(frame_header));
            radio_set_state(STATE_TX);

            frame_sent = 0 ;
            radio_send_frame(BUFFER_SIZE(pbuf)+ 2, BUFFER_PDATA(pbuf), 0);

            WAIT_MS(500) ;

            /* prepare next run */
            BUFFER_RESET(pbuf,sizeof(frame_header));
            frame_header[2]++;
        }

        /* wait after this run */
        LED_SET(0);
        WAIT_MS(100);
        LED_CLR(0);

    }
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    frame_sent = 1 ;
}

/* XEOF */
