/* Copyright (c) 2013 Axel Wachtler
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
/* Example use of the sensor functions */

#include "board.h"
#include "sensor.h"
#include "ioutil.h"
#include "xmpl.h"

int main(void)
{
    uint8_t buf[128], rv;
    char line[128];
    uint8_t data_size, one_shot = 0, i;


    /* init ressources */
    mcu_init();
    hif_init(HIF_DEFAULT_BAUDRATE);
    LED_INIT();
    LED_SET(0);
    MCU_IRQ_ENABLE();
    i2c_init( 4000000UL );

    /* init all sesnors, individual sensors are adressed with a number
     * from 0 to NB_OF_SENSORS - 1.
     */
    rv = create_board_sensors();

    PRINTF("sensor_init: sensors=%d"EOL, rv );

    PRINTF("number of sensors: %d"EOL, sensor_get_number());
    for (i = 0; i < sensor_get_number(); i++)
    {
        PRINTF("  %d : id=%d name=%s type=%s data_sz=%d"EOL,
               i, sensor_get_id(i),
               sensor_get_name(i),
               sensor_get_type(i),
               sensor_get(i, NULL));
    }
    /* probe for the max. size of the data record with a NULL pointer */
    data_size = sensor_get(-1, NULL);
    PRINTF("total data size: %d"EOL, data_size);

    /* for special configurations of individual sensors, you have to call the
     * low level functions from the sensor directly.
     */
    while (1)
    {
        sensor_trigger(-1, one_shot);
        DELAY_MS(10);
        sensor_get(-1, buf);
        if (one_shot)
        {
            sensor_sleep(-1);
        }
        PRINTF("decode: %s"EOL,sensor_decode(buf, line, sizeof(line)));

        DELAY_MS(2000);
    }
}
