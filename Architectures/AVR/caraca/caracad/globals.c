/*
 * globals.c - initialization code for global data
 */

#include <stdio.h>

#include "caracad.h"
#include "globals.h"

Globals		 *sglob;

int syslog_level = LOG_DEBUG;
int debug_level = 0;


/*
 * Initialize the globals
 */
Globals *init_globals()
{
    Globals *g;

    g = ( Globals * ) malloc( sizeof( Globals ));
    if ( g == NULL ) {
	return NULL;
    }
    g->client = malloc(REALLOC_CNT * sizeof(Client));
    g->client_cnt = REALLOC_CNT;
    /*g->configfile = DEFAULT_CONFIGFILE;*/
    g->can_device = DEFAULT_CAN_DEV;
    g->myname = NULL;
    g->last_can_filter = NULL;
    g->first_can_filter = NULL;
    g->port = 7776;
    return g;
}
