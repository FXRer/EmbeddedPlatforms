/*
 * CARACA - CAn Remote Automation and Control with the Avr
 * http://caraca.sourceforge.net
 *
 *      
 *	Copyright (c) 2000 Konrad Riedel 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*      $Id: sock_proto.c,v 1.7 2000/09/04 19:24:59 skr Exp $ */

#include <sys/socket.h>   /* struct msghdr */
#include <sys/uio.h>      /* struct iovec  */
#include <stddef.h>
#include "caracad.h"

#define LEN_CONTROL  (sizeof(struct cmsghdr) + sizeof(int))

static struct cmsghdr  *cmzgr = NULL;

int addrtoi(char *addr)
{
	int id,f;
	
	sscanf(addr,"%d.%d",&f,&id);
	return (f << 6) + id;
}

int can_fmt_msg(char *res, canmsg_t *rx )
{
	char *buf = res;
	int i;
	
	/* Bits 0..5 Node Addess 6..10 function code */ 
	buf += sprintf(buf,"%d.%d:",(int) rx->id & 0x3f ,(int) (rx->id >> 6) & 0x1f);
	for (i=0;i<rx->length;i++) buf += sprintf(buf,"%d,", rx->data[i]);
	*(buf-1) = '\0';
	return buf-res;
}

int send_can2client(canmsg_t *rx,int fd,int format)
{
	char msgbuf[50];
	return write(fd,msgbuf,can_fmt_msg(msgbuf,rx));	
}

/*----- send_fd --------------------------------------------------------
 *         sends a  file descriptor 
 *         */
int  send_fd(int spipefd , int fd)
{
	struct iovec    iov[1];
	struct msghdr   message;
	char            protokoll[2] = { 0, 0 };

	iov[0].iov_base = protokoll;
	iov[0].iov_len  = 2;
	message.msg_iov     = iov;
	message.msg_iovlen  = 1;
	message.msg_name    = NULL;
	message.msg_namelen = 0;

	if (fd < 0) {
		message.msg_control    = NULL;
		message.msg_controllen = 0;
		protokoll[1] = -fd;        /* != 0 means error */
		if (protokoll[1] == 0)
			protokoll[1] = 1;  
	} else {
		if (cmzgr == NULL && (cmzgr = malloc(LEN_CONTROL)) == NULL)
			return(-1);
		cmzgr->cmsg_level  = SOL_SOCKET;
		cmzgr->cmsg_type   = SCM_RIGHTS;
		cmzgr->cmsg_len    = LEN_CONTROL;
		message.msg_control    = (caddr_t) cmzgr;
		message.msg_controllen = LEN_CONTROL;
		*(int *)CMSG_DATA(cmzgr) = fd;  /* CAN fd to send*/
	}

	if (sendmsg(spipefd, &message, 0) != 2)
		return(-1);

	return(0);
}

/*----- rec_fd -----------------------------------------------------
 *         extra data will be parsed by
 *           (*callback)(STDERR_FILENO, buffer, byte_read)
 *                                                        */
int rec_fd(int spipefd, ssize_t (*callback)(int, const void *, size_t))
{
	int                recfd=0, byte_read, status=-1;
	char               *zgr, buffer[MAX_ZEICHEN];
	struct iovec       iov[1];
	struct msghdr      message;

	while (1) {
		iov[0].iov_base = buffer;
		iov[0].iov_len  = sizeof(buffer);
		message.msg_iov     = iov;
		message.msg_iovlen  = 1;
		message.msg_name    = NULL;
		message.msg_namelen = 0;
		if (cmzgr == NULL && (cmzgr = malloc(LEN_CONTROL)) == NULL)
			return(-1);
		message.msg_control    = (caddr_t) cmzgr;
		message.msg_controllen = LEN_CONTROL;

		if ( (byte_read = recvmsg(spipefd, &message, 0)) < 0)
			syslog(LOG_ERR, "recvmsg-error");
		else if (byte_read == 0) {
			syslog(LOG_WARNING, "connection closed.");
			return(-1);
		}

		for (zgr=buffer; zgr < &buffer[byte_read]; ) {
			if (*zgr++ == 0) {
				if (zgr != &buffer[byte_read-1])
					syslog(LOG_WARNING, "message inconsistant");
				status = *zgr & 0xff;
				if (status == 0) {
					/* receiving file descriptor */
					if (message.msg_controllen != LEN_CONTROL)
						syslog(LOG_WARNING, "message inconsistant");
					recfd = *(int *)CMSG_DATA(cmzgr);
				} else
					/* error occured */
					recfd = -status;
				byte_read -= 2;
			}
		}
		if (byte_read > 0)
			if ( (*callback)(STDERR_FILENO, buffer, byte_read)
					!= byte_read)
				return(-1);
		if (status >= 0)
			return(recfd);
	}
}

int send_err(int spipefd, int status, const char *errmsg)
{
	int     n;

	if ( (n=strlen(errmsg)) > 0)
		if (write(spipefd, errmsg, n) != n) 
			return(-1);

	if (status >= 0)
		status = -1;  

	if (send_fd(spipefd, status) < 0)
		return(-1);

	return(0);
}

int send_ok(int spipefd, const char *errmsg)
{
	struct iovec    iov[2];
	char            ok[3] = { 'O', 'K', ' ' };

	iov[0].iov_base = ok;
	iov[0].iov_len  = 3;
	iov[1].iov_base = (char *)errmsg;
	iov[1].iov_len  = strlen(errmsg);

	if (writev(spipefd, iov, 2) != iov[1].iov_len+3) 
		return(-1);
	return(0);
}
