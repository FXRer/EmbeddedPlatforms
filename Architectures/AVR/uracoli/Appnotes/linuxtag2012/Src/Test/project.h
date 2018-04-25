/* Copyright (c) 2011 Joerg Wunsch
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

#include <stdint.h>
#include <stdbool.h>

/*
 * row low, column high => LED is green
 * row low, column low  => LED is off, pad scan is off
 * row high, column low => LED is red
 * row high, column high => LED is off, pad scan is on
 */
typedef enum
{
    LED_OFF, LED_RED, LED_GREEN
}
    led_color_t;

typedef struct
{
    uint8_t row, col;
}
    row_col_t;

#define NCOLS 3
#define NROWS 3


/* == radio related macros == */

#define FCF(type,ack) (                                                 \
  (2 << 14)                     /* source address mode: short */ |      \
  (2 << 10)                     /* destination address mode: short */ |\
  (1 << 6)                      /* PAN ID compression */ |\
  (ack << 5)                    /* ack request */ |\
  (type)                        /* frame type */ \
                       )

#define TYPE_BEACON 0
#define TYPE_DATA   1
#define TYPE_ACK    2
#define TYPE_CMD    3           /* MAC command */

#define CHANNEL 17
#define SHORTADDR 0x4214
#define PANID 0xF00F
#define TRX_PRINTF(fmt, ...) trx_printf(FLASH_STRING(fmt), __VA_ARGS__)
#define TRX_PRINT(fmt) trx_printf(FLASH_STRING(fmt))

/** Max. number of payload bytes per frame */
#define PAYLD_SIZE        116
/** Number of bytes for CRC16 */
#define CRC_SIZE          (sizeof(crc_t))
/** Maximum frame size */
#define UART_FRAME_SIZE   (sizeof(prot_header_t) +      \
                           PAYLD_SIZE + CRC_SIZE )
/** Index of first payload byte */
#define PAYLD_START       (sizeof(prot_header_t))
/** Index of last payload byte */
#define PAYLD_END         (UART_FRAME_SIZE - CRC_SIZE)

typedef struct
{
  uint16_t fcf;   /**< Frame control field*/
  uint8_t  seq;   /**< Sequence number */
  uint16_t panid; /**< 16 bit PAN ID */
  uint16_t dst;   /**< 16 bit destination address */
  uint16_t src;   /**< 16 bit source address */
} prot_header_t;

typedef uint16_t crc_t;


extern led_color_t leds[NROWS][NCOLS];
extern bool pads[NROWS][NCOLS];
extern volatile row_col_t last_pad;    /* row, column of last pad that changed state */
extern volatile bool pad_toggled;
extern void update_pads(void);

