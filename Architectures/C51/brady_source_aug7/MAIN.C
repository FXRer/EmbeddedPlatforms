//---------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net MAIN.C
//
// 8051 Web Server project
// See Makefile for build notes 
// Written for Keil C51 V5.1 compiler, notes:
//   It uses big endian order, which is the same as the
//   network byte order, unlike x86 systems.
//   Use OPTIMIZE(2)or higher so that automatic variables get shared
//   between functions, to stay within the 256 bytes idata space
//---------------------------------------------------------------------------

#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include "C8051f.h"
#include "net.h"
#include "eth.h"
#include "serial.h"
#include "timer.h"
#include "analog.h"
#include "arp.h"
#include "cirrus.h"
#include "tcp.h"
#include "http.h"
#include "ip.h"


// Global variables
UINT volatile event_word;
char xdata text[20];  
UCHAR idata debug;
UCHAR idata rcve_buf_allocated;
extern char xdata serial_buf[];


// Port pins for debug pulses 
sbit DEBUG6 = P0^6;
sbit DEBUG7 = P0^7;


// This sets my hardware address to 00:01:02:03:04:05
UCHAR code my_hwaddr[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}; 

// Hardware addr to send a broadcast
UCHAR code broadcast_hwaddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// This sets my IP address to 192.168.10.10
ULONG code  my_ipaddr = 0xC0A80A0AL;

// This sets my subnet mask to 255.255.255.0
ULONG code my_subnet = 0xFFFFFF00L;

// Set to 0 if no gateway is present on network
ULONG code gateway_ipaddr = 0;



//------------------------------------------------------------------------
//	Switches the Cygnal 8051F microcontroller from its startup
// state (internal oscillator) to external 22.1184MHz crystal
//------------------------------------------------------------------------
void switch_to_xtal(void)
{
   UINT delay;

  	// Found that Cygnal board worked with XFCN = 101, 110, and 111
  	// for 22.1184MHz xtal.  Left at 110 = 0x06
  	OSCXCN = 0x66;	            // Enable external crystal	
		
	delay = 2000;				   // Delay 1 msec before polling XTLVLD
	while(delay--);

 	while (!(OSCXCN & 0x80));	// Wait until external crystal has started
 	OSCICN = 0x0C;					// Switch to external oscillator
 	OSCICN = 0x08;					// Disable internal oscillator

 	while (!(OSCXCN & 0x80));	// Wait until external crystal has started
 	OSCICN = 0x08;					// Switch to external oscillator

	delay = 2000;				   // Allow 1 msec time to stabilize
	while(delay--);
}




//--------------------------------------------------------------------------
// Initialize the memory management routines
// Initialize variables declared in main
//--------------------------------------------------------------------------
void init_main(void)
{
	// Start the memory pool for incoming and outgoing Ethernet
	// frames at 500, with length = 1500 bytes. Memory below 500
	// is used for program variables
	init_mempool((void xdata *)500, 1500);
	
	memset(text, 0, sizeof(text));
	event_word = 0;
   rcve_buf_allocated = FALSE;
   debug = FALSE;
}



//------------------------------------------------------------------------
//	Main runs after start.a51 does initialization 
//	This contains a simple priority-based task handler
//	Tasks are short and are not pre-empted
//------------------------------------------------------------------------
void main (void)
{
  	UINT j, event_word_copy;
   UCHAR xdata * inbuf;
                    	
  	WDTCN = 0xDE;              // Disable watchdog timer
	WDTCN = 0xAD;

  	// Configure ports
	PRT0CF = 0xFD;             // P0 port is push-pull except RS232 rcve
	PRT1CF = 0xFF;			      // P1 port is push-pull
   PRT2CF = 0x00;			      // P2 port is open-drain
   PRT3CF = 0x00;			      // P3 port is open-drain
   P1 = 0x38;                 // Initial state of Port 1
                                                            
	// Configure cross bar
	XBR0 = 0x04;               // Enable UART lines
	XBR2 = 0x40;	            // Enable crossbar and weak pull-ups
	
	switch_to_xtal();		
	init_main();
	init_serial();
   init_tcp();
   init_http();
	
		
	// Enable RS232 so the rest of init functions can send err messages
	ES = 1;
	EA = 1;
	
	// Finish initialization
	init_adc();
	init_timer2();
	init_arp();
	init_CS8900();
   
	j = 0;

   // Prevent uncalled segment linker error
   if (j) byte2text(j);

	ET2 = 1;		            // Enable timer 2 interrupt
	   	
   // The code below is a priority based RTOS.  The event
   // handlers are in priority order - highest priority first.
	while (1)
   {
      // Query CS8900A to see if Ethernet frame has arrived
      // If so, set EVENT_ETH_ARRIVED bit in event_word
      query_CS8900();
      
      // Use a copy of event word to avoid interference
      // with interrupts
		EA = 0;
		event_word_copy = event_word;
		EA = 1;
      
      // See if an Ethernet frame has arrived      
      if (event_word_copy & EVENT_ETH_ARRIVED)
      {
         EA = 0;
         event_word &= (~EVENT_ETH_ARRIVED);
         EA = 1;
         
         // Allocate a buffer and read frame from CS8900A
         inbuf = rcve_frame();
         if (inbuf != NULL)
         {
            // Process the received Ethernet frame 
            eth_rcve(inbuf); 
         
            // If the memory allocated for the rcve message has
            // not already been freed then free it now
            if (rcve_buf_allocated)
            {
               free(inbuf);
               rcve_buf_allocated = FALSE;
            }
         }
      }
      
      // See if TCP retransmit timer has expired            	       
      else if (event_word_copy & EVENT_TCP_RETRANSMIT)
      {
         EA = 0;
         event_word &= (~EVENT_TCP_RETRANSMIT);
         EA = 1;
         tcp_retransmit();
 		}

      // See if TCP inactivity timer has expired
      else if (event_word_copy & EVENT_TCP_INACTIVITY)
      {
         EA = 0;
         event_word &= (~EVENT_TCP_INACTIVITY);
         EA = 1;
         tcp_inactivity();
 		}

      // See if ARP retransmit timer has expired
 		else if (event_word_copy & EVENT_ARP_RETRANSMIT)
      {
         EA = 0;
         event_word &= (~EVENT_ARP_RETRANSMIT);
         EA = 1;
         arp_retransmit();
		}

      // See if it is time to age the ARP cache
      else if (event_word_copy & EVENT_AGE_ARP_CACHE)
      {
         EA = 0;
         event_word &= (~EVENT_AGE_ARP_CACHE);
			EA = 1;
         age_arp_cache();
		}

		// See if it is time to read the analog inputs
		else if (event_word_copy & EVENT_READ_ANALOG)
      {
         EA = 0;
         event_word &= (~EVENT_READ_ANALOG);
         EA = 1;
			// Read one of the 3 analog inputs each time
			read_analog_inputs();
      }

		// See if an RS232 message has arrived.  It is
      // not handled - RS232 is used for sending only
		else if (event_word_copy & EVENT_RS232_ARRIVED)
      {
         EA = 0;
         event_word &= (~EVENT_RS232_ARRIVED);
         EA = 1;
      }
   }
}


