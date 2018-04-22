//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net CIRRUS.C
//
// This is the interface to the Cirrus Logic CS8900A ethernet chip
// To speed things up, it calls assembler functions to do raw
// reads and writes
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include "C8051f.h"
#include "net.h"
#include "serial.h"
#include "CS8900.h"
#include "cirrus.h"

extern UCHAR idata debug;
extern UCHAR code my_hwaddr[]; 
extern UCHAR idata rcve_buf_allocated;
extern UINT volatile event_word;
sbit CHIPSEL = P1^5;

// For timing tests
sbit DEBUG6 = P0^6;
sbit DEBUG7 = P0^7;



//------------------------------------------------------------------------
// This function reads a packetpage register from the CS8900A.
// It is done indirectly by first setting the desired address
// then reading the data 
//------------------------------------------------------------------------
UINT read_packetpage(UINT pp_addr)
{
   write_port(pp_addr, PORT_PKTPG_PTR);
   return (read_port(PORT_PKTPG_DATA));
}



//------------------------------------------------------------------------
// This function writes a packetpage register from the CS8900A.
// It is done indirectly by first setting the desired address
// then writing the data 
//------------------------------------------------------------------------
void write_packetpage(UINT pp_addr, UINT pp_data)
{
   write_port(pp_addr, PORT_PKTPG_PTR);
   write_port(pp_data, PORT_PKTPG_DATA);
}



//------------------------------------------------------------------------
// Initialize the Cirrus Logic CS8900A chip
//------------------------------------------------------------------------
void init_CS8900(void)
{
   UINT temp;
         
   // Reset the CS8900A
   if (debug) serial_send("Initializing CS8900A\r");
   write_packetpage(PKTPG_SELF_CTL, 0x0040);
   wait_msec(10);

	// Toggle SBHE to put CS8900A in 16 bit mode
   CHIPSEL = 0;
   CHIPSEL = 1;
	CHIPSEL = 0;
   CHIPSEL = 1;
	
   // Wait for CS8900A to complete the reset
   wait_msec(200);
              
   // Make sure chip init was completed
	if ((read_packetpage(PKTPG_SELF_ST) & 0x0080) == 0)
   {
     	if (debug) serial_send("Error: Could not reset CS8900A\r");
   	while (1){;}		// Stay here
   }   
   		
	// Verify registration number of CS8900A chip
   if (read_packetpage(PKTPG_EISA_NUM) != 0x630E)
   {
      if (debug) serial_send("Error: Bad CS8900A EISA number\r");   
      while(1){;} // Stay here
   }
	           
   if (debug) serial_send("Starting CS8900A setup\r");
   write_packetpage(PKTPG_INT_NUM,  0x0000);	 	// Use INTRQ0
   write_packetpage(PKTPG_RX_CFG,   0x0000);		// No interrupts
   write_packetpage(PKTPG_RX_CTL,   0x0D00);	   // Acceptance criteria
   write_packetpage(PKTPG_TX_CFG,   0x0000);	   // Xmit Intrs
	write_packetpage(PKTPG_BUF_CFG,  0x0000); 	// More Intrs
	write_packetpage(PKTPG_SELF_CTL, 0x0000);	   // LEDs & power modes
   write_packetpage(PKTPG_BUS_CTL,  0x1000); 	// Do not enable intrs

   // Load the hardware address acceptance filter with my eth addr
   temp = (my_hwaddr[0]) + (my_hwaddr[1] << 8);
   write_packetpage(PKTPG_IND_ADDR, temp);
	temp = (my_hwaddr[2]) + (my_hwaddr[3] << 8);
   write_packetpage(PKTPG_IND_ADDR + 2, temp);
	temp = (my_hwaddr[4]) + (my_hwaddr[5] << 8);
   write_packetpage(PKTPG_IND_ADDR + 4, temp);

   // Enable reception and transmission of frames 
   write_packetpage(PKTPG_LINE_CTL, 0x00C0);

   if (debug) serial_send("Completed CS8900A init\r");
}




//------------------------------------------------------------------------
// This functions checks CS8900A status then sends an ethernet
// frame to it by calling an assembler function. 
//------------------------------------------------------------------------
void send_frame(UCHAR xdata * outbuf, UINT len)
{
 	// Prime the CS8900A for transmitting
 	write_port(0x00C0, PORT_TX_CMD);
	write_port(len, PORT_TX_LENGTH);
   	
  	// See if CS8900A is free.  If not don't send
  	if ((read_packetpage(PKTPG_BUS_ST) & 0x0100) == 0)
  	{
		if (debug) serial_send("Error: CS8900A not rdy to xmit\r");   
      return;
	}
	
   // Make sure len is an even number of bytes because data
   // is written to the CS8900A in 16 bit chunks
   if (len & 0x0001) len++;
           
   // For speed, use an assembler function to write outgoing
   // frame to the CS8900A.  Pass it the starting address
   write_frame((UINT xdata *)outbuf, len);
         	   
   // Release memory for re-use
   free(outbuf);
}




//------------------------------------------------------------------------
// This functions checks the CS8900 receive event status
// word to see if an ethernet frame has arrived.  If so,
// set EVENT_ETH_ARRIVED bit in global event_word
//------------------------------------------------------------------------
void query_CS8900(void)
{   
   UINT temp;
   
   // Read CS8900 receive event status word
   temp = read_packetpage(PKTPG_RX_EVENT);
   
   // A frame with errors has arrived
   if (temp & 0x7000)
   {
      // Tell CS8900 to drop the frame
		write_packetpage(PKTPG_RX_CFG, 0x0140);	 
   }
   else if (temp & 0x0100)
   {
      // A good frame has arrived, set flag
      EA = 0;
      event_word |= EVENT_ETH_ARRIVED;
      EA = 1;
   }
}




//------------------------------------------------------------------------
// This function gets an incoming Ethernet frame from the CS8900A.
// There may be more than 1 waiting but just allocate memory for
// one and read one in.  Use the CS8900A to queue incoming packets.
//------------------------------------------------------------------------
UCHAR xdata * rcve_frame(void)
{
	UINT len, temp;
   UCHAR xdata * buf;
 
   // The first word is status which I do not need
   temp = read_port(PORT_RXTX_DATA);
			
	// Second word is length in bytes
   len = read_port(PORT_RXTX_DATA);
		
	// Make sure len is an even number of bytes because data
   // is read from the CS8900A in 16 bit chunks
	if (len & 0x0001) len++;	
	
	// Allocate enough memory to hold the incoming frame
   buf = (UCHAR xdata *)malloc(len);
	if (buf == NULL)
   {
      if (debug) serial_send("CS8900: Error, out of RAM\r");
		// Tell CS8900 to skip the frame
		write_packetpage(PKTPG_RX_CFG, 0x0140);			
      return NULL;
   }
   // This flag keeps track of allocated rcve memory
   rcve_buf_allocated = TRUE;

   // Call the assembler function to get the incoming frame
   read_frame((UINT xdata *)buf, len);
     
   // Return pointer to start of buffer
   return (buf);	
}




