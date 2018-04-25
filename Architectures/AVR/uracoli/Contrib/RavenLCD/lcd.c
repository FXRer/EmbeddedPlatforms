/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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
/**
 * @file Driver module for LCD
 * @brief ....
 * @_addtogroup grpApp...
 *
 *
 *  --- ---
 * |\  |  /|
 * | \ | / |
 * |  \|/  |
 *  --- ---
 * |  /|\  |
 * | / | \ |
 * |/  |  \|
 *  --- ---
 *
 */

/* === includes ============================================================ */

/* avr-libc */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>

/* project */
#include "lcd.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* Lookup table for 14-segment characters
 * index follows ascii-table
 *
 */
/*
 * Segment bit masks (hex):
 *
 *    ----0004-----
 *   |\     |     /|
 *   | \    |    / |
 *   | 0002 | 0040 |
 * 0001  \ 0020/  0080
 *   |    \ | /    |
 *   |     \|/     |
 *    -0010- -0800-
 *   |     /|\     |
 *   |    / | \    |
 * 0100  / 2000\  8000
 *   | 0200 | 0400 |
 *   | /    |    \ |
 *   |/     |     \|
 *    ----1000-----
 *
 */
uint16_t LCD_character_table[] = {
	0x2E72, // '*'
	0x2830, // '+'
	0x0200, // ','
	0x0810, // '-'
	0x8C00, // '.'
	0x0240, // '/'
	0x93C5, // '0'
	0x80C0, // '1'
	0x1994, // '2'
	0x9894, // '3'
	0x8891, // '4'
	0x9815, // '5'
	0x9915, // '6'
	0x8084, // '7'
	0x9995, // '8'
	0x9895, // '9'
	0x1800, // ':'
	0x1080, // ';'
	0x0440, // '<'
	0x1810, // '='
	0x0202, // '>'
	0x2884, // '?'
	0x1987, // '@'
	0x8995, // 'A' (+ 'a')
	0xB8A4, // 'B' (+ 'b')
	0x1105, // 'C' (+ 'c')
	0xB0A4, // 'D' (+ 'd')
	0x1915, // 'E' (+ 'e')
	0x0915, // 'F' (+ 'f')
	0x9905, // 'G' (+ 'g')
	0x8991, // 'H' (+ 'h')
	0x2020, // 'I' (+ 'i')
	0x9180, // 'J' (+ 'j')
	0x0551, // 'K' (+ 'k')
	0x1101, // 'L' (+ 'l')
	0x81C3, // 'M' (+ 'm')
	0x8583, // 'N' (+ 'n')
	0x9185, // 'O' (+ 'o')
	0x0995, // 'P' (+ 'p')
	0x9585, // 'Q' (+ 'q')
	0x0D95, // 'R' (+ 'r')
	0x1406, // 'S' (+ 's')
	0x2024, // 'T' (+ 't')
	0x9181, // 'U' (+ 'u')
	0x0341, // 'V' (+ 'v')
	0x8781, // 'W' (+ 'w')
	0x0642, // 'X' (+ 'x')
	0x2042, // 'Y' (+ 'y')
	0x1244, // 'Z' (+ 'z')
	0x1105, // '['
	0x0402, // '\'
	0x9084, // ']'
	0x0600, // '^'
	0x1000 // '_'
};

/* Look-up table for 7-segment characters
      A
     ---
    |   |
   F|   |B
    | G |
     ---
    |   |
   E|   |C
    | D |
     ---

    msb| A | B | F | G | E | C | D |   |lsb (7seg)

*/

uint8_t LCD_digit_table[] = {
     //   ABFGECDX
		0xEE, // '0'
		0x44, // '1'
		0xDA, // '2'
		0xD6, // '3'
		0x74, // '4'
		0xB6, // '5'
		0xBE, // '6'
		0xC4, // '7'
		0xFE, // '8'
		0xF6, // '9'
        0b11111100, // 'A'
        0b00111110, // 'b'
        0b10101010, // 'C'
        0b01011110, // 'd'
        0b10111010, // 'E'
        0b10111000, // 'F'
        0x00, // clear digit
        0x08  // '-'
};

/* Map individual symbols to LCDDR register and bit positions
 *
 */
LCD_symbol_container_t LCD_symbol_table[] =
		{
				{.symbol=LCD_SYMBOL_LOGO, .reg=(uint8_t*)(uint8_t*)&LCDDR03, .bitpos=7, .name=(uint8_t*)(uint8_t*)"logo"},

				{.symbol=LCD_SYMBOL_ZIGBEE, .reg=(uint8_t*)&LCDDR03, .bitpos=5, .name=(uint8_t*)"zigbee"},
				{.symbol=LCD_SYMBOL_ZLINK, .reg=(uint8_t*)&LCDDR08, .bitpos=5, .name=(uint8_t*)"zlink"},
				{.symbol=LCD_SYMBOL_SUN, .reg=(uint8_t*)&LCDDR13, .bitpos=6, .name=(uint8_t*)"sun"},
				{.symbol=LCD_SYMBOL_MOON, .reg=(uint8_t*)&LCDDR03, .bitpos=6, .name=(uint8_t*)"moon"},
				{.symbol=LCD_SYMBOL_MAIL0, .reg=(uint8_t*)&LCDDR08, .bitpos=6, .name=(uint8_t*)"mail0"},
				{.symbol=LCD_SYMBOL_MAIL1, .reg=(uint8_t*)&LCDDR04, .bitpos=0, .name=(uint8_t*)"mail1"},
				{.symbol=LCD_SYMBOL_MAIL2, .reg=(uint8_t*)&LCDDR04, .bitpos=2, .name=(uint8_t*)"mail2"},
		//   msb| A | B | F | G | E | C | D |   |lsb (7seg)

				{.symbol=LCD_SYMBOL_MICROPHONE, .reg=(uint8_t*)&LCDDR03, .bitpos=3, .name=(uint8_t*)"mic"},
				{.symbol=LCD_SYMBOL_SPEAKER, .reg=(uint8_t*)&LCDDR18, .bitpos=2, .name=(uint8_t*)"spk"},

				/* Group of battery symbol */
				{.symbol=LCD_SYMBOL_BATT0, .reg=(uint8_t*)&LCDDR18, .bitpos=4, .name=(uint8_t*)"batt0"},
				{.symbol=LCD_SYMBOL_BATT1, .reg=(uint8_t*)&LCDDR03, .bitpos=4, .name=(uint8_t*)"batt1"},
				{.symbol=LCD_SYMBOL_BATT2, .reg=(uint8_t*)&LCDDR08, .bitpos=4, .name=(uint8_t*)"batt2"},
				{.symbol=LCD_SYMBOL_BATT3, .reg=(uint8_t*)&LCDDR13, .bitpos=4, .name=(uint8_t*)"batt3"},

				/* Group of 4 7-segment digits */
				{.symbol=LCD_SYMBOL_COLON, .reg=(uint8_t*)&LCDDR04, .bitpos=6, .name=(uint8_t*)"colon"},
				{.symbol=LCD_SYMBOL_POINT, .reg=(uint8_t*)&LCDDR04, .bitpos=4, .name=(uint8_t*)"point"},
				{.symbol=LCD_SYMBOL_MINUS, .reg=(uint8_t*)&LCDDR08, .bitpos=7, .name=(uint8_t*)"minus"},

				{.symbol=LCD_SYMBOL_ATTENTION, .reg=(uint8_t*)&LCDDR02, .bitpos=7, .name=(uint8_t*)"att"},
				{.symbol=LCD_SYMBOL_KEY, .reg=(uint8_t*)&LCDDR02, .bitpos=3, .name=(uint8_t*)"key"},
				{.symbol=LCD_SYMBOL_MELODY, .reg=(uint8_t*)&LCDDR17, .bitpos=6, .name=(uint8_t*)"melody"},
				{.symbol=LCD_SYMBOL_BELL, .reg=(uint8_t*)&LCDDR17, .bitpos=2, .name=(uint8_t*)"bell"},

				/* Group RF */		//   msb| A | B | F | G | E | C | D |   |lsb (7seg)

				{.symbol=LCD_SYMBOL_RX, .reg=(uint8_t*)&LCDDR18, .bitpos=6, .name=(uint8_t*)"rx"},
				{.symbol=LCD_SYMBOL_TX, .reg=(uint8_t*)&LCDDR13, .bitpos=5, .name=(uint8_t*)"tx"},
				{.symbol=LCD_SYMBOL_ANT0, .reg=(uint8_t*)&LCDDR18, .bitpos=5, .name=(uint8_t*)"ant0"},
				{.symbol=LCD_SYMBOL_ANT1, .reg=(uint8_t*)&LCDDR00, .bitpos=4, .name=(uint8_t*)"ant1"},
				{.symbol=LCD_SYMBOL_ANT2, .reg=(uint8_t*)&LCDDR00, .bitpos=8, .name=(uint8_t*)"ant2"},
				{.symbol=LCD_SYMBOL_ANT3, .reg=(uint8_t*)&LCDDR01, .bitpos=4, .name=(uint8_t*)"ant3"},
				{.symbol=LCD_SYMBOL_ANT4, .reg=(uint8_t*)&LCDDR01, .bitpos=8, .name=(uint8_t*)"ant4"},

				{.symbol=LCD_SYMBOL_IP, .reg=(uint8_t*)&LCDDR13, .bitpos=7, .name=(uint8_t*)"ip"},
				{.symbol=LCD_SYMBOL_PANID, .reg=(uint8_t*)&LCDDR18, .bitpos=7, .name=(uint8_t*)"panid"},
				{.symbol=LCD_SYMBOL_AM, .reg=(uint8_t*)&LCDDR15, .bitpos=3, .name=(uint8_t*)"am"},
				{.symbol=LCD_SYMBOL_PM, .reg=(uint8_t*)&LCDDR15, .bitpos=6, .name=(uint8_t*)"pm"},
				{.symbol=LCD_SYMBOL_degC, .reg=(uint8_t*)&LCDDR16, .bitpos=6, .name=(uint8_t*)"degc"}, /**< degree Celsius */
				{.symbol=LCD_SYMBOL_degF, .reg=(uint8_t*)&LCDDR16, .bitpos=3, .name=(uint8_t*)"degf"}, /**< degree Fahrenheit */
		};
/* === prototypes ========================================================== */

/* === functions =========================================================== */

void LCD_init(void)
{
	LCDCRA = (1<<LCDEN);
	LCDCRB = 0x3F;
	LCDFRR = 0x65;
	LCDCCR = 0x0C; /* Contrast */
}

/*
 *\brief Special routine acc. to datasheet
 *
 */
void LCD_disable(void)
{
	/* Port pins must be configured to Hi-Z or Active Low because diabling
	 * LCD re-activates Port pin functions
	 */
	/* TODO */ 
	DDRA  = 0xFF;
	PORTA = 0x00;
	DDRC  = 0xFF;
	PORTC = 0x00;
	DDRD  = 0xFF;
	PORTD = 0x00;
	DDRG  = 0x07;
	PORTG = 0x00;
	
	
	LCDCRA |= (1<<LCDIF);
	LCDCRA &= ~(1<<LCDIE); /* disable interrupt */
	
	/* Wait until a new frame is started. */
	while ( !(LCDCRA & (1<<LCDIF)) );

	/* Set LCD Blanking and clear interrupt flag */
	/* by writing a logical one to the flag. */
	LCDCRA |= (1<<LCDIF)|(1<<LCDBL);

	/* Wait until LCD Blanking is effective. */
	while ( !(LCDCRA & (1<<LCDIF)) );

	/* Disable LCD */
	LCDCRA = (0<<LCDEN);
}

/*
 * \brief Clear Screen
 */
void LCD_cls(void)
{
	memset((void*)&LCDDR00, 0, 20); /* 20 registers */
}

uint8_t LCD_getsymboltable(LCD_symbol_container_t **tbl)
{
	*tbl = LCD_symbol_table;
	return sizeof(LCD_symbol_table) / sizeof(LCD_symbol_table[0]);
}


/*
 * \brief Diplays a character on the 14-segment digits
 *		//   msb| A | B | F | G | E | C | D |   |lsb (7seg)

 * @param c Character to be displayed.
 * @param digit Which digit to be write. Valid range [0..6].
 *
 */
void LCD_putchar(unsigned char c, unsigned char digit)
{
	unsigned int seg, segMask;
	unsigned char i;
	unsigned char mask, nibble, nibbleMask;
	unsigned char *ptr;

	if (LCD_NBDIGITS14 <= digit)
	{
		return;
	}

	//Lookup character table for segment data.
	if ((c >= '*') && (c <= 'z'))
	{
		if (c >= 'a')		//   msb| A | B | F | G | E | C | D |   |lsb (7seg)

			c &= ~0x20; // c is in character_table. Convert to upper if necessarry.
		c -= '*';
		if (c > 0x35)
		{
			return; // c points outside array.
		}
		else
		{
			seg = LCD_character_table[c];
		}
	}
	else if (c == ' ')
	{
		seg = 0; // ASCII space
	}
	else if (c == '\033')
	{
		seg = 0x1600; // ASCII <ESC>, small triangle
	}
	else
	{
		return; //ASCII code out of range.
	}

	segMask = 0x4008; // masking out two bits not part of the segment.

	ptr = (uint8_t*) (LCD_BASEADDR + (digit >> 1));

	i = 4; //i used as loop counter.
	do
	{
		nibble = seg & 0x000F;
		nibbleMask = segMask & 0x000F;

		seg >>= 4;
		segMask >>= 4;

		if (digit & 0x01)
		{
			nibble <<= 4;
			mask = 0x0F | (nibbleMask << 4);
		}
		else
		{
			mask = 0xF0 | nibbleMask;
		}
		*ptr = (*ptr & mask) | nibble; //Write new bit values.
		ptr += 5;
	} while (--i);
}

/*
 * \brief Write a hex value to one of the 7-segment digits on the display
 *	      Digit[0..3] can display [0..9][A..F]
 *	      See LCD_digit_table[] for some special symbols that can also be displayed.
 * @param c     Character to be displayed on the 7-segment digit. Valid range is ['*' .. 'F']
 * @param digit Which digit to use. Valid Range [0 .. 3]
 */
void LCD_putdigit(unsigned char c, unsigned char digit)
{
	unsigned char i, j, seg;
	unsigned char mask, lastnibblemask, lookupdata;
	unsigned char *ptr;

	if ((c <= sizeof(LCD_digit_table)/sizeof(uint8_t)) && (digit < 4))
	{
		lookupdata = LCD_digit_table[c];
	}
	else
	{
		return; // ASCII code out of range.
	}

	ptr = (uint8_t*) (LCD_BASEADDR + 19);

	for (j = 0; j < 4; j++) // Read out the 4 half-nibbles from lookupdata.
	{
		mask = 0xC0;
		lastnibblemask = 0x40;
		seg = (lookupdata & mask); // Variable lookupdata contains:
		//   msb| A | B | F | G | E | C | D |   |lsb (7seg)

		//Shift half-nibble and masks into correct location.
		for (i = 0; i < digit; i++)
		{
			seg >>= 2;
			mask >>= 2;
			lastnibblemask >>= 2;
		}
		if (j == 3) //need special mask for the last half nibble since it contains only one valid bit.
		{
			mask = mask & ~lastnibblemask;
		}

		*ptr = (*ptr & ~mask) | seg; //Write new bit values.
		ptr -= 5;
		lookupdata <<= 2; //Prepare for next two bits.
	}
}

static void LCD_clr_digits(void)
{
    /* clear digit display */
    LCD_setsymbol(LCD_SYMBOL_MINUS, 0);
    LCD_putdigit(16, 0);
    LCD_putdigit(16, 1);
    LCD_putdigit(16, 2);
    LCD_putdigit(16, 3);
}


/*
 * \brief Display integer number on 4-digit numeric
 *
 * @param val value interpreted as decimal and displayed on the
 *        7-segment display. If abs(val) is larger then 9999, the
 *        digit display is cleared.
 *
 */
void LCD_puti(int16_t val)
{
    int16_t decpos[4] = {1000, 100, 10, 1}, i;
    int16_t tmp, absval;
    int8_t cnt;

    tmp = absval = abs(val);

    /* clear digit display */
    LCD_clr_digits();

    /* if abs value can be displayed */
    if (absval < 10000)
    {
        if (val < 0)
        {
            LCD_setsymbol(LCD_SYMBOL_MINUS, 1);
        }
        tmp = absval;
        for (i=0; i<4; i++)
        {
            if (absval >= decpos[i])
            {
                cnt = 0;
                while(tmp >= decpos[i])
                {
                    tmp -= decpos[i];
                    cnt ++;
                }
                LCD_putdigit(cnt, i);
            }
        }
    }
}

/*
 * \brief Display hex number on 4-digit numeric
 *
 */
void LCD_puthex(uint16_t val)
{
    int i, digit, shift;
    /* clear digit display */
    LCD_clr_digits();
    shift = 12;
    for (i = 0; i < 4; i++)
    {
        digit = ((val >> shift) & 0x0f);
        shift -= 4;
        LCD_putdigit(digit, i);
    }
}

/*
 * \brief Display float number on 4-digit numeric
 * Mantisse: 2 digits
 * Decimal: 2 digits
 *
 */
void LCD_putf(float f)
{
	char s[6];
	
	sprintf(s, "%+06.2f", (double)f);
	
	LCD_putdigit(s[1]-'0', 0);
	LCD_putdigit(s[2]-'0', 1);
	LCD_putdigit(s[4]-'0', 2);
	LCD_putdigit(s[5]-'0', 3);
	
	LCD_setsymbol(LCD_SYMBOL_POINT, 1); /* always the decimal point */
	LCD_setsymbol(LCD_SYMBOL_MINUS, (s[0]=='-')?1:0);
}

/*
 * \brief Put String to 7-digit line
 * Max length: 7
 *
 */
void LCD_puts(const char *s)
{
	uint8_t i;

	i = 0;
	do
	{
		LCD_putchar(*s++, i++);
	} while (*s && (LCD_NBDIGITS14 > i));
}

void LCD_setsymbol(LCD_symbol_t symbol, uint8_t state)
{
	uint8_t *reg;
	uint8_t bit;

	/** this is dangerous: Enum used as array index requires the
	 * compiler to start enum at zero
	 * avr-gcc does this at the moment
	 * TODO: re-implement as searching for the symbol by array iteration
	 */
	reg = LCD_symbol_table[symbol].reg;
	bit = LCD_symbol_table[symbol].bitpos;

	if (state)
	{
		*reg |= (1<<bit);
	}
	else
	{
		*reg &= ~(1<<bit);
	}
}

void LCD_setbatt(uint8_t level)
{
	uint8_t *reg;

	for(uint8_t i=0;i<4;i++){
		reg = (uint8_t*)(&LCDDR03 + 5*i);
		if(i <= level){
			*reg |= (1<<4);
		}else{
			*reg &= ~(1<<4);
		}
	}
}

void LCD_setantenna(uint8_t level)
{

}

/* EOF */
