/*
 * CARACA - CAn Remote Automation and Control with the Avr
 * http://caraca.sourceforge.net
 *
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

/*      $Id: sock.c,v 1.5 2001/08/22 12:09:49 skr Exp $ */

#include   <sys/socket.h>
#include   <sys/stat.h>
#include   <arpa/inet.h>
#include   <stddef.h>
#include   <time.h>
#include "caracad.h"
#include "log.h"

int  serv_init(unsigned short myport)
{
	int                  server_socket;
	struct sockaddr_in   address;
	struct linger opt_nolinger = { 0 , 0 };
	int opt_true = 1;

	// create the server socket	
	server_socket = socket( PF_INET, SOCK_STREAM, 0);
	if ( server_socket < 0 ) error_exit("socket");
	
	// let socket reuse address
	if ( setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &opt_true, sizeof(opt_true) ) )
		error_exit("setsockopt"); 	 	 	 	
	// don´t linger on close
 	if ( setsockopt( server_socket, SOL_SOCKET, SO_LINGER, &opt_nolinger, sizeof(opt_nolinger) ))
 		error_exit("setsockopt");
 	
 	address.sin_family = AF_INET;
 	address.sin_port = htons(myport);
 	address.sin_addr.s_addr = INADDR_ANY;
 	
 	// set address and port
 	if ( bind(server_socket, (struct sockaddr *)&address, sizeof(address) ) )
 		error_exit("bind");
 	
 	// make sure accept will never block,
 	// implies all accepted connections also will be nonblocking
 	if ( fcntl ( server_socket, F_SETFL, O_NONBLOCK ) )
 		error_exit("fcntl");
 		
 	// make socket listening
 	if ( listen( server_socket, 3 ) )
 		error_exit("listen");
 	
	return(server_socket);
}

int  serv_accept(int sockfd, pid_t *pidzgr)
{
	int                client_fd;
	struct sockaddr_in client_address;
	socklen_t len = sizeof(struct sockaddr_in);
	struct linger opt_nolinger = { 0 , 0 };

	if ((client_fd = accept(sockfd, (struct sockaddr *) &client_address, &len)) < 0)
	{
		// accept will return errors in case of network problems
		// so we don´t exit here but log the error
		log(CARACAD_LOG_INFO,"accept() error: %s",strerror(errno));
		return(-1);
	}

	// don´t linger on close
	if ( setsockopt( client_fd, SOL_SOCKET, SO_LINGER, &opt_nolinger, sizeof(opt_nolinger) ))
		error_exit("setsockopt");

	// we cannot look up the hostname, might take far too long 8-(
	log(CARACAD_LOG_CLIENT,"connection %u from host %s, port %hu",
			1,//client->identity, XXX
			inet_ntoa(client_address.sin_addr),
			ntohs(client_address.sin_port));

	return(client_fd);
}

