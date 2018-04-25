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
/* $Id$ */
#ifndef PYDUDE_H
#define PYDUDE_H (1)
#include <stdint.h>
#include "avr.h"
#include "config.h"
#include "avrdude.h"
#include "lists.h"
#include "config.h"
#include "update.h"
#include "safemode.h"


typedef struct
{
    size_t size;
    uint8_t *buffer;
} memblock_t;


class AvrDude
{
    private:
        PROGRAMMER     * pgm;
        struct avrpart * part;
        struct avrpart * vpart;
        AVRMEM         * sig;
        UPDATE         * update;
    public:
        int verbose;
        unsigned char safemode_lfuse;
        unsigned char safemode_hfuse;
        unsigned char safemode_efuse;
        unsigned char safemode_fuse;
        void dump(void);
        void set_programmer(char * pgmname);
        void set_part(char * partname);
        void open(char * portname);
        void close(void);

        void part_check_signature(memblock_t *dataout);
        void read_fuses(void);
        void part_update(char *updop);
        void part_read(char *memtype, uint32_t addr, uint32_t len, memblock_t * dataout);
        void part_write(char *memtype, uint32_t addr, memblock_t *datain);
};

#endif
