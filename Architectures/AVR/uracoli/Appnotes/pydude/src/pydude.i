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
%module pydude
%{
    #include "pydude.h"
    #include <stdio.h>
    #include <string.h>
    #include "avr.h"
    #include "config.h"

    #include "avrdude.h"
    #include "lists.h"
    #include "config.h"
    
    char *progname   = "pydude";  
    char progbuf[20] = "      ";


    int    do_cycles = 0;   /* track erase-rewrite cycles */
    int    verbose = 0;     /* verbose output */
    int    quell_progress = 0; /* un-verebose output */
    int    ovsigck = 0;     /* 1=override sig check, 0=don't */
    
    PROGRAMMER     * pgm;
    
    char    sys_config[PATH_MAX]; /* system wide config file */
    char    usr_config[PATH_MAX]; /* per-user config file */

%}

%apply unsigned char  {uint8_t};
%apply unsigned short {uint16_t};
%apply unsigned long  {uint32_t};
#ifdef SWIGPYTHON
/* typemap for handling of an output buffers, which is 
   allocated inside a function */

%typemap(in, numinputs=0)(memblock_t * dataout)
{
    memblock_t memblk;
    memblk.size = 0;
    memblk.buffer = NULL;
    $1 = &memblk;
}

%typemap(argout, fragment="t_output_helper") (memblock_t * dataout)
{
    size_t i;
    long l;
    PyObject *list = PyList_New($1->size);
    for(i=0;i<$1->size;i++)
    {
        l = (long) $1->buffer[i];
        PyList_SetItem(list, i, PyInt_FromLong(l));
    }
    $result = t_output_helper($result, list);
}

%typemap(freearg) (memblock_t * dataout)
{
    if ($1->buffer != NULL)
    {
        free($1->buffer);
    }
}

/* data input */
%typemap(in)(memblock_t * datain)
{
    memblock_t memblk;
    size_t i;
    PyObject *item;
    memblk.buffer = NULL;
    memblk.size = 0;
        
    if (!PyList_Check($input)) 
    {
        PyErr_SetString(PyExc_ValueError, "Expected a list");
        return NULL;
    }
    
    memblk.size = PyList_Size($input);
    memblk.buffer = (uint8_t*)malloc(memblk.size);
    for(i = 0;i<memblk.size;i++)
    {
        item = PyList_GetItem($input, i);
        if (!PyInt_Check(item))
        {
            PyErr_SetString(PyExc_ValueError, "Expected a list of integers");
            free(memblk.buffer);
            memblk.buffer = NULL;
            memblk.size = 0;
            return NULL;
        }
        memblk.buffer[i] = (uint8_t) PyInt_AsLong(item);
    }
    $1 = &memblk;
}

%typemap(freearg) (memblock_t * dataout)
{
    if ($1->buffer != NULL)
    {
        free($1->buffer);
    }
}
#endif

/* wrapped variables and stuff */
%include "pydude.h"
%ignore memblock_t;

int verbose;
//char sys_config[PATH_MAX];
//char usr_config[PATH_MAX];

/* low level avr routines */
int read_config(const char * file);

%init
%{
    init_config();
%}
