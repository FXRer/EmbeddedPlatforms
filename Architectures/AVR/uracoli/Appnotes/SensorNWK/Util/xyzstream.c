/* Copyright (c) 2012 Daniel Thiele
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

/* avr-libc */
#include <string.h>

/* project */
#include <sensors.h>
#include <sensorproto.h>
#include <sensornwk.h>

static uint8_t idx = 0;
static sensorproto_generic_xyzstream_t xyzstream;

static void xyzstream_transmit(sensornwk_frametype_t frametype)
{
    xyzstream.nbsamples = idx;
    idx = 0;
    sensornwk_tx((uint8_t*) &xyzstream,
                 sizeof(sensorproto_generic_xyzstream_t),
                 frametype, 1);
}

/*
 * \brief Put XYZ sample into queue
 */
void xyzstream_put(sensornwk_frametype_t frametype, sensors_xyz_result_t *res)
{
    if (idx < sizeof(xyzstream.xyz) / sizeof(xyzstream.xyz[0]))
    {
        memcpy(&xyzstream.xyz[idx], res, sizeof(sensors_xyz_result_t));
        idx++;
    }
    else
    {
        xyzstream_transmit(frametype);
    }
}

/*
 * \brief Flush queue: force sending of data that is stored already
 */
void xyzstream_flush(sensornwk_frametype_t frametype)
{
    xyzstream_transmit(frametype);
}

/* EOF */
