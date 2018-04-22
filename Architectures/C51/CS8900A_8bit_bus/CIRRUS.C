//-----------------------------------------------------------------------------
// CIRRUS.C
// Last revised April 2004 - Jim Brady
//
// This is an 8051 interface to a Cittus CS8900A Ethernet controller over
// a conventional 8 bit bus.  The CS8900A is in 8 bit mode and connected
// as shown in Cirrus app note AN181.  This was tested by compiling with
// Keil v7 C51 compiler and run on an Atmel AT89C52 processor. The CS8900A
// is located at 0xF300 in the external memory space of the 8051.
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <at89x52.h>
#include <intrins.h>
#include <string.h>
#include <stdlib.h>
#include "cirrus.h"

typedef unsigned char  	   UCHAR;
typedef unsigned int 		UINT;
typedef unsigned long		ULONG;


// Macros
#define LOBYTE(w)			   ((UCHAR)(w))
#define HIBYTE(w)			   ((UCHAR)(0x00FF & ((UINT)(w) >> 8)))


// Constants
#define TRUE      			1
#define FALSE     			0


UCHAR idata debug = FALSE;      // Set to TRUE for debug
// This sets my hardware address to 00:01:02:03:04:05
UCHAR code my_hwaddr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};


// External functions
void serial_send(char *);  // Sends a string to RS-232 serial port
void wait_msec(UINT);      // Waits specified number of milliseconds


// Absolute addressses for the Cirrus CS8900A Ethernet controller
UCHAR xdata PORT_DATA_LO     	_at_ 0xF300;
UCHAR xdata PORT_DATA_HI    	_at_ 0xF301;
UCHAR xdata PORT_TX_CMD_LO    _at_ 0xF304;
UCHAR xdata PORT_TX_CMD_HI		_at_ 0xF305;
UCHAR xdata PORT_TX_LEN_LO    _at_ 0xF306;
UCHAR xdata PORT_TX_LEN_HI    _at_ 0xF307;
UCHAR xdata PORT_ISQ_LO       _at_ 0xF308;
UCHAR xdata PORT_ISQ_HI       _at_ 0xF309;
UCHAR xdata PORT_PKTPTR_LO    _at_ 0xF30A;
UCHAR xdata PORT_PKTPTR_HI    _at_ 0xF30B;
UCHAR xdata PORT_PKTDATA_LO   _at_ 0xF30C;
UCHAR xdata PORT_PKTDATA_HI   _at_ 0xF30D;




//------------------------------------------------------------------------
// This function reads a packetpage register from the CS8900A.
// It is done indirectly by first setting the desired address
// then reading the data 
//------------------------------------------------------------------------
UINT read_packetpage(UINT pp_addr)
{
   UCHAR temp;
   UINT result;

   // Set address by writing lo byte, hi byte
	PORT_PKTPTR_LO = LOBYTE(pp_addr);
	PORT_PKTPTR_HI = HIBYTE(pp_addr);
	
	// Read data lo byte, hi byte
	temp = PORT_PKTDATA_LO;
	result = (UINT)PORT_PKTDATA_HI;
	result = result << 8;
	result = result | (UINT)temp;
	return (result);
}



//------------------------------------------------------------------------
// This function writes a packetpage register from the CS8900A.
// It is done indirectly by first setting the desired address
// then writing the data 
//------------------------------------------------------------------------
void write_packetpage(UINT pp_addr, UINT pp_data)
{
   // Set address by writing lo byte, hi byte
	PORT_PKTPTR_LO = LOBYTE(pp_addr);
	PORT_PKTPTR_HI = HIBYTE(pp_addr);

	// Write data lo byte, hi byte
	PORT_PKTDATA_LO = LOBYTE(pp_data);
	PORT_PKTDATA_HI = HIBYTE(pp_data);
}



//------------------------------------------------------------------------
// Initialize the Cirrus Logic CS8900A chip
// SBHE should be tied high for 8 bit mode
//------------------------------------------------------------------------
void init_CS8900(void)
{
   UINT temp;
         
   // Reset the CS8900A
   if (debug) serial_send("CIRRUS: Initializing CS8900A\r");
   write_packetpage(PKTPG_SELF_CTL, 0x0040);

	// Wait for CS8900A to complete the reset
   wait_msec(200);
              
   // Make sure chip init was completed
	if ((read_packetpage(PKTPG_SELF_ST) & 0x0080) == 0)
   {
     	if (debug) serial_send("CIRRUS: Could not reset CS8900A\r");
   	while (1){;}		// Stay here
   }   

	// Verify registration number of CS8900A chip
   if (read_packetpage(PKTPG_EISA_NUM) != 0x630E)
   {
      if (debug) serial_send("CIRRUS: Bad CS8900A EISA number\r");   
      while(1){;} // Stay here
   }

	// Verify receiver config per FAQ
	if (read_packetpage(PKTPG_RX_CFG) != 0x0003)
	{
		if (debug) serial_send("CIRRUS: Bad CS8900A RX CONFIG\r");   
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

   if (debug) serial_send("CIRRUS: Completed CS8900A init\r");
}




//------------------------------------------------------------------------
// This functions checks CS8900A status and if it is ready then
// it sends an Ethernet frame to it.
//------------------------------------------------------------------------
int CS8900_out(UCHAR * outbuf, UINT len)
{
 	UINT i;

 	// Prime the CS8900A for transmitting
 	PORT_TX_CMD_LO = LOBYTE(0x00C0);
	PORT_TX_CMD_HI = HIBYTE(0x00C0);

	PORT_TX_LEN_LO = LOBYTE(len);
  	PORT_TX_LEN_HI = HIBYTE(len);

   // See if CS8900A is able to send.  If not return error
   if ((read_packetpage(PKTPG_BUS_ST) & 0x0100) == 0) return -4;

   // Make sure len is an even number of bytes
   if (len & 0x0001) len++;

   // Write the frame to the CS8900A
   for (i=0; i < len; i += 2)
	{
		PORT_DATA_LO = outbuf[i];
		PORT_DATA_HI = outbuf[i+1];
	}

   return 0;
}




//------------------------------------------------------------------------
// This functions checks the CS8900A receive event status
// word to see if an ethernet frame has arrived. If so,
// and frame has no errors, it reads it from the CS8900A.
// and returns pointer to frame buffer.  If no frame, or
// frame with errors it returns NULL.
//------------------------------------------------------------------------
UCHAR * CS8900_in(void)
{
	UINT i, len, temp;
	volatile UCHAR dummy;
   UCHAR *ptr;

   // Read CS8900 receive event status word
   temp = read_packetpage(PKTPG_RX_EVENT);

   // If nothing has arrived return null ptr
   if ((temp & 0x7100) == 0) return NULL;

   // If frame has errors, drop it
   if (temp & 0x7000)
   {
      // Read RX status and RX event per Cirrus AN205 FAQ
		dummy = PORT_DATA_HI;
		dummy = PORT_DATA_LO;
		dummy = PORT_DATA_HI;
		dummy = PORT_DATA_LO;

      // Tell CS8900 to drop the frame
		write_packetpage(PKTPG_RX_CFG, 0x0140);
      if (debug) serial_send("CS8900: Rcvd bad frame\r");
      return NULL;
   }

   // If we get this far, a good frame has arrived
   // The first word is status which I do not need
   dummy = PORT_DATA_HI;
	dummy = PORT_DATA_LO;

	// Second word is length in bytes
   len = (UINT)PORT_DATA_HI;
	len = len << 8;
	len = len | (UINT)PORT_DATA_LO;

	// Make sure len is an even number of bytes
	if (len & 0x0001) len++;

   // Allocate memory for incoming frame
   ptr = (UCHAR *)malloc(len);
   if (ptr == NULL)
   {
   	// Not enough memory - try to read frame again later
   	if (debug) serial_send("CS8900: Not enough RAM\r");
   	return NULL;
   }

   // Read incoming frame into buffer
   for (i=0; i < len; i += 2)
	{
		ptr[i] = PORT_DATA_LO;
		ptr[i+1] = PORT_DATA_HI;
   }

   if (debug) serial_send("CS8900: Good frame rcvd\r");
   return ptr;
}


