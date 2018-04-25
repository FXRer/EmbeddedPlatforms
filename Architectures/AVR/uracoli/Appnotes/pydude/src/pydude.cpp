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
/**
 * @file
 * @brief Implementation of methods for class AvrDude.
 */
#include "pydude.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 1
# define DBG(fmt, ...) printf(fmt, __VA_ARGS__)
#else
# define DBG(fmt, ...)
#endif

void AvrDude::dump(void)
{
    printf(" - LF=0x%02x\n"
           " - HF=0x%02x\n"
           " - EF=0x%02x\n"
           " -  F=0x%02x\n",
            this->safemode_lfuse,
            this->safemode_hfuse,
            this->safemode_efuse,
            this->safemode_fuse);
}


void AvrDude::set_programmer(char * pgmname)
{
    this->pgm = locate_programmer(programmers, pgmname);
    DBG("pgm=%p\n", this->pgm );
}

void AvrDude::set_part(char * partname)
{
struct avrpart * p;
    p = locate_part(part_list, partname);
    if (p!= NULL)
    {
        this->part = avr_dup_part(p);
        this->vpart = avr_dup_part(p);
        DBG("part=%p, vpart=%p\n", this->part,  this->vpart);
    }
    else
    {
        DBG("Error Locate Part %p, name=%s\n", p, partname);
    }
}


void AvrDude::open(char * portname)
{
int rc;
    rc = this->pgm->open(this->pgm, portname);
    DBG("Open PGM: %d, port=%s\n", rc, portname);

    /* enable the programmer */
    this->pgm->enable(this->pgm);

    /* turn off all the status leds */
    this->pgm->rdy_led(this->pgm, OFF);
    this->pgm->err_led(this->pgm, OFF);
    this->pgm->pgm_led(this->pgm, OFF);
    this->pgm->vfy_led(this->pgm, OFF);

    /* initialize the chip in preperation for accepting commands */
    rc = this->pgm->initialize(this->pgm, this->part);
    if (rc < 0) {
        DBG("%s: initialization failed, rc=%d\n", progname, rc);
#if 0
        if (!ovsigck) {
          DBG("%sDouble check connections and try again, "
              "or use -F to override\n"
              "%sthis check.\n\n", progbuf, progbuf);
          exitrc = 1;
          goto main_exit;
        }
#endif
    }

    /* indicate ready */
    this->pgm->rdy_led(this->pgm, ON);
}


void AvrDude::close(void)
{
    this->pgm->powerdown(this->pgm);
    this->pgm->disable(this->pgm);
    this->pgm->rdy_led(this->pgm, OFF);
    this->pgm->close(this->pgm);
}


void AvrDude::part_check_signature(memblock_t *pmemblk)
{


    this->sig = avr_locate_mem(this->part, "signature");
    pmemblk->size = this->sig->size;
    pmemblk->buffer = (uint8_t*)malloc(this->sig->size);


    if (this->sig != NULL)
    {
        int i,ff, zz;
        ff = zz = 1;
        for (i=0; i<this->sig->size; i++)
        {
            pmemblk->buffer[i] = this->sig->buf[i];
            DBG("0x%02x ",this->sig->buf[i]);
            if (this->sig->buf[i] != 0xff)
                ff = 0;
            if (this->sig->buf[i] != 0x00)
                zz = 0;
        }
        if (ff || zz) {
              DBG("%s: Yikes!  Invalid device signature.\n", progname);
        }
    }
}


void AvrDude::read_fuses(void)
{
    int rc;
    rc = safemode_readfuses( &this->safemode_lfuse, &this->safemode_hfuse,
                             &this->safemode_efuse, &this->safemode_fuse,
                             this->pgm, this->part, this->verbose);
}

void AvrDude::part_update(char *updop)
{
int rc;
AVRMEM * m;
    this->update =  parse_op(updop);
    m = avr_locate_mem(this->part, this->update->memtype);
    if (m == NULL) return;
    rc = do_op(this->pgm, this->part, this->update, 1, 0);
}


void AvrDude::part_read(char *memtype, uint32_t addr, uint32_t len, memblock_t *pmemblk)
{
    AVRMEM * mem;
    uint32_t maxsize, i;
    int rc;

    /* check memory type */

    mem = avr_locate_mem(this->part, memtype);
    if (mem == NULL)
    {
        /* throw exception */
        return;
    }

    /* validate and correct address information */
    maxsize = mem->size;

    if(addr > maxsize)
    {
        /* throw exception */
        return;
    }

    if((addr+len) > maxsize)
    {
        len = maxsize - addr;
    }

    pmemblk->buffer = (uint8_t*)malloc(len);
    pmemblk->size = (size_t)len;


    for(i=0;i<len;i++)
    {
        rc = pgm->read_byte(this->pgm, this->part, mem, addr+i, &pmemblk->buffer[i]);
        if(rc < 0)
        {
            /* throw exception */
            return;
        }
    }
}

void AvrDude::part_write(char *memtype, uint32_t addr, memblock_t *pmemblk)
{
    AVRMEM * mem;
    int rc, werror;
    uint32_t maxsize, i;
    uint8_t tmp;
    if(strstr(memtype, "fu")!=NULL)
    {
        /* throw exception ... in this early state we do not mess around with fuses */
        fprintf(stderr, "Fuses still unsupported");
        return;
    }

    mem = avr_locate_mem(this->part, memtype);
    if (mem == NULL)
    {
        /* throw exception */
        return;
    }

    /* validate and correct address information */
    maxsize = mem->size;

    if(addr > maxsize)
    {
        /* throw exception */
        return;
    }

    if((addr+pmemblk->size) > maxsize)
    {
        /* throw exception like avrdude */
        return;
        //len = maxsize - addr;
    }


    this->pgm->err_led(pgm, OFF);
    for (werror=0, i=0; i<pmemblk->size; i++)
    {
        rc = avr_write_byte(this->pgm, this->part, mem, addr+i, pmemblk->buffer[i]);
        if (rc)
        {
            fprintf(stderr, "%s (write): error writing 0x%02x at 0x%05x, rc=%d\n",
                    progname, pmemblk->buffer[i], addr+i, rc);
            if (rc == -1)
            {
                fprintf(stderr,
                        "write operation not supported on memory type \"%s\"\n",mem->desc);
            }
            werror = 1;
        }

        rc = pgm->read_byte(this->pgm, this->part, mem, addr+i, &tmp);
        if (tmp != pmemblk->buffer[i])
        {
            fprintf(stderr,
                    "%s (write): error writing 0x%02x at 0x%05x cell=0x%02x\n",
                    progname, pmemblk->buffer[i], addr+i, tmp);
            werror = 1;
        }

        if (werror)
        {
            this->pgm->err_led(pgm, ON);
        }
        printf("%02x ",pmemblk->buffer[i]);

    }

}
