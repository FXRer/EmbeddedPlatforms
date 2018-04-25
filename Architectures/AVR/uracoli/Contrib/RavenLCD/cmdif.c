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

/**
 * @file
 * @brief Command interface for Raven LCD Application
 *
 * @ingroup grpAppRavenLCD
 */

/* === includes ============================================================ */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "cmdif.h"
#include "lcd.h"
#include "analog.h"

/* === macros ============================================================== */
#define MAXLINELEN (80)

/* === types =============================================================== */

/* === globals ============================================================= */

static uint8_t lnbuf[MAXLINELEN + 1];

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/*
 * \brief Wait for complete line, no character echoing
 *
 * @return 1 for line completed, 0 else
 */
static inline uint8_t getline()
{
	uint16_t inchar;
	static uint8_t idx = 0;
    uint8_t ret = 0;

	inchar = getchar();
	if (EOF != inchar) {
		lnbuf[idx] = 0x00; /* NULL terminated string */
		if ((inchar == '\n') || (inchar == '\r')) {
            ret = (idx > 1) ? 1 : 0;
			idx = 0;
		}
		else if ( (inchar >= 0x20) && (idx < MAXLINELEN) ){
			lnbuf[idx++] = inchar;
		}
		else {
			/* TODO: catch line length overflow */
		}
	}

	return ret;
}

/**
 * @brief Split a null terminated string.
 *
 * This function creates argc,argv style data from a null
 * terminated string. The splitting is done on the base of
 * spaces (ASCII 32).
 *
 * @param  txtline  string to split
 * @param  maxargs  maximum number of arguments to split
 * @retval argv     array of pointers, that store the arguments
 * @return number of arguments splitted (argc)
 */
static inline int split_args(char *txtline, int maxargs, char **argv)
{
	uint8_t argc = 0, nextarg = 1;

	while ((*txtline != 0) && (argc < maxargs)) {
		if (*txtline == ' ') {
			*txtline = 0;
			nextarg = 1;
		}
		else {
			if (nextarg) {
				argv[argc] = txtline;
				argc++;
				nextarg = 0;
			}
		}
		txtline++;
	}

	return argc;
}

/*
 * \brief Command to echo a string
 * This is intended to check the UART connection
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_echo(char **params)
{
	printf("%s\n", params[0]);
}

static inline void cmd_batt(char **params)
{
	uint8_t battlevel = atoi(params[0]);

	LCD_setbatt(battlevel);
}

/*
 * \brief Put String to 7x14-segment digits
 *
 */
static inline void cmd_print(char **params)
{
	LCD_puts(params[0]);
}

/*
 * \brief Put 16-bit integer to 4x7-segment digits
 *
 */
static inline void cmd_puti(char **params)
{
	int16_t i = atoi(params[0]);
	LCD_puti(i);
}
/*
 * \brief Put 16-bit integer to 4x7-segment digits
 *
 */
static inline void cmd_putx(char **params)
{
	uint16_t i = (uint16_t)strtol(params[0], NULL, 16);
	LCD_puthex(i);
}

/*
 * \brief Put float to 4x7-segment digits
 * Fixed format: +2.2
 *
 */
static inline void cmd_putf(char **params)
{
	float f;
	sscanf(params[0], "%f", &f);
	LCD_putf(f);
}

static inline void cmd_listsymbols(char **params)
{
	uint8_t i;
	uint8_t size;
	LCD_symbol_container_t *symtable;

	size = LCD_getsymboltable(&symtable);

	//printf("Available symbols:\n");
	for (i = 0; i < size; i++) {
		printf("%02d %s\n", i, symtable[i].name);
	}
}

static inline void cmd_symbol(char **params)
{
	uint8_t i;
	uint8_t size;
	LCD_symbol_container_t *symtable;
	uint8_t state;
	uint8_t found;

	size = LCD_getsymboltable(&symtable);

	state = (uint8_t)strtoul(params[1], (void*)0x0000, 10);
	found = 0;
	for(i=0; (i < size) && (0 == found); i++){
		if(0 == strcasecmp((const char*)symtable[i].name, (const char*)params[0])){
			LCD_setsymbol(symtable[i].symbol, state);
			found=1;
		}
	}
	if(0 == found){
		printf("ERR Unknown Symbol\n");
	}
}

static inline void cmd_cls(char **params)
{
	LCD_cls();
}

static inline void cmd_smp(char **params)
{
	joystick_sampling_stop();

	audio_sampling_start(5000);
}

static inline void cmd_temp(char **params)
{
	temperature_start();
}

static inline void cmd_vmcu(char **params)
{
	vmcu_start();
}

static inline void cmd_lcdoff(char **params)
{
	LCD_disable();
}

/* forward declaration */
static inline void cmd_help(char **params);

/* List of commands that are available at the command interface */
const struct
{
	const char *name; /**< Name of the command */
	void (*execfunc)(char**); /**< Function to be called */
	uint8_t nbparams; /**< Expected number of parameters */
	const char *description; /**< Textual description of the command to print help screen */
} commands[] = {
		{ "echo", cmd_echo, 1, "Echo a string" },
		{ "batt", cmd_batt,	1, "Set battery symbol" },
		{ "print", cmd_print, 1, "Print ascii to line" },
		{ "listsymbols", cmd_listsymbols, 0, "List all available symbols" },
		{ "sym", cmd_symbol, 2, "Set symbol <symbol> <state>" },
		{ "cls", cmd_cls, 0, "Clear Screen" },
		{ "puti", cmd_puti, 1, "Display integer" },
		{ "putx", cmd_putx, 1, "Display hex" },
		{ "putf", cmd_putf, 1, "Display float" },
		{ "smp", cmd_smp, 0, "Start microphone sampling" },
		{ "temp", cmd_temp, 0, "Get temperature" },
		{ "vmcu", cmd_vmcu, 0, "Get MCU voltage" },
		{ "lcdoff", cmd_lcdoff, 0, "Turn off LCD" },
		{ "?", cmd_help, 0, "Help on commands" },
};

static inline void cmd_help(char **params)
{
	uint8_t i;
	printf("Help on commands:\n");
	for(i=0;i<sizeof(commands)/sizeof(commands[0]);i++) {
		printf("%s %s\n", commands[i].name, commands[i].description);
	}
}

/*
 * \brief Parsing a shell input line
 *
 * @param *ln String containing the command line to parse
 */
static inline void process_cmdline(char *ln)
{
	char *params[10];
	uint8_t nbparams;
	uint8_t i;
	uint8_t found = 0;

	nbparams = split_args(ln, 10, params);

	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		if (!strcasecmp(params[0], commands[i].name)) {
			if (commands[i].nbparams == (nbparams - 1)) { /* substract the command itself */
				commands[i].execfunc(&params[1]);
			}
			else {
				puts_P(PSTR("ERR Parameter"));
			}
			found = 1;
		}
	}

	if (!found) {
		puts_P(PSTR("ERR Unknown command"));
	}
}

void cmdif_task(void)
{
	if (getline()) {
		process_cmdline((char*) lnbuf);
	}
}

/* EOF */
