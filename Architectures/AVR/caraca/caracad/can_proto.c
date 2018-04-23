#include <termios.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "can_proto.h"

static struct termios   alt_terminal;
static enum { RESET, RAW, CBREAK } tty_modus = RESET;

int tty_set(int fd)
{
   struct termios  terminal;

   if (tcgetattr(fd, &alt_terminal) < 0)
      return(-1);

   terminal = alt_terminal;
   
   if (cfsetospeed(&terminal,B38400) < 0)
      return(-1);
   if (cfsetispeed(&terminal,B38400) < 0)
      return(-1);
  
   terminal.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
   terminal.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
   terminal.c_cflag &= ~(CSIZE | PARENB); 
   terminal.c_cflag |= CS8; /* 8 Bits */
   terminal.c_oflag &= ~(OPOST); 

   terminal.c_cc[VMIN] = 1;
   terminal.c_cc[VTIME] = 0;
  
   if (tcsetattr(fd, TCSAFLUSH, &terminal) < 0)
      return(-1);
   return(0);
}

int tty_reset(int fd)
{
   if (tty_modus != CBREAK && tty_modus != RAW)
      return(0);
   if (tcsetattr(fd, TCSAFLUSH, &alt_terminal) < 0)
      return(-1);

   tty_modus = RAW;
   return(0);
}


int can_Init(char *device,int flags)
{
	int fd;
	if (( fd = open(device, flags|O_SYNC)) > 0 ) {
		tty_set(fd);
		return(fd);
	}
	return(-1);
}

static void err_dump(int fd)
{
	unsigned char rx_err_cnt, tx_err_cnt, can_bits, err_code;

	read(fd, &rx_err_cnt, 1);
	read(fd, &tx_err_cnt, 1);
	read(fd, &can_bits, 1);
	read(fd, &err_code, 1);
	syslog(LOG_ERR, "CAN Error: RxErr=%d, TxErr=%d, Bits=0x%02X, Code=%c",
				rx_err_cnt, tx_err_cnt, can_bits, err_code);
}

//wait for starting character
//log all errors
//TODO make error packet (reason, error count, bus state)

static int wait_for_message(int fd)
{
	unsigned char start = 0;
	while (read(fd, &start, 1) == 1)
	{
		if (start == 'C' || start == 'E')
			return start;
		else
			syslog(LOG_ERR, "CAN garbage: %d (0x%02X) %c", start, start, isprint(start) ? start : '.');
	}

	return 0;
}

/* Send/Receive CAN Messages via CANDONGLE
 *
 * * Message is packed:
 *
 * 15   14   13  12  11  10   9   8     7   6   5   4  3  2  1  0
 * Id11 Id10 Id9 Id8 Id7 Id6 Id5 Id4   Id3 Id2 Id1  R N4 N3 N2 N1
 * +---    Function  --+ +---- Node ID     -------+  +--length--+
 */


int can_Receive(int fd,canmsg_t *rx )
{
	/* we need min == 0 and time == 0 */
	/* read Header 2Bytes */
	unsigned char id1,id2,type;
	struct timezone  tz;

	while ( (type = wait_for_message(fd)) == 'E' )
		err_dump(fd);

	if (type == 'C')
	{
		gettimeofday(&(rx->timestamp), &tz);
		/*TODO: get timestamp from candongle - but I think, we don't need this
		* extra precisition*/

		read(fd, &id1, 1);
		read(fd, &id2, 1);
		rx->id = (id1 << 3) + (id2 >> 5);
		rx->length = (id2 & 0x0f);
		if (rx->length > 8) {
			syslog(LOG_ERR, "CAN Message too long: %0d",rx->length);
			rx->length = 8;
		}
		/* read Message */
		read(fd, rx->data, rx->length);
		rx->data[rx->length] = 0;
		rx->flags = 0;

		return 1;
	}
	else
		return 0;
}
/* must be atomic!*/
int can_Send(int fd,canmsg_t *tx)
{
	//extern int errno;
	unsigned char rtr=0;
	char buf[20];
	char *ptr;
	int  id = tx->id;

	ptr = buf;
	*ptr++ = 'C';
	*ptr++ = id >> 3;
	if (tx->flags & MSG_RTR) {
		tx->length=0;
		rtr = 0x10;
	}

	*ptr++ = ((tx->id & 7) << 5) + (tx->length & 0x0f);
	/*i = ((tx->id & 7) << 5) + (tx->length & 0x0f);*/
	/*printf("tx->id %x, Byte: %x\n",(tx->id),i);*/
	memcpy(ptr,tx->data,tx->length);
        ptr+=tx->length;
	*ptr++ = 0x0ff;
	return(write(fd, buf, ptr - buf));
}

