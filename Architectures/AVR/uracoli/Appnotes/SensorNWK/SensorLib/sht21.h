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

#ifndef SHT21_H_
#define SHT21_H_

/* === includes ============================================================ */
#include <stdint.h>

/* === macros ============================================================== */

#define STH21_CMD_TMEAS_HOLD	(0xE3)
#define STH21_CMD_RHMEAS_HOLD	(0xE5)
#define STH21_CMD_TMEAS_NOHOLD	(0xF3)
#define STH21_CMD_RHMEAS_NOHOLD	(0xF5)
#define SHT21_CMD_WR_USERREG	(0xE6)
#define SHT21_CMD_RD_USERREG	(0xE7)
#define STH21_CMD_SOFTRESET		(0xFE)

#define SHT21_IDENTIFICATION_WIDTH (8)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void sht21_init(uint8_t addr);
    void sht21_softreset(void);
    void sht21_triggerMeas(uint8_t cmd);
    uint8_t sht21_readMeas(uint16_t *res);
    void sht21_identification(uint8_t *buf, uint8_t maxbytes);
    void sht21_write_user_reg(uint8_t value);
    uint8_t sht21_read_user_reg(void);

    float sht21_scale_temperature(uint16_t raw);
    float sht21_scale_humidity(uint16_t raw);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SHT21_H_ */
