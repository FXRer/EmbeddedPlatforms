//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net ARP.C
//
// This module handles ARP messages and ARP resolution and manages
// the ARP cache. Refer to RFC 826 and RFC 1122
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "C8051f.h"
#include "net.h"
#include "eth.h"
#include "serial.h"
#include "ip.h"
#include "arp.h"



extern WAIT xdata wait;
extern UCHAR code my_hwaddr[];
extern UCHAR code broadcast_hwaddr[]; 
extern ULONG code my_ipaddr;
extern ULONG code my_subnet;
extern ULONG code gateway_ipaddr;
extern UCHAR idata debug;
ARP_CACHE xdata arp_cache[CACHESIZE];
UCHAR waiting_for_arp;



void init_arp(void)
{
  	memset(arp_cache, 0, sizeof(arp_cache)); 
	memset(&wait, 0, sizeof(wait));
	waiting_for_arp = FALSE;
}




//------------------------------------------------------------------------
//	This is called every 60 seconds to age the ARP cache
// If an entry times out then it is deleted from the cache
// See "TCP/IP Illustrated, Volume 1" Sect 4.3
//------------------------------------------------------------------------
void age_arp_cache(void)
{
 	UCHAR i;
    	
   for (i=0; i < CACHESIZE; i++)
   {
      if ((arp_cache[i].ipaddr != 0) && (arp_cache[i].timer))
      {
         arp_cache[i].timer--;
			if (arp_cache[i].timer == 0)
         {
				// Timed out so clear out cache entry
				// Do not need to zero hwaddr
				arp_cache[i].ipaddr = 0;
            if (debug) serial_send("ARP: Aged out a cache entry\r");
			}
		}
   }
}




//------------------------------------------------------------------------
// This allocates memory for the entire outgoing message,
// including eth and ip headers, then builds an outgoing
// ARP response message
// See "TCP/IP Illustrated, Volume 1" Sect 4.4
//------------------------------------------------------------------------
void arp_send(UCHAR * hwaddr, ULONG ipaddr, UCHAR msg_type)
{
	UCHAR xdata * outbuf;
	ARP_HEADER xdata * arp;
         
   
   // Allocate memory for entire outgoing message including
   // eth header. Always 42 bytes
   outbuf = (UCHAR xdata *)malloc(42);
   if (outbuf == NULL)
   {
      if (debug) serial_send("ARP: Oops, out of memory\r");
      return;
   }      
     
   // Allow 14 bytes for the ethernet header
   arp = (ARP_HEADER xdata *)(outbuf + 14);
 	
	arp->hardware_type = DIX_ETHERNET; 
   arp->protocol_type = IP_PACKET;
   arp->hwaddr_len = 6;
	arp->ipaddr_len = 4;               
   arp->message_type = (UINT)msg_type;
   
   // My hardware address and IP addresses 
   memcpy(arp->source_hwaddr, my_hwaddr, 6);
   arp->source_ipaddr = my_ipaddr;

   // Destination hwaddr and dest IP addr
   if (msg_type == ARP_REQUEST) memset(arp->dest_hwaddr, 0, 6);
   else memcpy(arp->dest_hwaddr, hwaddr, 6);
   
   arp->dest_ipaddr = ipaddr;
      
   // If request then the message is a brodcast, if a response then
   // send to specified hwaddr
   // ARP payload size is always 28 bytes
	if (msg_type == ARP_REQUEST) eth_send(outbuf, broadcast_hwaddr, ARP_PACKET, 28);
   else eth_send(outbuf, hwaddr, ARP_PACKET, 28);
}



//------------------------------------------------------------------------
// This re-sends an ARP request if there was no response to
// the first one.	 It is called every 0.5 seconds.  If there
// is no response after 2 re-tries, the datagram that IP was 
// trying to send is deleted
//-----------------------------------------------------------------------
void arp_retransmit(void)
{
	static UCHAR idata retries = 0; 
	
	if ((waiting_for_arp) && (wait.timer))
	{
		wait.timer--;
		if (wait.timer == 0)
		{
			retries++;
			if (retries <= 2)
			{
				if (debug) serial_send("ARP: Re-sending ARP broadcast\r");
	 			arp_send(NULL, wait.ipaddr, ARP_REQUEST);
				wait.timer = ARP_TIMEOUT;
			}
			else
			{	
				if (debug) serial_send("ARP: Gave up waiting for response\r");
	 			wait.timer = 0;
				waiting_for_arp = 0;
				free(wait.buf);
			}
		}
	}
}




//------------------------------------------------------------------------
// Find the ethernet hardware address for the given ip address
// If destination IP is on my subnet then we want the eth
// address	of destination, otherwise we want eth addr of gateway. 
// Look in ARP cache first.  If not found there, send ARP request.
// Return pointer to the hardware address or NULL if not found
// See "TCP/IP Illustrated, Volume 1" Sect 4.5
//------------------------------------------------------------------------
UCHAR xdata * arp_resolve(ULONG dest_ipaddr)
{
   UCHAR i;
      
   // If destination IP is not on my subnet then we really want eth addr
	// of gateway, not destination IP
	if ((dest_ipaddr ^ my_ipaddr) & my_subnet)
	{
		if (gateway_ipaddr == 0)
		{
			if (debug) serial_send("ARP: Error, gateway addr not set\r");
			return (NULL);	
	 	}
	 	else dest_ipaddr = gateway_ipaddr;
	}
	
	   
   // See if IP addr of interest is in ARP cache
   for (i=0; i < CACHESIZE; i++)
   {
      if (arp_cache[i].ipaddr == dest_ipaddr)
         return (&arp_cache[i].hwaddr[0]);
   }

	if (debug) serial_send("ARP: IP addr not found in cache\r");
	if (debug) serial_send("ARP: Sending an ARP broadcast\r");
	// Not in cache so broadcast ARP request
  	arp_send(NULL, dest_ipaddr, ARP_REQUEST);
     	   
   // Set a flag to indicate that an IP datagram is waiting
   // to be sent
   waiting_for_arp = TRUE;
				      
   // Null means that we have sent an ARP request
   return (NULL); 
}





//------------------------------------------------------------------------
// This handles incoming ARP messages
// See "TCP/IP Illustrated, Volume 1" Sect 4.4
// Todo:  Resolve problem of trying to add to a full cache
//------------------------------------------------------------------------
void arp_rcve(UCHAR xdata * inbuf)
{
   UCHAR idata i, cached, oldest;
   UINT idata minimum;
   ARP_HEADER xdata * arp;
      
   arp = (ARP_HEADER xdata *)(inbuf + 14);
   cached = FALSE;
   
   // Print message
   if (debug)
   {
      if (arp->message_type == ARP_REQUEST)
         serial_send("ARP: Request rcvd\r");
      else serial_send("ARP: Response rcvd\r");
   }
         
   // Validate incoming frame
   if ((arp->hardware_type != DIX_ETHERNET) ||
       (arp->protocol_type != IP_PACKET)) return;

   // Search ARP cache for senders IP address
   // If found, update entry and restart timer
   for (i=0; i < CACHESIZE; i++)
   {
      if (arp_cache[i].ipaddr == arp->source_ipaddr)
      {
         memcpy(&arp_cache[i].hwaddr[0], &arp->source_hwaddr[0], 6);
         arp_cache[i].timer = CACHETIME;		
         cached = TRUE;
         if (debug) serial_send("ARP: Cache entry updated\r");
                  
         break;  
      }
   }
   
   if (arp->dest_ipaddr != my_ipaddr) return;
   
   // At this point we know the the frame is addressed to me
   // If not already in cache then add entry and start timer
   if (cached == FALSE)
   {
      // Find first blank space and add entry
		// Blank entries are indicated by ip addr = 0
      for (i=0; i < CACHESIZE; i++)
      {
         if (arp_cache[i].ipaddr == 0) 
         {
            arp_cache[i].ipaddr = arp->source_ipaddr;
            memcpy(&arp_cache[i].hwaddr[0], &arp->source_hwaddr[0], 6);   
            arp_cache[i].timer = CACHETIME;
         	if (debug) serial_send("ARP: New cache entry added\r");
         	break;
         }
      }

		// If no blank entries in arp cache	then sort cache
		// to find oldest entry and replace it
		if (i == CACHESIZE)
		{
			// Oldest entry is the one with lowest timer value			
			minimum = 0xFFFF;
			for (i=0; i < CACHESIZE; i++)
      	{
				if (arp_cache[i].timer < minimum) 
				{
					minimum = arp_cache[i].timer;
					oldest = i;
				}
			}
      	
			// "oldest" is now index of oldest entry, so replace it
			arp_cache[oldest].ipaddr = arp->source_ipaddr;
         memcpy(&arp_cache[oldest].hwaddr[0], &arp->source_hwaddr[0], 6);   
         arp_cache[oldest].timer = CACHETIME;
        if (debug) serial_send("ARP: Cache full, so replaced oldest\r");
   	}
	}

   
   // If we are waiting for an arp response and the arp response
  	// that just came in is addressed to me and is from the host
  	// we are waiting for, then send	the message-in-waiting
   if (arp->message_type == ARP_RESPONSE)
   {
   	if ((waiting_for_arp) && (wait.ipaddr == arp->source_ipaddr))
   	{
   		waiting_for_arp = FALSE;
		  	ip_send(wait.buf, wait.ipaddr, wait.proto_id, wait.len);
		}
	}
	else if (arp->message_type == ARP_REQUEST)
   {
    	// Send ARP response 
    	if (debug) serial_send("ARP: Sending response\r");
    	arp_send(arp->source_hwaddr, arp->source_ipaddr, ARP_RESPONSE);
	}
}


