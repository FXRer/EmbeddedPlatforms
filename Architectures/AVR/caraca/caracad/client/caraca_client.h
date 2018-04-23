/*      $Id: caraca_client.h,v 1.2 2001/08/22 12:09:49 skr Exp $      */

/****************************************************************************
 ** caraca_client.h ***********************************************************
 ****************************************************************************
 *
 * caraca_client - common routines for caracad clients
 *
 * Copyright (C) 2000 Konrad Riedel <k.riedel@gmx.de>
 *
 */ 
 
#ifndef CARACA_CLIENT_H
#define CARACA_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define CARACA_ALL ((char *) (-1))
#define CAN_ID(a,b) ((a)+((b)<<6))

enum caraca_flags {none=0x00,
		 once=0x01,
		 quit=0x02,
		 mode=0x04,
		 ecno=0x08
};

struct caraca_list
{
	char *string;
	struct caraca_list *next;
};

struct caraca_code
{
	char *remote;
	char *button;
	struct caraca_code *next;
};

struct caraca_config
{
	char *current_mode;
	struct caraca_config_entry *next;
	struct caraca_config_entry *first;
};

struct caraca_config_entry
{
	char *prog;
	struct caraca_code *code;
	unsigned int rep;
	struct caraca_list *config;
	char *change_mode;
	unsigned int flags;
	
	char *mode;
	struct caraca_list *next_config;
	struct caraca_code *next_code;

	struct caraca_config_entry *next;
};

int caraca_open(const char *prog);
int set_filter(int fd, int id, int mask, int flags);

#ifdef __cplusplus
}
#endif

#endif
