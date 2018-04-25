/* Copyright (c) 2011 Daniel Thiele
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

#ifndef SENSORS_H_
#define SENSORS_H_

/* === includes ============================================================ */
#include <stdint.h>
#include <sensors.h>

/* === macros ============================================================== */

#define MMA7455_INT1_EN() do{ PCMSK2 |= (1<<PCINT18); }while(0)
#define MMA7455_INT1_DIS() do{ PCMSK2 &= ~(1<<PCINT18); }while(0)
#define MMA7455_INT2_EN() do{ PCMSK2 |= (1<<PCINT17); }while(0)
#define MMA7455_INT2_DIS() do{ PCMSK2 &= ~(1<<PCINT17); }while(0)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    uint16_t measure_vmcu(void);
    uint16_t measure_light(void);
    uint16_t measure_sht_temperature(void);
    uint16_t measure_sht_humidity(void);

    void measure_acceleration(sensors_xyz_result_t *acc, uint8_t type);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SENSORS_H_ */
