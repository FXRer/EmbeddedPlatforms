//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net ETH.C
//
// This module is the Ethernet layer
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include "C8051f.h"
#include "net.h"
#include "serial.h"
#include "arp.h"
#include "ip.h"
#include "cirrus.h"
#include "eth.h"

extern UCHAR idata debug;
extern UCHAR xdata arpbuf[];
extern UCHAR code my_hwaddr[]; 


//------------------------------------------------------------------------
// This adds the ethernet header and sends completed Ethernet
// frame to CS8900A.  See "TCP/IP Illustrated, Volume 1" Sect 2.2
//------------------------------------------------------------------------
void eth_send(UCHAR xdata * outbuf, UCHAR * hwaddr, UINT ptype, UINT len)
{
	ETH_HEADER xdata * eth;
   
   eth = (ETH_HEADER xdata *)outbuf;
	  
	// Add 14 byte Ethernet header
	memcpy(eth->dest_hwaddr, hwaddr, 6);
	memcpy(eth->source_hwaddr, my_hwaddr, 6); 
   eth->frame_type = ptype;

   // We just added 14 bytes to length
   send_frame(outbuf, len + 14);
}




//------------------------------------------------------------------------
// This is the handler for incoming Ethernet frames
//	This is designed to handle standard Ethernet (RFC 893) frames
// See "TCP/IP Illustrated, Volume 1" Sect 2.2
//------------------------------------------------------------------------
void eth_rcve(UCHAR xdata * inbuf)
{
   ETH_HEADER xdata * eth;
   
   eth = (ETH_HEADER xdata *)inbuf;
   
   // Reject frames in IEEE 802 format where Eth type field
   // is used for length.  Todo: Make it handle this format
   if (eth->frame_type < 1520)
   {
      if (debug) serial_send("ETH: IEEE 802 pkt rejected\r");
      return;      
   }

   // Figure out what type of frame it is from Eth header
   // Call appropriate handler and supply address of buffer
   switch (eth->frame_type)
   {
	   case ARP_PACKET:
	   arp_rcve(inbuf);
	   break;
		      
	   case IP_PACKET:
	   ip_rcve(inbuf);
      break;

      default:
		if (debug) serial_send("Error: Unknown pkt rcvd\r");
      break;
   }
}



