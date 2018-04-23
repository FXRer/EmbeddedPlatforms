/*
 * globals.h - definition of structure holding global data.
 */
#include "caracad.h"


typedef struct globals {
	Client *client;         /* adress of array */
	int client_cnt;         /* entries in client[] */
        struct can_filter *first_can_filter;	
        struct can_filter *last_can_filter;	
	/* The name of the config file */
	char *configfile;
	/* How to write to the CAN-Bus */
	char *can_device;
	int can_fd;
	int can_wr_fd;
	/* Name of caracad status file (timestamp of last replog */
	char caracad_status_file[ MAXPATHLEN ];
	/* Name of program */
	char *myname;
	/* TCP port to use */
	unsigned short port;
} Globals;


extern Globals *sglob;
