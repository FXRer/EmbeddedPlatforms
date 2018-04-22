//-----------------------------------------------------------------------------
// UTIL.C
// Jim Brady Dec 2005
// Source code for Profibus test board
// Target: Atmel AT89C52 11.0592MHz with 64K RAM and 64K flash
// Compiler: Keil C51 version 7 with large memory model
//
//	This module contains general utility stuff
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "AT89C52.h"
#include "main.h"
#include "util.h"

// Global variables
extern char rs232_send_buf[RS232BUFSIZE];
extern UCHAR rs232_send_idx;
extern TIMER timer;


char code banner[3][80] =
{ "Profibus code, Rev 2, Jim Brady, Dec 2005\r",
  "Press CR to see most recent data from master\r",
  "Or enter a value 0-FF hex to send to master\r"};



//-----------------------------------------------------------------------------
// This provides a busywait delay.  Caution - Only for Atmel AT89C52
// at 11.0592MHz with Keil v7 optimization level 5
//-----------------------------------------------------------------------------
void wait_millisecond(UINT msec)
{
	// Make sure these are xdata space (take longer to access)
	volatile UINT xdata i, j;

  	// This loop takes about 1.0 msec
	for (i = 0; i < msec; i++)
	{
		for (j=0; j < 36; j++){;}
   }
}




//-----------------------------------------------------------------------------
// This corrects an error on the PC board layout where the address switch
// connections to the 82C55 chip are reversed.  This un-reverses them
// It also inverts them because the switches pull down the inputs
//-----------------------------------------------------------------------------
UCHAR bit_reverse(UCHAR inp)
{
	UCHAR result;

	result = 0x00;
	if ((inp & 0x01) == 0) result |= 0x08;
	if ((inp & 0x02) == 0) result |= 0x04;
	if ((inp & 0x04) == 0) result |= 0x02;
	if ((inp & 0x08) == 0) result |= 0x01;

	return result;
}



//-----------------------------------------------------------------------------
// This sends a startup banner over the serial port
// Interrupts are disabled when this runs so use a kludgy
// polling approach to figure out when a char has been sent 
//-----------------------------------------------------------------------------
void display_banner(void)
{
   static UCHAR line_nr = 0;

   if (line_nr <= 2) // Send a total of 3 lines
   {
      strcpy(rs232_send_buf, banner[line_nr]);
      line_nr++;
      rs232_send_idx = 0;
      SBUF = rs232_send_buf[0];
      // Reset timer to send next line in 100 msec      
      EA = 0; timer.send_banner = 2; EA = 1;
   }
}


