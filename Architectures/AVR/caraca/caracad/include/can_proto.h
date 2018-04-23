 /* the can message structure
  * from can4linux             */
#include   <sys/time.h>

#define CAN_MSG_LENGTH 8

#define MSG_RTR (1<<0)
#define MSG_OVR (1<<1)
#define MSG_EXT (1<<2)

 typedef struct {
	int             flags;
	int             cob;		/* can object slot, not used*/
	unsigned   long id;
        //unsigned   long timestamp;
	struct timeval  timestamp;
	short      int  length;
	unsigned   char data[CAN_MSG_LENGTH]; 

 } canmsg_t;

int can_Send(int fd, canmsg_t *Tx);
int can_Receive(int fd, canmsg_t *Rx);
int can_Init(char *device,int flags);
