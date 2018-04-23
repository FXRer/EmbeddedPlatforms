/*
caraca.h

Author: Konrad Riedel k.riedel@gmx.de 
*/

/*
 * $Id: caraca.h,v 1.1 2000/08/21 22:00:13 skr Exp $
 * $Log: caraca.h,v $
 * Revision 1.1  2000/08/21 22:00:13  skr
 * restructuring
 *
 * Revision 1.2  2000/07/24 13:49:12  skr
 * Caracad - receive CAN message vi perl
 *
 * Revision 1.1  2000/05/05 16:17:21  skr
 * caracad added
 *
 */

#ifndef  CARACA_H
#define  CARACA_H

#include   <sys/types.h>
#include   <errno.h>
#include   <fcntl.h>

/* Name of Stream Pipe */
#define CS_NAME   "/tmp/caracad"

/* Definition of message types.  New values can be added, but old values
   should not be removed or without careful consideration of the consequences
   for compatibility.  The maximum value is 254; value 255 is reserved
   for future extension. */
/* Message name */                      /* msg code */  /* arguments */
#define CARACA_MSG_NONE                         0       /* no message */
#define CARACA_MSG_DISCONNECT                   1       /* cause (string) */
#define CARACA_CMSG_RTR                         'R'     /* data (2 int) */
#define CARACA_CMSG_FILTER                      'F'     /* */
#define CARACA_CMSG_SEND_CAN_MSG                'S'     /* data (canmsg_t) */
#define CARACA_CMSG_DUMP_FILTERS                'D'     /* data (2 int) */
#define CARACA_SMSG_SUCCESS                     14      /* */
#define CARACA_SMSG_FAILURE                     15      /* */
#define CARACA_SMSG_EXITSTATUS                  20      /* status (int) */
#define CARACA_MSG_DEBUG                        36      /* string */

/* for local connections via unix sockets */
#define CARACA_CMSG_GET_CAN_FD                  'f'      /* data (string) */
#define CARACA_SMSG_CAN_FD                      17      /* data (fd) */


#endif
