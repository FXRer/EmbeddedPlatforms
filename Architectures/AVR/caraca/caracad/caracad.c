/*
 * CARACA - CAn Remote Automation and Control with the Avr
 * http://caraca.sourceforge.net
 *
 *
 *      caracad - A CAN Message filtering and dispatching daemon
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

/*      $Id: caracad.c,v 1.11 2001/08/22 12:09:49 skr Exp $ */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <argp.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "caracad.h"
#include "log.h"
#include "globals.h"

// these are read by argp_parse()
const char *argp_program_version = "caracad 0.3";
const char *argp_program_bug_address = "<k.riedel@gmx.de>";
static char doc[] = "caracad -- a LinuX Interface to the CAN BUS";

char   err_msg[240];
FILE *pidfile;
static sig_atomic_t term=0,hup=0;
static int termsig;
int sockfd;
mode_t permission=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;

extern Globals	*init_globals();
static void  client_alloc(void);
static int   client_add(int fd, pid_t pid);
static void  client_del(Client *client);
inline int serve(void);
int addrtoi(char *addr);
int can_fmt_msg(char *res, canmsg_t *rx );
int send_can2client(canmsg_t *rx,int fd,int format);
static void  request(char *buf, int bytes_read, Client *client);
static int   buf_argv(char *buf, int *argc, char *argv[]);
int  serv_init(unsigned short port);
int  serv_accept(int sockfd, uid_t *uidzgr);
int  send_fd(int spipefd , int fd);
int send_ok(int spipefd, const char *errmsg);
int send_err(int spipefd, int status, const char *errmsg);

void usage( char *name )
{
    fprintf( stderr, "usage: %s\t[-d debug-level] [-s syslog-level]\n", name );
    fprintf( stderr, "\t\t[-f CAN device]\n" );
}

// The options we understand.
static struct argp_option options[] =
{
        {"debug",       'd', "level", 0,  "don´t go into background" },
        {"nolock",      'n', 0, 0,      "do not manage tty lockfiles" },
        {"tty",         't', "tty_device",      0,  "tty device to use, defaults to /dev/can" },
        {"port",        'p', "TCP_port", 0, "TCP Port to use, defaults to 7776" },
        {"log",         'l', "filename", OPTION_ARG_OPTIONAL,
                "enable logging to syslog or file\nfilename '-' means: log to stderr" },
        {"verbosity",   'v', "level", 0,
                "enable enhanced logging, valid levels: d,l,i,a,c" },
        { 0 }
};


/*
 * Parse a single option and fill in any appropriate globals.
 */

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	unsigned long ul_value;
	char *endptr;
	switch (key) {
		case 'd':	/* turn on debugging */
			if ( optarg[0] == '?' ) {
				printf( "Debug levels:\n" );
				printf( "\tDEBUG_TRACE\t%d\n", DEBUG_TRACE );
				printf( "\tDEBUG_MSGS\t%d\n", DEBUG_MSGS );
				printf( "\tDEBUG_ARGS\t%d\n", DEBUG_ARGS );
				printf( "\tDEBUG_CONNS\t%d\n", DEBUG_CONNS );
				printf( "\tDEBUG_FILTER\t%d\n", DEBUG_FILTER );
				printf( "\tDEBUG_CONFIG\t%d\n", DEBUG_CONFIG );
				printf( "\tDEBUG_ANY\t%d\n", DEBUG_ANY );
				return( -1 );
			} else {
				ul_value = strtoul(optarg,&endptr,0);
				if (*endptr || !ul_value || ul_value > 0xfffe)
					argp_failure(state,1,0,"invalid TCP port requested");
				debug_level = ul_value;
			}
			break;
		case 't':	/* can device */
			sglob->can_device= arg;
			break;
		case 'p':
			ul_value = strtoul(optarg,&endptr,0);
			if (*endptr || !ul_value || ul_value > 0xfffe)
				argp_failure(state,1,0,"invalid TCP port requested");
			sglob->port = ul_value;
			break;
		case ARGP_KEY_ARG:
			// no arguments accepted
			argp_usage (state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;

}

static struct argp opt_parser = { options, parse_opt, 0, doc };

void sigterm(int sig)
{
	/* all signals are blocked now */
	if(term) return;
	term=1;
	termsig=sig;
}

void dosigterm(int sig)
{
	signal(SIGALRM,SIG_IGN);
	
	syslog(LOG_NOTICE,"caught signal");
	shutdown(sockfd,2);
	close(sockfd);
	fclose(pidfile);
	(void) unlink(PIDFILE);
	closelog();
	signal(sig,SIG_DFL);
	raise(sig);
}

void sighup(int sig)
{
	hup=1;
}

void dosighup(int sig)
{
	int i;

	/*config();*/
	for (i=0; i<sglob->client_cnt; i++)
	{
/*	
 *	TODO:
 *	send sighup to all clients 
 *	(reread config)
 *	if(!(write_socket_len(clis[i],protocol_string[P_BEGIN]) &&
		     write_socket_len(clis[i],protocol_string[P_SIGHUP]) &&
		     write_socket_len(clis[i],protocol_string[P_END])))
		{
			client_del(&sglob->client[i]);
			i--;
		}*/
	}
}


static void client_alloc(void)
{
	int   i;

	sglob->client = realloc(sglob->client, (sglob->client_cnt+REALLOC_CNT) * sizeof(Client));
	if (sglob->client == NULL)
		syslog(LOG_ERR, "out of memory (client_alloc)");

	for (i=sglob->client_cnt; i<sglob->client_cnt+REALLOC_CNT; i++)
		sglob->client[i].fd = -1;
	sglob->client_cnt += REALLOC_CNT;
	debug(DEBUG_CONNS, "client_alloc\n");
}

static int  client_add(int fd, pid_t pid)
{
	int   i;

	do {
		for (i=0; i<sglob->client_cnt; i++) {
			if (sglob->client[i].fd == -1) {
				sglob->client[i].fd = fd;
				sglob->client[i].pid = pid;
				return(i);
			}
		}
		client_alloc(); /* get space */
	} while (1);
}

/* adding new filter after existing filter f
 */
static struct can_filter *filter_create(struct can_filter *f)
{
	 struct can_filter *fnew = malloc(sizeof(struct can_filter));
	 if (f == NULL) {              /* first one */
		 sglob->first_can_filter = sglob->last_can_filter = fnew;
		 fnew->next = NULL;
	 } else {
		 /*fnew->next = f->next;*/
		 f->next = fnew;
		 if (f->next == NULL) 
			 sglob->last_can_filter = fnew;
	 }
	 fnew->counter = 0;
	 return fnew;
}

static void filter_dump()
{
	struct can_filter *cur;

	syslog(LOG_DEBUG, "Dump Filters");
	for (cur=sglob->first_can_filter; cur; cur=cur->next) 
		syslog(LOG_DEBUG, "pid %d: %x,%x (%d Messages)", 
				cur->client->pid,cur->id,cur->mask,cur->counter);
}


static void filter_del(struct can_filter *f)
{
	struct can_filter *cur, *prev;

	for (cur=prev=sglob->first_can_filter; cur; prev=cur, cur=cur->next) 
		if (cur == f) {
			prev->next = cur->next;
			debug(DEBUG_FILTER, "drop filter for pid %d\n", cur->client->pid);
			if (cur == sglob->first_can_filter) 
				sglob->first_can_filter = cur->next;
			if (cur == sglob->last_can_filter) 
				sglob->last_can_filter = (prev == cur ? NULL : prev);
			free(cur);
			break;
		}
}

static void client_del(Client *client)
{
	int   i;
	struct can_filter *f;
	for (f = sglob->first_can_filter; f; f = f->next)
		if (f->client == client) 
			filter_del(f);

	for (i=0; i<sglob->client_cnt; i++) {
		if (&sglob->client[i] == client) {
			sglob->client[i].fd = -1;
			debug(DEBUG_FILTER, "client deleted %d\n", client->pid);
			return;
		}
	}
	syslog(LOG_ERR, "found no entry for fd %d", client[i].fd);
}

static void can_dispatch(canmsg_t *rx)
{
	struct can_filter *i;
	for (i = sglob->first_can_filter; i; i = i->next) {

		debug(DEBUG_FILTER, "rx->id: %x, i->mask: %x, rx->id & i->mask: %x, i->id: %x\n", 
				rx->id, i->mask, rx->id & i->mask, i->id);
		if ((rx->id & i->mask) == i->id){
			i->counter++;
			send_can2client(rx, i->client->fd, i->flags);
			//XXX check for E_PIPE
		}
	}
}

inline void waitfordata(fd_set *rset, int maxfd)
{
	int ret;

	do{
		do{
			/* handle signals */
			if(term)
			{
				dosigterm(termsig);
				/* never reached */
			}
			if(hup)
			{
				dosighup(SIGHUP);
				hup=0;
			}
			ret = select(maxfd+1, rset, NULL, NULL, NULL);
		} while(ret==-1 && errno==EINTR);
		if(ret==-1)
		{
			syslog(LOG_ERR, "select error");
			continue;
		}
	} while(ret==-1);

}

inline int serve(void)
{
	int       i, maxfd, maxi, client_fd, bytes_read;
	char      buf[MAX_ZEICHEN];
	char      msgbuf[MAX_ZEICHEN];
	pid_t     pid;
	fd_set    rmenge, allmenge;
        canmsg_t  rx;

	FD_ZERO(&allmenge);
	if ( (sockfd = serv_init(sglob->port)) < 0)
		syslog(LOG_ERR, "can't get socket");
	FD_SET(sockfd, &allmenge);
	FD_SET(sglob->can_fd, &allmenge);
	maxfd = sockfd;
	maxi  = -1;

	while (1) {
		rmenge = allmenge;
	
		(void) waitfordata(&rmenge,maxfd);

		if (FD_ISSET(sockfd, &rmenge)) {  
			if ( (client_fd = serv_accept(sockfd, &pid)) < 0) {
				syslog(LOG_ERR, "connection refused - fd: %d", client_fd);
				continue;
			}
			i = client_add(client_fd, pid);
			FD_SET(client_fd, &allmenge);
			if (client_fd > maxfd)
				maxfd = client_fd;  
			if (i > maxi)
				maxi = i;
			syslog(LOG_DEBUG, "new connection: pid %d, fd %d", pid, client_fd);
			continue;
		} else if (FD_ISSET(sglob->can_fd, &rmenge)) {
	
			can_Receive(sglob->can_fd, &rx);
			can_dispatch(&rx);
			//look thro filter queue
			//id,mask,client_fd
			//send msg - format?
			can_fmt_msg(msgbuf,&rx);
			syslog(LOG_ERR,msgbuf);

		} else
		
			for (i=0; i<=maxi; i++) {  /* each client */
				if ( (client_fd = sglob->client[i].fd) >= 0 &&
						FD_ISSET(client_fd, &rmenge)) {
					if ( (bytes_read = read(client_fd, buf, MAX_ZEICHEN)) < 0)
						syslog(LOG_ERR, "read error fd %d", client_fd);
					else if (bytes_read == 0) {
						syslog(LOG_DEBUG, "connection closed: pid %d, fd %d",
								sglob->client[i].pid, client_fd);
						client_del(&sglob->client[i]);
						FD_CLR(client_fd, &allmenge);
						close(client_fd);
					} else
						request(buf, bytes_read, &sglob->client[i]);
				}
			}
	}
}

/*----- request ---------------------------------------------------*/
/* client can:
 * send a can-message 
 * set a filter
 */

static void  request(char *buf, int bytes_read, Client *client)
{
	char     *argv[MAX_ARGC];
	char     can_msg[40];
	int      argc;
	char     *token;
	int      client_fd  = client->fd;
	canmsg_t tx;
	int      sent=0;
	struct   can_filter *f;
	
	if (buf[bytes_read-1] != '\0') {
		syslog(LOG_INFO, "request from pid %d without \\0: %*.*s\n",
				 client->pid, bytes_read, bytes_read, buf);
		send_err(client_fd, -1, err_msg);
		return;
	}
	debug(DEBUG_CONNS, "request: %s, pid %d\n", buf, client->pid);
	if (buf_argv(buf, &argc, argv) < 0) {
		send_err(client_fd, -1, err_msg);
		syslog(LOG_DEBUG, err_msg);
		return;
	}
	// parse args - adopted from CANterm 
	// (c) 1998 by C.Schroeter (clausi@chemie.fu-berlin.de)
	// 
	token = argv[0];
	switch (token[0]) {
	    case CARACA_CMSG_GET_CAN_FD:
		send_fd(client_fd,sglob->can_wr_fd);
		//strncpy(&client->name,argv[1],40);
		debug(DEBUG_CONNS, "CAN fd send to %.40s, fd %d\n",client->name,client_fd);
		break; 
	    case CARACA_CMSG_SEND_CAN_MSG:
		memcpy(&tx,buf+2,sizeof(tx));
		sent = can_Send(sglob->can_fd,&tx);
		can_fmt_msg(can_msg,&tx);
		
		send_ok(client_fd, err_msg);
		debug(DEBUG_MSGS, "CAN msg from pid %d: %s\n",client->pid,can_msg);
		break;
	    case CARACA_CMSG_FILTER:
		f = filter_create(sglob->last_can_filter);
		f->client = client;
		sscanf(argv[1],"%d,%d,%d", &(f->id),&(f->mask),&(f->flags)); 
		sglob->last_can_filter = f;
		sprintf(err_msg, "filter %x,%x,%x set for pid %d.\n", 
				f->id, f->mask, f->flags, client->pid);
		send_ok(client_fd, err_msg);
		debug(DEBUG_CONNS, err_msg);
		break; 
	    case CARACA_CMSG_DUMP_FILTERS:
		filter_dump();
		break; 
	    default:
		if( token[0] == 'R' && !strncmp(token,"RTR",3) ) {
			int sent=0;

			tx.flags= MSG_RTR;
			tx.id = addrtoi(argv[1]);
			sent = can_Send(sglob->can_fd,&tx);
			sprintf(err_msg, "RTR: %s\n", argv[1]);
			send_ok(client_fd, err_msg);
			syslog(LOG_DEBUG, err_msg);
		}
	}
	//syslog(LOG_DEBUG, "CAN fd %d for %s sent over fd %d.",
	//		neufd, argv[1], client_fd);
}

static int  buf_argv(char *buf, int *argc, char *argv[])
{
	char    *zgr;

	if (strtok(buf, " \t\n") == NULL)
		return(-1);

	argv[*argc=0] = buf;
	while ( (zgr = strtok(NULL, " \t\n")) != NULL) {
		if (++*argc >= MAX_ARGC-1)
			return(-1);
		argv[*argc] = zgr;
	}
	argv[++*argc] = NULL;
	return(0);
}

int main(int argc, char *argv[])
{	
	int fd;
	struct sigaction act;

	/* 
	 * Create and initialize globals.
	 */
	if (( sglob = init_globals()) == NULL ) {
		fprintf( stderr, "Out of memory initializing globals\n" );
		exit( 1 );
	}
	/*
	 * Read config file and initialize  structs.
	 */
	/*
	 * Process command-line args and fill in globals.
	 */
	if ( (sglob->myname = strrchr( argv[0], '/' )) == NULL ) {
		sglob->myname = strdup( argv[0] );
	} else {
		sglob->myname = strdup( sglob->myname + 1 );
	}

	if (argp_parse (&opt_parser, argc, argv, 0, 0, NULL)) 
		error_exit("argp_parse");

	printf("using CAN device %s\n", sglob->can_device);
	/* 
	 * Open a read/write CAN-connection for us and a write-only CAN-connection
	 * for clients (Hopefully my write is atomic).
	 */

	if (( sglob->can_fd = can_Init(sglob->can_device, O_RDWR)) < 0 ) {
		fprintf(stderr,"Error opening CAN device %s\n", sglob->can_device);
		exit(1);
	}

	if (( sglob->can_wr_fd = open(sglob->can_device, O_WRONLY | O_SYNC)) < 0 ) {
		fprintf(stderr,"Error opening CAN device %s\n", sglob->can_device);
		exit(1);
	}
	
	/* create pid lockfile in /var/run */
	if((fd=open(PIDFILE,O_RDWR|O_CREAT,0644))==-1 ||
	   (pidfile=fdopen(fd,"r+"))==NULL)
	{
		fprintf(stderr,"can't open or create %s\n",PIDFILE);
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	if(flock(fd,LOCK_EX|LOCK_NB)==-1)
	{
		int otherpid;
		
		if(fscanf(pidfile,"%d\n",&otherpid)>0)
		{
			fprintf(stderr,"already running (pid %d)\n", otherpid);
		}
		else
		{
			fprintf(stderr,"invalid %s encountered\n", PIDFILE);
		}
		exit(EXIT_FAILURE);
	}
	(void) fcntl(fd,F_SETFD,FD_CLOEXEC);
	rewind(pidfile);
	(void) fprintf(pidfile,"%d\n",getpid());
	(void) fflush(pidfile);
	(void) ftruncate(fileno(pidfile),ftell(pidfile));


	if (debug_level == 0) daemon(0,0);
	openlog( sglob->myname, OPENLOG_OPTIONS, LOG_DAEMON );
	
	act.sa_handler=sigterm;
	sigfillset(&act.sa_mask);
	act.sa_flags=SA_RESTART;           /* don't fiddle with EINTR */
	sigaction(SIGTERM,&act,NULL);
	sigaction(SIGINT,&act,NULL);
	
	act.sa_handler=sighup;
	sigemptyset(&act.sa_mask);
	act.sa_flags=SA_RESTART;           /* don't fiddle with EINTR */
	sigaction(SIGHUP,&act,NULL);
	
	// ignore SIGPIPE, writing to broken socket will return EPIPE instead
	signal(SIGPIPE,SIG_IGN);

	debug(DEBUG_CONNS, "started\n");
	return(serve());
}


