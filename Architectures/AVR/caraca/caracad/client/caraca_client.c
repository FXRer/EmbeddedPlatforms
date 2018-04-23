
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "can_proto.h"
#include "caraca.h"
#include "caraca_client.h"
#include <sys/uio.h>   /* struct iov */

#define PORT   7776

int set_filter(int fd, int id, int mask, int flags)
{
	char buf[20];
	char *ptr;
	ptr = buf;
	*ptr++ = CARACA_CMSG_FILTER;
	*ptr++ = ' ';
	ptr += sprintf(ptr,"%d,",id);
	ptr += sprintf(ptr,"%d,",mask);
	ptr += sprintf(ptr,"%d",flags);
	*ptr++ = 0;
	/*printf("debug: %d %.20s \n",buf[0],buf);*/
	return(write(fd, buf, ptr - buf));
}

/* opens connection to caracad and receives fd for direct CAN-Bus writes
 */

int caraca_open(const char *prog)
{
	int                  fd, len;
	struct sockaddr_in   inet_adr;
	struct in_addr       inadr;
	struct hostent       *caraca_server;

        caraca_server = gethostbyname("pandora");

	if (caraca_server == NULL) {
		return(-1);
	}

	inet_adr.sin_family = AF_INET;
	inet_adr.sin_port   = htons(PORT);
	memcpy(&inet_adr.sin_addr, caraca_server->h_addr_list[0],
			sizeof(inet_adr.sin_addr));

	if ( (fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return(-2);

	if (connect(fd, (struct sockaddr *) &inet_adr, sizeof(inet_adr)))
		return(-3);

	return(fd);
}

