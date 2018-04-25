/* Copyright (c) 2015 Axel Wachtler
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
/* Example for use of the One-Wire functions */
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "board.h"
#include "ioutil.h"
#include "ow.h"
#include "hif.h"
#include "xmpl.h"


/* === macros =============================================================== */
#define PROMPT() PRINT(EOL"ow>>> ")
#define LINE_SIZE (80)
#define MAX_ARGS  (16)

/* === globals ============================================================== */
char *argv[MAX_ARGS];
int argc;

/* === prototypes =========================================================== */
bool process_input(int c);
void process_commands(char **argv, int argc);

/* === implementation ======================================================= */
int main(void)
{
 const uint32_t br = HIF_DEFAULT_BAUDRATE;
 int chr;

    mcu_init();
    /* setting up UART and adjusting the baudrate */
    hif_init(br);
    LED_INIT();
    LED_SET(0);
    MCU_IRQ_ENABLE();
#if HIF_TYPE == HIF_AT90USB
    /*
     * Wait for terminal user pressing a key so there is time to
     * attach a terminal emulator after the virtual serial port has
     * been established within the host OS.
     */
    do
    {
        inchar = hif_getc();
    }
    while (EOF == inchar);
#endif
    ow_init();

    /* using the basic hif_xxx functions */
    PRINTF(EOL"1-Wire Example : %s : %ld bit/s"EOL, BOARD_NAME, br);

    PROMPT();
    do
    {
        chr = hif_getc();
        if (process_input(chr))
        {
            if (argc)
            {
                process_commands(argv, argc);
            }
            PROMPT();
        }
    }
    while (1);

}

bool process_input(int c)
{
    static char line[LINE_SIZE];
    static int idx;

    bool rv = false;
    if (c != EOF)
    {
        if (c == '\n' || c == '\r')
        {
            line[idx] = 0;
            idx = 0;
            hif_putc('\n');
            argc = hif_split_args(line, MAX_ARGS, argv);
            rv = true;
        }
        else if (idx < sizeof(line))
        {
            line[idx++] = c;
            hif_putc(c);
        }
        else
        {
            /* buffer full, throw away */
            idx = 0;
        }
    }

    return rv;

}

void process_commands(char **argv, int argc)
{
    uint8_t wbuf[32], rbuf[32], i;

    for (i = 1; i < argc; i++)
    {
        wbuf[i-1] = (uint8_t) strtol(argv[i], NULL, 16);
    }
    switch (argv[0][0])
    {
        case 'R':
            i = ow_reset();
            PRINTF("ow_reset: %d"EOL, i);
            break;

        case 'r':
            PRINTF("READ rlen=%d:", wbuf[0]);
            for(i = 0; i < wbuf[0]; i++)
            {
                PRINTF("%02x ", ow_byte_read());
            }
            break;
        case 'w':
            PRINT("WRITE: ");
            for (i = 0; i < argc-1; i++)
            {
                PRINTF("%02x ", wbuf[i]);
                ow_byte_write(wbuf[i]);
            }
            break;

        case 's':
            PRINT("SEARCH:\nnb: a7 a6 a5 a4 a3 a2 a1 a0\n");
            uint8_t first = 1;
            uint8_t dev_present;
            uint8_t devnb = 0;
            do
            {
                dev_present = ow_master_searchrom((ow_serial_t*)rbuf, first);
                first = 0;
                PRINTF("% 2d:"\
                       " %02x %02x %02x %02x"\
                       " %02x %02x %02x %02x"EOL,
                        devnb++,
                        rbuf[7], rbuf[6], rbuf[5], rbuf[4],
                        rbuf[3], rbuf[2], rbuf[1], rbuf[0]);
                if ( !ow_crc_valid(rbuf, 8) )
                {
                    PRINT("invalid CRC - exit search\n");
                    break;
                }
            }
            while(dev_present);
            break;

        case 'h':
            PRINT("R             : reset"EOL\
                  "r <n>         : read <n> bytes"EOL\
                  "w <a> <b> ... : write byte <a>, <b>, ..."EOL\
                  "s             : search devices"EOL\
                  "h             : show help"EOL
        );

        default:
            PRINTF("invalid command: \"%s\""EOL, argv[0]);
            break;

    }
    PRINT(EOL);

}

/* E_O_F */

