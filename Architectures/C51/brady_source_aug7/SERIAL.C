//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net SERIAL.C
//
// This module handles RS-232 messages and associated tasks
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include "C8051f.h"
#include "net.h"
#include "serial.h"


char xdata serial_buf[BUFSIZE];
UCHAR serial_idx;
extern UINT volatile event_word;
extern char xdata text[];


//--------------------------------------------------------------------------
// This is the interrupt handler for the UART in Cygnal chip
//--------------------------------------------------------------------------
void uart_interrupt(void) interrupt 4
{
   UCHAR idata dummy;
   
   if (RI)
	{
		// This interrupt happens when a character has been
		// received into UART
		RI = 0;
		
		// For now, this program does not do anything with
		// received RS232 messages so just discard the char
		dummy = SBUF;
      event_word |= EVENT_RS232_ARRIVED;
	}

	if (TI)
	{
		// This interrupt happens when the UART has sent the previous
		// char. If there is another char to send, send it now
		TI = 0;
		serial_idx++;
		if ((serial_buf[serial_idx] != 0) && (serial_idx < BUFSIZE))
		{
			SBUF = (UCHAR)serial_buf[serial_idx];  // send next char
		}
	}
}



//--------------------------------------------------------------------------
// Setup RS232 UART in the Cygnal chip.  UART uses timer 1 in mode 2
// to generate baud rate.  SMOD = 0 and T1M = 1 so baud rate formula is
// Baud rate = 22.1184 MHz /(32 * (256 - TH1))
// For 9600 baud TH1 = 0xB8, for 115.2K TH1 = 0xFA
//--------------------------------------------------------------------------
void init_serial(void)
{
	CKCON = 0x10;        // T1M = 1
	SCON = 0x50;
	TMOD = 0x20;
	TH1 = 0xFA;          // 115.2K baud    	
	TR1 = 1;

	memset(serial_buf, 0, sizeof(serial_buf));
	serial_idx = 0;
}




//------------------------------------------------------------------------
//	Waits a number of milliseconds.  Based on measurements with
// Keil compiler set to Optimize level 2, a value of 2000 
// produces about 1 millisecond delay.
//------------------------------------------------------------------------
void wait_msec(UINT msec)
{
    UINT i, delay;

    for (i=0; i < msec; i++)
    {
        delay = 2000;
        while (delay--);
    }
}


 
//------------------------------------------------------------------------
//	Send a string out over RS232.  Caution -
//	Make sure str is null terminated
//------------------------------------------------------------------------
void serial_send(char *str)
{
	UCHAR idata duration;
   	
	strcpy(serial_buf, str);
	serial_idx = 0;
	// Send first char here, the rest of the chars will
   // be sent from the interrupt handler
	SBUF = (UCHAR)serial_buf[0];		
	
	// Wait here for message to get sent to avoid 
	// Wait time is about 250 usec per char
	duration = (UCHAR)strlen(str);
	duration = 1 + (duration / 4);
	wait_msec((UINT)duration);
}



//------------------------------------------------------------------------
//	For debug.  Converts an byte to text and places the
// text in a global buffer called text[].
//------------------------------------------------------------------------
void byte2text(UCHAR val)
{
   UCHAR temp;
   
   temp = 0x000F & (val >> 4); 
   if (temp > 9) text[0] = temp + 55; else text[0] = temp + 48;
   temp = 0x000F & val; 
   if (temp > 9) text[1] = temp + 55; else text[1] = temp + 48;
   text[2] = 0x20;   // append space
}



//------------------------------------------------------------------------
// This function converts an integer to an ASCII string.  It is a 
// normally provided as a standard library function but the Keil
// libraries do not include it.  Caution: The string passed to this
// must be at least 12 bytes long
//------------------------------------------------------------------------
char * itoa(UINT value, char * buf, UCHAR radix)
{
	UINT i;
	char * ptr;
	char * temphold;

  	temphold = buf;
	ptr = buf + 12;
	*--ptr = 0;		// Insert NULL char
	do
	{
	   // First create string in reverse order
	   i = (value % radix) + 0x30;
		if(i > 0x39) i += 7;
		*--ptr = i;
      value = value / radix;
  	} while(value != 0);

	// Next, move the string 6 places to the left
	// Include NULL character
	for( ; (*buf++ = *ptr++); );	
	return(temphold);
}


