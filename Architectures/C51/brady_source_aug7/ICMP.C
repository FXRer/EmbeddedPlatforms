//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net ICMP.C
//
// This module handles ICMP messages
// Refer to RFC 792, 896, 950, 1122, and 1191
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include "C8051f.h"
#include "net.h"
#include "cksum.h"
#include "ip.h"
#include "serial.h"
#include "icmp.h"

extern UCHAR idata debug;


//------------------------------------------------------------------------
// This builds a ping response message.  It allocates memory for the
// entire outgoing message, including Eth and IP headers.  See "TCP/IP
// Illustrated, Volume 1" Sect 7.2 for info on Ping messages
//------------------------------------------------------------------------
void ping_send(UCHAR xdata * inbuf, ULONG ipaddr, UINT len)
{
   PING_HEADER xdata * ping_in;
   PING_HEADER xdata * ping_out;
   UCHAR xdata * outbuf;
            
   ping_in = (PING_HEADER xdata *)(inbuf + 34);
      
   // Allocate memory for entire outgoing message
   outbuf = (UCHAR xdata *)malloc(len + 34);
   if (outbuf == NULL)
   {
      if (debug) serial_send("PING: Oops, out of memory\r");
      return;
   }      
      
   // Ping response message payload starts at offset 34
   ping_out = (PING_HEADER xdata *)(outbuf + 34);

   ping_out->msg_type = 0;
   ping_out->msg_code = 0;
   ping_out->checksum = 0;
   ping_out->identifier = ping_in->identifier;
   ping_out->sequence = ping_in->sequence;
   
   memcpy(&ping_out->echo_data, &ping_in->echo_data, len - 8);
            
   // Compute checksum over the ICMP header plus
	// optional data and insert complement
   ping_out->checksum = ~cksum(outbuf + 34, len);
                        
   if (debug) serial_send("ICMP: Sending response to IP layer\r");

   ip_send(outbuf, ipaddr, ICMP_TYPE, len);
}



//------------------------------------------------------------------------
// This builds an outgoing ICMP destination port unreachable response
// message.  See See "TCP/IP Illustrated, Volume 1" Sect 6.5.  This
// message is typically sent in response to a UDP message directed
// to a port that has no corresponding application running. 
// Todo: The spec says we should return all options that were in
// the original incoming IP header.  Right now we cut off everything
// after the first 20 bytes. 
//------------------------------------------------------------------------
void dest_unreach_send(UCHAR xdata * inbuf, ULONG ipaddr)
{
   UCHAR xdata * outbuf;
   ICMP_ERR_HEADER xdata * icmp;
            
   // Allocate memory for entire outgoing message
   // including eth and IP haders.  Always 70 bytes
   outbuf = (UCHAR xdata *)malloc(70);
   if (outbuf == NULL)
   {
      if (debug) serial_send("ICMP: Oops, out of memory\r");
      return;
   }      
      
   icmp = (ICMP_ERR_HEADER xdata *)(outbuf + 34);
   
   // Fill in ICMP error message header
   icmp->msg_type = 3;   // destination unreachable
   icmp->msg_code = 3;   // port unreachable
   icmp->checksum = 0;
      
   // Fill in ICMP error message data
   icmp->msg_data = 0;
           
   // Copy in 20 byte original incoming IP header
   // plus 8 bytes of data
   memcpy(&icmp->echo_data, inbuf + 14, 28);
                               
   // Compute checksum over the 36 byte long ICMP
   // header plus data and insert complement
   icmp->checksum = ~cksum(outbuf + 34, 36);
      
   // Forward message to the IP layer
   if (debug) serial_send("ICMP: Sending dest unreach to IP layer\r");
   ip_send(outbuf, ipaddr, ICMP_TYPE, 36);
}




//------------------------------------------------------------------------
// This handles incoming ICMP messages.  See "TCP/IP Illustrated,
// Volume 1" Sect 6.2 for discussion of the various ICMP messages
//------------------------------------------------------------------------
void icmp_rcve(UCHAR xdata * inbuf, UINT len)
{
   IP_HEADER * ip;
   UCHAR idata msg_type;
   UINT idata temp;
   
   // Allow for 14 bytes eth header
   ip = (IP_HEADER *)(inbuf + 14);
        
   // IP header has been adjusted if necessary to always be 
	// 20 bytes so message starts at an offset of 34
   // Validate checksum of entire ICMP message incl data 
	temp = cksum(inbuf + 34, len);
	
	if (temp != 0xFFFF)
	{
	  	if (debug) serial_send("ICMP: Error, cksum bad\r");
      return; 
   }
      
   // Switch on the message type
   msg_type = *(inbuf + 34);
   switch(msg_type)
   {
      case 3:
      if (debug) serial_send("ICMP: Dest unreachable rcvd\r");
      break;

	   case 8:
      if (debug) serial_send("ICMP: Ping rcvd\r");
      ping_send(inbuf, ip->source_ipaddr, len); 
      break;

      default:
      if (debug) serial_send("ICMP: Error, unknown msg rcvd\r");
      break;
   }
}


