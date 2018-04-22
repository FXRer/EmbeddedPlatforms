//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net TCP.C
//
// This module handles TCP segments
// Refer to RFC 793, 896, 1122, 1323, 2018, 2581
//
// A "connection" is a unique combination of 4 items:  His IP address,
// his port number, my IP address, and my port number.
//
// Note that a SYN and a FIN count as a byte of data, but a RST does
// not count. Neither do any of the other flags.
// See "TCP/IP Illustrated, Volume 1" Sect 17.3 for info on flags
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include <ctype.h>		// toupper
#include "C8051f.h"
#include "net.h"
#include "cksum.h"
#include "serial.h"
#include "ip.h"
#include "http.h"
#include "tcp.h"

// For timing tests
sbit DEBUG6 = P0^6;
sbit DEBUG7 = P0^7;

// These structures keep track of connection information
CONNECTION xdata conxn[5];

ULONG idata initial_sequence_nr;
UINT xdata sender_tcpport;
static ULONG xdata sender_ipaddr;
UCHAR idata just_closed; // Keeps track of when a conxn closed

extern ULONG code my_ipaddr;
extern UCHAR idata debug;
extern char xdata text[];

// Options: MSS (4 bytes), NOPS (2 bytes), Selective ACK (2 bytes) 
UCHAR code opt[10] = {
0x02, 0x04, 0x05, 0xB4,
0x01, 0x01,
0x04, 0x02};



//------------------------------------------------------------------------
//  Initialize variables declared in this module
//
//------------------------------------------------------------------------
void init_tcp(void)
{
   memset(conxn, 0, sizeof(conxn));
   just_closed = FALSE;
   initial_sequence_nr = 1;
}



//------------------------------------------------------------------------
// This sends a TCP segments that do not include any data.
// http_send() in the HTTP module is used to send data.
// See "TCP/IP Illustrated, Volume 1" Sect 17.3
//------------------------------------------------------------------------
void tcp_send(UINT flags, UINT hdr_len, UCHAR nr)
{
   ULONG idata sum, dest;
  	UINT idata result;
   UCHAR xdata * outbuf;
   TCP_HEADER xdata * tcp;
   IP_HEADER xdata * ip;
          
   
   // Allocate memory for entire outgoing message including
   // eth & IP headers 
   outbuf = (UCHAR xdata *)malloc(34 + hdr_len);
   if (outbuf == NULL)
   {
      if (debug) serial_send("TCP: Oops, out of memory\r");
      return;
   }

	   
   tcp = (TCP_HEADER xdata *)(outbuf + 34);
   ip = (IP_HEADER xdata *)(outbuf + 14);

   // If no connection, then message is probably a reset
   // message which goes back to the sender
   // Otherwise, use information from the connection.
   if (nr == NO_CONNECTION)
   {
      tcp->source_port = HTTP_PORT;
      tcp->dest_port = sender_tcpport;
      tcp->sequence = 0;
      tcp->ack_number = 0;
      dest = sender_ipaddr;
   }
   else if (nr < 5)
   {
      // This message is to connected port
      tcp->source_port = HTTP_PORT;
      tcp->dest_port = conxn[nr].port;
      tcp->sequence = conxn[nr].my_sequence;
      tcp->ack_number = conxn[nr].his_sequence;
      dest = conxn[nr].ipaddr;
   }
   else
   {
    	if (debug) serial_send("TCP: Oops, sock nr out of range\r");
		free(outbuf);
		return;
	}

   // Total segment length = header length
      
   // Insert header len
   tcp->flags = (hdr_len << 10) | flags;
   tcp->window = 1024;
   tcp->checksum = 0;
   tcp->urgent_ptr = 0;
   
   // Sending SYN with header options
   if (hdr_len == 28)
   {
      memcpy(&tcp->options, opt, 8);
   }   
   
   // Compute checksum including 12 bytes of pseudoheader
	// Must pre-fill 2 items in ip header to do this
	ip->dest_ipaddr = dest;
	ip->source_ipaddr = my_ipaddr;
		
	// Sum source_ipaddr, dest_ipaddr, and entire TCP message 
	sum = (ULONG)cksum(outbuf + 26, 8 + hdr_len);
				
	// Add in the rest of pseudoheader which is
	// protocol id and TCP segment length
	sum += (ULONG)0x0006;
	sum += (ULONG)hdr_len;

	// In case there was a carry, add it back around
	result = (UINT)(sum + (sum >> 16));
	tcp->checksum = ~result;
   
   if (debug) serial_send("TCP: Sending msg to IP layer\r");
	ip_send(outbuf, dest, TCP_TYPE, hdr_len);

   // (Re)start TCP retransmit timer
   conxn[nr].timer = TCP_TIMEOUT;
}




//------------------------------------------------------------------------
// This runs every 0.5 seconds.  If the other end has not ACK'd
// everyting we have sent, it re-sends it.  To save RAM space, we 
// regenerate a segment rather than keeping a bunch of segments 
// hanging around eating up RAM.  A connection should not be in an
// opening or closing state when this timer expires, so we simply
// send a reset.
//
//	If a connection is in the ESTABLISHED state when the timer expires
// then we have just sent a web page so re-send the page
//------------------------------------------------------------------------
void tcp_retransmit(void)
{
   static UCHAR idata retries = 0;
   UCHAR idata nr;

   // Scan through all active connections 
   for (nr = 0; nr < 5; nr++)
   {
      if ((conxn[nr].ipaddr != 0) && (conxn[nr].timer))
      {
         // Decrement the timer and see if it hit 0
         conxn[nr].timer--;
         if (conxn[nr].timer == 0)
         {
            // Socket just timed out. If we are not in ESTABLISHED state
            // something is amiss so send reset and close connection
            if (conxn[nr].state != STATE_ESTABLISHED)
            {
               // Send reset and close connection
               if (debug) serial_send("TCP: Timeout, sending reset\r");
               tcp_send(FLG_RST, 20, nr);
               conxn[nr].ipaddr = 0;
               return;
            }
            else
            {
               // Socket is in ESTABLISHED state. First make sure his
               // ack number is not bogus.
               if (conxn[nr].his_ack > conxn[nr].my_sequence)
               {
                  // Send reset and close connection
                  if (debug) serial_send("TCP: Timeout, sending reset\r");
                  tcp_send(FLG_RST, 20, nr);
                  conxn[nr].ipaddr = 0;
                  return;
               }
               
               // We always increment our sequence number immediately
					// after sending, so the ack number from the other end
					// should be equal to our sequence number.  If it is less,
					// it means he lost some of our data.
               if (conxn[nr].his_ack < conxn[nr].my_sequence)
					{
                  retries++;
						if (retries <= 2)
						{
                   	// The only thing we send is a web page, and it looks
                  	// like other end did not get it, so resend
                  	// but do not increase my sequence number
                  	if (debug) serial_send("TCP: Timeout, resending data\r");
                  	http_server(conxn[nr].query, 0, nr, 1);
			            conxn[nr].inactivity = INACTIVITY_TIME;
                  }
						else
						{
						  	if (debug) serial_send("TCP: Giving up, sending reset\r");
							// Send reset and close connection
               		tcp_send(FLG_RST, 20, nr);
               		conxn[nr].ipaddr = 0;
               	}
               }
            }
         }
      }
   }
}




//------------------------------------------------------------------------
// This runs every 0.5 seconds.  If the connection has had no activity
// it initiates closing the connection.
//
//------------------------------------------------------------------------
void tcp_inactivity(void)
{
   UCHAR idata nr;
   
   // Look for active connections in the established state
   for (nr = 0; nr < 5; nr++)
   {
      if ((conxn[nr].ipaddr != 0) && 
          (conxn[nr].state == STATE_ESTABLISHED) &&
          (conxn[nr].inactivity))
      {
         // Decrement the timer and see if it hit 0
         conxn[nr].inactivity--;
         if (conxn[nr].inactivity == 0)
         {
            // Inactivity timer has just timed out.
            // Initiate close of connection
            tcp_send((FLG_ACK | FLG_FIN), 20, nr);
            conxn[nr].my_sequence++;    // For my FIN
            conxn[nr].state = STATE_FIN_WAIT_1;
            if (debug) serial_send("TCP: Entered FIN_WAIT_1 state\r");	
         }
      }
   }
}



//------------------------------------------------------------------------
// This handles incoming TCP messages and manages the TCP state machine
// Note - both the SYN and FIN flags consume a sequence number.
// See "TCP/IP Illustrated, Volume 1" Sect 18.6 for info on TCP states
// See "TCP/IP Illustrated, Volume 1" Sect 17.3 for info on flags
//------------------------------------------------------------------------
void tcp_rcve(UCHAR xdata * inbuf, UINT len)
{
   UCHAR idata i, j, nr;
   UINT idata result, header_len, data_len;
   TCP_HEADER xdata * tcp;
   IP_HEADER xdata * ip;
   ULONG idata sum;
   
   // IP header is always 20 bytes so message starts at index 34      
   tcp = (TCP_HEADER xdata *)(inbuf + 34);
   ip = (IP_HEADER xdata *)(inbuf + 14);
				   
	// Compute TCP checksum including 12 byte pseudoheader
	// Sum source_ipaddr, dest_ipaddr, and entire TCP message 
	sum = (ULONG)cksum(inbuf + 26, 8 + len);
		
	// Add in the rest of pseudoheader which is
	// protocol id and TCP segment length
	sum += (ULONG)0x0006;     
	sum += (ULONG)len;

	// In case there was a carry, add it back around
	result = (UINT)(sum + (sum >> 16));
		
	if (result != 0xFFFF)
	{
		if (debug) serial_send("TCP: Error, bad cksum\r");
		return;
   }

	if (debug) serial_send("TCP: Msg rcvd with good cksum\r");
   
	// See if message is for http server
	if (tcp->dest_port != HTTP_PORT)	
   {
      if (debug)
      {
         serial_send("TCP: Error, msg to port ");
         memset(text, 0, 10);
         itoa(tcp->dest_port, text, 10);
         serial_send(text);
		   serial_send("\r");
      }
      tcp_send(FLG_RST, 20, NO_CONNECTION);
      return;
   }
   
   // Capture sender's IP address and port number
   sender_ipaddr = ip->source_ipaddr;
   sender_tcpport = tcp->source_port;
   
   // See if the TCP segment is from someone we are already
   // connected to. 
   for (i=0; i < 5; i++)
   {
      if ((ip->source_ipaddr == conxn[i].ipaddr) &&
         (tcp->source_port == conxn[i].port))
      {   
         nr = i;
         if (debug) serial_send("TCP: Rcvd msg from existing conxn\r");
         break;
      }       
   }
   
   // If i = 5, we are not connected. If it is a SYN then assign
   // a temporary conection  to it for processing
   if (i == 5)
   {
      if (tcp->flags & FLG_SYN)
      {
         // Find first unused connection (one with IP = 0) 
         for (j=0; j < 5; j++)
         {
            if (conxn[j].ipaddr == 0)
            {
               nr = j;
               // Initialize new connection
               conxn[nr].state = STATE_LISTEN;
               break;
            }
         }
      
         // If all connections are used then drop msg
         if (j == 5) return;
         
         if (debug)
         {
            serial_send("TCP: New connection ");
            memset(text, 0, 10);
            itoa((UINT)nr, text, 10);
            serial_send(text);
			   serial_send("\r");
         }
      }
   }


   // By now we should have a connection number in range of 0-4
   // Do a check to avoid any chance of exceeding size of struct
   if (nr > 4)
   {
      if (debug) serial_send("TCP: Error in assigning conxn number\r");
      return;
   }

   // Eventually put in protection against wrapping sequence
   // numbers, for now make the client start over if his
   // sequence number is close to wrapping
   if (tcp->sequence > 0xFFFFFF00L) 
   {
      if (debug) serial_send("TCP: Rcvd a high sequence number\r");
		conxn[nr].ipaddr = 0;			
		tcp_send(FLG_RST, 20, NO_CONNECTION);
		return;		
   }
           
   // Handle messages whose action is mostly independent of state
   // such as RST, SYN, and segment with no ACK.  That way the
	// state machine below does not need to worry about it.
   if (tcp->flags & FLG_RST)
   {
      // An RST does not depend on state at all.  And it does
      // not count as data so do not send an ACK here.  Close
      // connection
 		if (debug) serial_send("TCP: Rcvd a reset\r");
      conxn[nr].ipaddr = 0;
      return;
   }
	
	else if (tcp->flags & FLG_SYN)
	{
	   // A SYN segment only makes sense if connection is in LISTEN 
	   if ((conxn[nr].state != STATE_LISTEN) &&
          (conxn[nr].state != STATE_CLOSED))
		{
			if (debug) serial_send("TCP: Error, rcvd bogus SYN\r");
			conxn[nr].ipaddr = 0;			
		  	tcp_send(FLG_RST, 20, NO_CONNECTION);
		  	return;		
		}
	}
	
	else if ((tcp->flags & FLG_ACK) == 0)
	{
		// Incoming segments except SYN or RST must have ACK bit set
	 	// See TCP/IP Illustrated, Vol 2, Page 965
      // Drop segment but do not send a reset
		if (debug) serial_send("TCP: Error, rcvd segment has no ACK\r");
		return;
	}
	   
   // Compute length of header including options, and from that
   // compute length of actual data
   header_len =  (tcp->flags & 0xF000) >> 10;
   data_len = len - header_len;


         
   // Handle TCP state machine for this connection
   switch (conxn[nr].state)
   {
      case STATE_CLOSED:
      case STATE_LISTEN:
            
      // If incoming segment contains SYN and no ACK, then handle 
      if ((tcp->flags & FLG_SYN) && ((tcp->flags & FLG_ACK) == 0))
      {
         // Capture his starting sequence number and generate
         // my starting sequence number
         // Fill in connection information
         conxn[nr].ipaddr = ip->source_ipaddr;
         conxn[nr].port = tcp->source_port;
         conxn[nr].state = STATE_LISTEN;
         conxn[nr].his_sequence = 1 + tcp->sequence;
         conxn[nr].his_ack = tcp->ack_number;
         
         // Use system clock for initial sequence number
         EA = 0;
         conxn[nr].my_sequence = initial_sequence_nr;
         initial_sequence_nr += 64000L;
         EA = 1;                  
                  
         // Send header options with the next message
         // Since timestamps are optional and we do not use
         // them, do not have to send them 
         // After sending the SYN ACK the client browser will
         // blast me with 2 messages, an ACK, and a HTTP GET
         tcp_send(FLG_SYN | FLG_ACK, 28, nr);
         
         // My SYN flag increments my sequence number
         // My sequence number is always updated to point to 
         // the next byte to be sent.  So the incoming ack
         // number should equal my sequence number  
         conxn[nr].my_sequence++;
      
         conxn[nr].state = STATE_SYN_RCVD;
         if (debug) serial_send("TCP: Entered SYN RCVD state\r");
      }
		else 
      {
         // Sender is out of sync so send reset
         conxn[nr].ipaddr = 0;
         tcp_send(FLG_RST, 20, NO_CONNECTION);   
      } 
		break;


      case STATE_SYN_RCVD:
      // He may already be sending me data - should process it
		conxn[nr].his_sequence += data_len;
      conxn[nr].his_ack = tcp->ack_number;
            
      if (tcp->flags & FLG_FIN)
		{
		   // His FIN counts as a byte of data
         conxn[nr].his_sequence++;
         tcp_send(FLG_ACK, 20, nr);
         conxn[nr].state = STATE_CLOSE_WAIT;
         if (debug) serial_send("TCP: Entered CLOSE_WAIT state\r");
               
        	// At this point we would normally wait for the	application
         // to close.  For now, send FIN right away.
         tcp_send(FLG_FIN | FLG_ACK, 20, nr);
         conxn[nr].my_sequence++;   // For my FIN
         conxn[nr].state = STATE_LAST_ACK;
         if (debug) serial_send("TCP: Entered LAST ACK state\r");
      }

		// Make sure he is ACKing my SYN
		else if (tcp->ack_number == conxn[nr].my_sequence)
      {
         conxn[nr].state = STATE_ESTABLISHED;
         if (debug) serial_send("TCP: Entered ESTABLISHED state\r");
         // If sender sent data ignore it and he will resend
         // Do not send response because we received no
         // data... wait for client to send something to me 
      }
      break;


      case STATE_ESTABLISHED:
      conxn[nr].his_ack = tcp->ack_number;
           
      if (tcp->flags & FLG_FIN)
		{
		   // His FIN counts as a byte of data
         conxn[nr].his_sequence++;
         tcp_send(FLG_ACK, 20, nr);
         conxn[nr].state = STATE_CLOSE_WAIT;
         if (debug) serial_send("TCP: Entered CLOSE_WAIT state\r");
               
        	// At this point we would normally wait for the	application
         // to close.  For now, send FIN immediately.
         tcp_send(FLG_FIN | FLG_ACK, 20, nr);
         conxn[nr].my_sequence++;   // For my FIN
         conxn[nr].state = STATE_LAST_ACK;
         if (debug) serial_send("TCP: Entered LAST ACK state\r");
      }
		else if (data_len != 0)
      {
			// Received normal TCP segment from sender with data
      	// Send an ACK immediately and pass the data on to
			// the application
			conxn[nr].his_sequence += data_len;
         tcp_send(FLG_ACK, 20, nr); 		// Send ACK
      	  
      	      	      	 									
			// Send pointer to start of TCP payload
			// http_server increments my sequence number when 
         // sending so don't worry about it here
         result = http_server(inbuf, header_len, nr, 0);
								
			// Start timer to close conxn if no activity
			conxn[nr].inactivity = INACTIVITY_TIME;
		}
	   break;


      case STATE_CLOSE_WAIT:
      // With this code, should not get here
      if (debug) serial_send("TCP: Oops! Rcvd unexpected message\r");
      
      break;

      
      case STATE_LAST_ACK:
      conxn[nr].his_ack = tcp->ack_number;
            
      // If he ACK's my FIN then close
      if (tcp->ack_number == conxn[nr].my_sequence)
      {
         conxn[nr].state = STATE_CLOSED;
         conxn[nr].ipaddr = 0;  // Free up struct area
         just_closed = TRUE;
      }
      break;

      
      case STATE_FIN_WAIT_1:
      // He may still be sending me data - should process it
		conxn[nr].his_sequence += data_len;
      conxn[nr].his_ack = tcp->ack_number;
                  
      if (tcp->flags & FLG_FIN)
      {
         // His FIN counts as a byte of data
         conxn[nr].his_sequence++;
         tcp_send(FLG_ACK, 20, nr);
         
         // If he has ACK'd my FIN then we can close connection
         if (tcp->ack_number == conxn[nr].my_sequence)
			{
         	conxn[nr].state = STATE_TIME_WAIT;
         	if (debug) serial_send("TCP: Entered TIME_WAIT state\r");
               
         	conxn[nr].state = STATE_CLOSED;
         	conxn[nr].ipaddr = 0;  // Free up connection
         	just_closed = TRUE;
      	}
			else
			{
				// He has not ACK'd my FIN.  This happens when there is a simultaneous
				// close - I got his FIN but he has not yet ACK'd my FIN
				conxn[nr].state = STATE_CLOSING;
				if (debug) serial_send("TCP: Entered CLOSING state\r");
			}
		}
      else if (tcp->ack_number == conxn[nr].my_sequence)
      {
         // He has ACK'd my FIN but has not sent a FIN yet himself
         conxn[nr].state = STATE_FIN_WAIT_2;
         if (debug) serial_send("TCP: Entered FIN_WAIT_2 state\r");
      }
      break;

      
      case STATE_FIN_WAIT_2:
      // He may still be sending me data - should process it
		conxn[nr].his_sequence += data_len;
      conxn[nr].his_ack = tcp->ack_number;
      
      if (tcp->flags & FLG_FIN)
      {
         conxn[nr].his_sequence++; // For his FIN flag
         tcp_send(FLG_ACK, 20, nr);
         conxn[nr].state = STATE_TIME_WAIT;
         if (debug) serial_send("TCP: Entered TIME_WAIT state\r");
         conxn[nr].state = STATE_CLOSED;
         conxn[nr].ipaddr = 0;  // Free up struct area
         just_closed = TRUE;
      }
      break;
            
            
      case STATE_TIME_WAIT:
      // With this code, should not get here
      if (debug) serial_send("TCP: Oops! In TIME_WAIT state\r");
      break;

      
      case STATE_CLOSING:
      // Simultaneous close has happened. I have received his FIN
      // but he has not yet ACK'd my FIN.  Waiting for ACK.
		// Will not receive data in this state
		conxn[nr].his_ack = tcp->ack_number;
      		
		if (tcp->ack_number == conxn[nr].my_sequence)
      {
		   conxn[nr].state = STATE_TIME_WAIT;
         if (debug) serial_send("TCP: Entered TIME_WAIT state\r");
         
         // Do not send any response to his ACK
         conxn[nr].state = STATE_CLOSED;
         conxn[nr].ipaddr = 0;  // Free up struct area
         just_closed = TRUE;
      }
      break;

      
      default:
      if (debug) serial_send("TCP: Error, no handler\r");
      break;
   }
   
   // This is for debug, to see when conxn closes
   if (just_closed)
   {
      just_closed = FALSE;
      if (debug)
      {
         serial_send("TCP: Closed connection ");
         memset(text, 0, 10);
         itoa((UINT)nr, text, 10);
		   serial_send(text);
		   serial_send("\r");
      }
   }
}



