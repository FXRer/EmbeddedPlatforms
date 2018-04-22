//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net IP.C
//
// This module is the IP layer
// Refer to RFC 791, 1122, and RFC 815 (fragmentation)
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include "C8051f.h"
#include "net.h"
#include "cksum.h"
#include "arp.h"
#include "eth.h"
#include "icmp.h"
#include "udp.h"
#include "serial.h"
#include "tcp.h"
#include "ip.h"

extern UCHAR idata debug;
extern ULONG code my_ipaddr;
WAIT xdata wait;


//------------------------------------------------------------------------
// This handles outgoing IP datagrams.  It adds the 20 byte IP header
// and checksum then forwards the IP datagram to the Ethernet layer
// for sending. See "TCP/IP Illustrated, Volume 1" Sect 3.2
//------------------------------------------------------------------------
void ip_send(UCHAR xdata * outbuf, ULONG ipaddr, UCHAR proto_id, UINT len)
{
   IP_HEADER xdata * ip;
   UCHAR xdata * hwaddr;
   static UINT ip_ident = 0;
   
   ip = (IP_HEADER xdata *)(outbuf + 14);
   ip->ver_len = 0x45;          // IPv4 with 20 byte header
   ip->type_of_service = 0;
   ip->total_length = 20 + len;
   ip->identifier = ip_ident++;     // sequential identifier
   ip->fragment_info = 0;           // not fragmented
   ip->time_to_live = 32;           // max hops
   ip->protocol_id = proto_id;      // type of payload
   ip->header_cksum = 0;
   ip->source_ipaddr = my_ipaddr;
   
   // Outgoing IP address
   ip->dest_ipaddr = ipaddr;

   // Compute and insert complement of checksum of ip header
   // Outgoing ip header length is always 20 bytes
   ip->header_cksum = ~cksum(outbuf + 14, 20);
   
   // Use ARP to get hardware address to send this to
   hwaddr = arp_resolve(ip->dest_ipaddr);
	
	// Null means that the ARP resolver did not find the IP address
	// in its cache so had to send an ARP request
	if (hwaddr == NULL)
	{
		// Fill in the destination information so ehrn the ARP response
		// arrives we can identify it and know what to do when we get it
      wait.buf = outbuf;
		wait.ipaddr = ip->dest_ipaddr;
		wait.proto_id = proto_id;
		wait.len = len;
		wait.timer = ARP_TIMEOUT; 
      return;
	}	
	
	eth_send(outbuf, hwaddr, IP_PACKET, 20 + len);
}



//------------------------------------------------------------------------
// This handles incoming IP datagrams from the Ethernet layer
// See "TCP/IP Illustrated, Volume 1" Sect 3.2
//------------------------------------------------------------------------
void ip_rcve(UCHAR xdata * inbuf)
{
	IP_HEADER xdata * ip;
	UINT idata header_len, payload_len;
		
   ip = (IP_HEADER xdata *)(inbuf + 14);
            
   // Make sure it is addressed to my IP address
   if (ip->dest_ipaddr != my_ipaddr) return;

   // Validate checksum of ip header
	header_len = 4 * (0x0F & ip->ver_len);
	payload_len = ip->total_length - header_len;
   if (cksum(inbuf + 14, header_len) != 0xFFFF)
	{
	  	if (debug) serial_send("IP:  Error, cksum bad\r");
	   return; 
   }
	
	// Make sure incoming message is IP version 4
	if ((ip->ver_len >> 4) != 0x04)
	{
		if (debug) serial_send("IP:  Error, not IPv4\r");
   	return;
	}

	// Make sure incoming message is not fragmented because
   // we cannot handle fragmented messages
   if ((ip->fragment_info & 0x3FFF) != 0)
   {
      if (debug) serial_send("IP:  Error, fragmented msg rcvd\r");
	   return; 
   }

   // At this point we have received a valid IP datagram addressed
   // to me.  We do not use header options, and do not forward
   // messages, so in the unlikely event there are header options,
   // delete them and shift the data down. The advantage is that
   // layers such as UDP and TCP know where their data starts
	if (header_len > 20)
	{
		if (debug) serial_send("IP: Rcvd header > 20 bytes\r");
   			
      // Use memmove because of overlap
      memmove(inbuf + 34, inbuf + 14 + header_len, payload_len);

		// Adjust info to reflect the move
		header_len = 20;
		ip->ver_len = 0x45;
		ip->total_length = 20 + payload_len;
	}
	
	
	// Look at protocol ID byte and call the appropriate
   // function to handle the received message.  See 
   // "TCP/IP Illustrated, Volume 1" Sect 1.7 and RFC 791
   // for values for various protocols
   switch (ip->protocol_id)
	{
	   case ICMP_TYPE:
      if (debug) serial_send("IP:  ICMP pkt rcvd\r");
      icmp_rcve(inbuf, payload_len);
		break;

      case IGMP_TYPE:
		// We cannot handle IGMP messages
		if (debug) serial_send("IP:  Error, IGMP pkt rcvd\r");
		break;
		  
		case UDP_TYPE:
		if (debug) serial_send("IP:  UDP pkt rcvd\r");
      udp_rcve(inbuf, payload_len);
		break;

		case TCP_TYPE:   
		if (debug) serial_send("IP:  TCP pkt rcvd\r");
      tcp_rcve(inbuf, payload_len);
		break;

      default:
      if (debug) serial_send("IP:  Unknown IP proto id rcvd\r");
      break;
   }
}



