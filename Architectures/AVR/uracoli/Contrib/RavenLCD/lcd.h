/* Copyright (c) 2009 Axel Wachtler
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

#ifndef LCD_H
#define LCD_H

/* === includes ============================================================ */

/* === macros ============================================================== */
/* Number of 14-segment digits */
#define LCD_NBDIGITS14 (7)

/* Number of 7-segment digits */
#define LCD_NBDIGITS7 (4)

/* Base address of LCD data field */
#define LCD_BASEADDR (0xEC)

/* === types =============================================================== */
typedef enum {
	LCD_SYMBOL_LOGO,

	LCD_SYMBOL_ZIGBEE,
	LCD_SYMBOL_ZLINK,
	LCD_SYMBOL_SUN,
	LCD_SYMBOL_MOON,
	LCD_SYMBOL_MAIL0,
	LCD_SYMBOL_MAIL1,
	LCD_SYMBOL_MAIL2,

	LCD_SYMBOL_MICROPHONE,
	LCD_SYMBOL_SPEAKER,

	/* Group of battery symbol */
	LCD_SYMBOL_BATT0,
	LCD_SYMBOL_BATT1,
	LCD_SYMBOL_BATT2,
	LCD_SYMBOL_BATT3,

	/* Group of 4 7-segment digits */
	LCD_SYMBOL_COLON,
	LCD_SYMBOL_POINT,
	LCD_SYMBOL_MINUS,

	LCD_SYMBOL_ATTENTION,
	LCD_SYMBOL_KEY,
	LCD_SYMBOL_MELODY,
	LCD_SYMBOL_BELL,

	/* Group RF */
	LCD_SYMBOL_RX,
	LCD_SYMBOL_TX,
	LCD_SYMBOL_ANT0,
	LCD_SYMBOL_ANT1,
	LCD_SYMBOL_ANT2,
	LCD_SYMBOL_ANT3,
	LCD_SYMBOL_ANT4,

	LCD_SYMBOL_IP,
	LCD_SYMBOL_PANID,
	LCD_SYMBOL_AM,
	LCD_SYMBOL_PM,
	LCD_SYMBOL_degC, /**< degree Celsius */
	LCD_SYMBOL_degF, /**< degree Fahrenheit */
} LCD_symbol_t;

typedef struct{
	LCD_symbol_t symbol; /**< symbol enum */
	uint8_t *reg; /**< pointer to register (LCDDR00 .. 19) */
	uint8_t bitpos; /**< bit position in register */
	uint8_t *name; /**< name as string to use in command interface */
} LCD_symbol_container_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

void LCD_init(void);
void LCD_disable(void);
void LCD_cls(void);
uint8_t LCD_getsymboltable(LCD_symbol_container_t **tbl);
void LCD_setsymbol(LCD_symbol_t symbol, uint8_t state);
void LCD_setbatt(uint8_t level);
void LCD_setantenna(uint8_t level);
void LCD_putdigit(unsigned char c, unsigned char digit);
void LCD_putchar(unsigned char c, unsigned char digit);

void LCD_puti(int16_t i);
void LCD_putf(float f);
void LCD_puts(const char *s);
void LCD_puthex(uint16_t val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef LCD_H */
