#ifndef _CARACAD_H_
#define _CARACAD_H_

#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "can_proto.h"
#include "config.h"
#include "caraca.h"

/* debugging stuff */
#ifdef DEBUG
extern int	debug_level;
#define DEBUG_TRACE	0x001
#define DEBUG_MSGS	0x002
#define DEBUG_ARGS	0x004
#define DEBUG_CONNS	0x008
#define DEBUG_FILTER	0x020
#define DEBUG_CONFIG	0x040
#define DEBUG_STATS	0x100
#define DEBUG_SHELL	0x400
#define DEBUG_PARSE	0x800
#define DEBUG_ANY	0xffff
#endif
extern int	syslog_level;

/* Name of file which stores saved caracad state info, for restarting */
#define	DEFAULT_STATUS_FILE	"caracad.status"

/* We support simple (plaintext password) and kerberos authentication */
#define	AUTH_SIMPLE	1
#define	AUTH_KERBEROS	2

/*
 * ****************************************************************************
 * Data types 
 * ****************************************************************************
 */


typedef struct client_info_struct 
{  
	int   fd;  
	pid_t pid;
	char  name[40];   
} Client;

struct can_filter {
	int id,mask;
	struct can_filter *next;  /* where to go next, if packet don't match */
	Client *client;           /* where to send matching  packet */
	int flags;
	int counter;     
};

	
#endif /* _CARACAD_H_ */

