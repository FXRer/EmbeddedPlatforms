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

#ifndef MMA7455_H_
#define MMA7455_H_

/* === includes ============================================================ */

/* === macros ============================================================== */

#define MMA7455_INT1_EN() do{ PCMSK2 |= _BV(PCINT18); }while(0)
#define MMA7455_INT1_DIS() do{ PCMSK2 &= ~_BV(PCINT18); }while(0)
#define MMA7455_INT2_EN() do{ PCMSK2 |= _BV(PCINT17); }while(0)
#define MMA7455_INT2_DIS() do{ PCMSK2 &= ~_BV(PCINT17); }while(0)

/* === types =============================================================== */

typedef struct{
    int16_t x;	/**< acceleration x-axis [mg] */
	int16_t y;	/**< acceleration y-axis [mg] */
	int16_t z;	/**< acceleration z-axis [mg] */
}mma7455_result_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

uint8_t mma7455_regrd(const uint8_t addr, uint8_t nbbytes, uint8_t *buf);
void mma7455_regwr(const uint8_t addr, const uint8_t val);
int16_t mma7455_readresult(const uint8_t axis);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef MMA7455_H_ */
