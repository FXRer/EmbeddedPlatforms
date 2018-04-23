#ifndef	_AVRGANG_H
#define	_AVRGANG_H

#include "rs232int.h"

#define CAN_MSG_LENGTH 8

#define MSG_RTR (1<<0)
#define MSG_OVR (1<<1)
#define MSG_EXT (1<<2)

/**
typedef struct {
	int      flags;
	//int      cob;		// can object slot, not used
	unsigned long id;
	//struct timeval  timestamp;
	short length;
	unsigned char data[CAN_MSG_LENGTH]; 

} canmsg_t;
**/

#endif