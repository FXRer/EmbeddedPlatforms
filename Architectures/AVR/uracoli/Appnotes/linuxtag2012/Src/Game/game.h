/* Copyright (c) 2012 Axel Wachtler, Joerg Wunsch
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

#ifndef GAME_H
#define GAME_H

/* === includes ============================================================ */

/* === macros ============================================================== */
#define COL_PADS    (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))
#define NB_FIELDS (9)

/* FIXNE: move KEY_NONE, START_GAME, KEY0...KEY8 to enum */
#define REQUEST_GAME (9)
#define START_GAME (10)
#define KEY_NONE (255)

#define XXO_PRINTF(fmt, ...) xxo_radio_printf(FLASH_STRING(fmt), __VA_ARGS__)
#define XXO_PRINT(fmt)       xxo_radio_printf(FLASH_STRING(fmt))

#define AVR_DO_SLEEP() sleep_mode()

#define FLASH_STRING(x) PSTR(x)

/* === types =============================================================== */
typedef struct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t col;
} winner_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

void xxo_do_sleep(int16_t sleep_time);

void xxo_leds_init();
void xxo_leds_display(uint8_t row);
void xxo_flash_leds(uint8_t n, uint8_t color);
void xxo_flood_leds(uint8_t color);
void xxo_set_leds(uint8_t idx, uint8_t color);
void xxo_highlight_winning_pattern(uint8_t n, winner_t *w);

void xxo_scan_pads(uint8_t row);
uint8_t xxo_get_key(void);

void xxo_radio_init(void);
void xxo_radio_printf(const char *fmt, ...);
uint8_t xxo_radio_get_event(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef GAME_H */
