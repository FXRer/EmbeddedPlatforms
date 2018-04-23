
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "can_proto.h"
#include "caraca.h"
#include "caraca_client.h"

#include <sys/uio.h>   /* struct iov */

static int     csfd = -1;  /* server connection */

int rec_fd(int spipefd, ssize_t (*callback)(int, const void *, size_t));

/* opens connection to caracad and receives fd for direct CAN-Bus writes
 */

static int caraca_get_can_fd(int fd, char *prog)
{
	struct iovec   iov[3];
        char           type = CARACA_CMSG_GET_CAN_FD;
	
	iov[0].iov_base = &type;
	iov[0].iov_len  = 1;
	iov[1].iov_base = prog;
	iov[1].iov_len  = strlen(prog)+1;   /* trailing \0 */
	if (writev(fd, &iov[0], 2) != iov[0].iov_len + iov[1].iov_len)
		fprintf(stderr, "error: writing to caracad");

	return( rec_fd(fd, write) );
}

/*--------- main -----------------------------------------------------
 * set filter for keys
 * press, store time, set timer for 300ms, start dimming thread 
 * release: stop dimming thread, store light val?


 */

int main(int argc, char *argv[])
{
	int      fd,i,bit;
        canmsg_t tx;
	fd_set rfds;
	struct timeval tv;
	int retval;

	if ((csfd = caraca_open(argv[0])) <= 0) exit(EXIT_FAILURE); 
	
	if ( (fd = caraca_get_can_fd(csfd, argv[0])) >= 0) {
		set_filter(csfd, CAN_ID(0,14),CAN_ID(0,31),0);
		FD_ZERO(&rfds);
		FD_SET(csfd, &rfds);
		tv.tv_sec = 5;
		tv.tv_usec = 350000;
                retval = select(csfd+1, &rfds, NULL, NULL, &tv);	
		if (retval)
			printf("Daten sind jetzt da.\n");
		/* FD_ISSET(0, &rfds) müßte jetzt true sein. */
		else
			printf("Keine Datein innerhalb von fünf Sekunden.\n");

		
		tx.id = CAN_ID(15,2);
		tx.length = 1;
		tx.flags = 0;
		for(i=0;i<40;i++){
			tx.data[0] = i*3;
			if (can_Send(fd, &tx) < 0)
				printf("error writing to CAN-Bus");
			usleep(100000);
		}
		for(i=40;i>=0;i--){
			tx.data[0] = i*3;
			if (can_Send(fd, &tx) < 0)
				printf("error writing to CAN-Bus");
			usleep(100000);
		}
		close(fd);
	}
	exit(0);
}

