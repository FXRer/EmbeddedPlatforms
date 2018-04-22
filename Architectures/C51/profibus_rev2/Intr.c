//-----------------------------------------------------------------------------
// INTR.C                                            	
// Jim Brady Dec 2005
// Source code for Profibus test board
// Target: Atmel AT89C52 11.0592MHz with 64K RAM and 64K flash 
// Compiler: Keil C51 version 7 with large memory model
//
// This module contains interrupt handlers
//-----------------------------------------------------------------------------

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include "AT89C52.h" 
#include "main.h"


// Global variables
extern char rs232_rcve_buf[RS232BUFSIZE], rs232_send_buf[RS232BUFSIZE];
extern UCHAR event, rs232_rcve_idx, rs232_send_idx;
extern TIMER timer;
extern volatile union
{
  	PROFREADTYPE   		PROFRD;
	PROFWRITETYPE  		PROFWR;
	PROFORGTYPE				PROF;
} NET;




//-----------------------------------------------------------------------------
// This timer happens every 50.00 milliseconds
//-----------------------------------------------------------------------------
void timer0(void) interrupt 1
{
  	// Stop timer, set for 50mS, restart timer
	TR0=0;
	TH0=0x4C;
	TL0=0;
	TR0=1;
								
   // Handle timer for startup banner
   if (timer.send_banner)
   {   
      timer.send_banner--;
      if (timer.send_banner == 0) event |= EVENT_SEND_BANNER;
   }

   // Handle half second timer
   if (timer.half_second)
   {
      timer.half_second--;
      if (timer.half_second == 0) event |= EVENT_HALF_SECOND;
   }
}

 

//-----------------------------------------------------------------------------
// Verify that the configuration message sent by the Master matches
// my desired configuration.  This is hard coded for 16 bytes input
// and 16 bytes output
//-----------------------------------------------------------------------------
UCHAR spc3_check_config(void)
{
	UCHAR length, result, data0, data1;
	UCHAR xdata *ptr;
	UINT temp;

   // The xdata above may seem redundant with using large memory model, 
   // but the Keil compiler saves generic pinters as 3 byte values
   // that could cause problem making a pointer from an integer.  

	// Get length of config data
	length = NET.PROF.LEN_CONFIGBUF;
	
	// Get pointer to config data, then get data
	temp = (UINT)NET.PROF.PTR_CONFIGBUF;
	temp = temp << 3;
	ptr = (UCHAR xdata *)(UINT)(0xF800 + temp);
	data0 = *ptr; ptr++;
	data1 = *ptr;


   // Configuration should be two bytes, 0x1F and 0x2F
	// this represents 16 bytes input and output
   if ((length == 2) && (data0 == 0x1F) && (data1 == 0x2F))
	{
		NET.PROFWR.MODE1_R = 0x10;
		result = NET.PROFRD.CONFIGDATA_OK;
	}
	else	  // not OK
	{
		NET.PROFWR.MODE1_R = 0x10;
		result = NET.PROFRD.CONFIGDATA_NOTOK;
	}
	return result;
}




//-----------------------------------------------------------------------------
// Verify that the parameterization message sent by the Master matches
// my desired params.  This is hard coded to just check that there are 
// no more than 20 bytes of parameters - somewhat arbitrary
// The Siemens DPMT program sends 12 bytes of parameters
//-----------------------------------------------------------------------------
UCHAR spc3_check_params(void)
{
	UCHAR length, result;
	UCHAR xdata *ptr;
	UINT temp;

	// Get length of parameter data
	length = NET.PROF.LEN_PARAMBUF;


   // Get pointer to parameter data
	temp = (UINT)NET.PROF.PTR_PARAMBUF;
	temp = temp << 3;
	ptr = (UCHAR xdata *)(UINT)(0xF800 + temp);

	// Accept up to 10 bytes of param data
	if (length > 10) result = NET.PROFRD.PARAMDATA_NOTOK;
	else result = NET.PROFRD.PARAMDATA_OK;

	// If no conflict in params, load min turn around time
	if (result != 1) NET.PROFWR.MIN_TSDR = *(ptr + 3);

	return result;
}




//-----------------------------------------------------------------------------
// Responds to interrupt on the INT0 pin
// Profibus network board
//-----------------------------------------------------------------------------
void external_interrupt0(void) interrupt 0
{
	UCHAR i, result;


   if (NET.PROFRD.INT_LO & 0x02)	 // Go-leave data exchange
	{
		// Don't need to do anything for this
		NET.PROFWR.INTACK_LO = 0x02;
	}

	if (NET.PROFRD.INT_LO & 0x04)	 // Baud rate has been detected
	{
      // Don't need to do anything for this
      NET.PROFWR.INTACK_LO = 0x04;
	}

	if (NET.PROFRD.INT_LO & 0x08)	 // Communications watchdog timeout
	{
		// Set event bit to reset SPC3, and re-initialize chip
		event |= EVENT_PROF_RST_REQUEST;
		NET.PROFWR.INTACK_LO = 0x08;
	}

   if (NET.PROFRD.INT_HI & 0x01)	 // Global control msg was received
	{
      // Don't need to do anything for this
      NET.PROFWR.INTACK_HI = 0x01;
	}

	if (NET.PROFRD.INT_HI & 0x02)	 // Master has assinged a new address
	{
      // We do not support address change
      NET.PROFWR.INTACK_HI = 0x02;
	}

	if (NET.PROFRD.INT_HI & 0x04)	 // Config message received
	{
	  	// Do config check up to 3 times
		for (i=0; i < 3; i++)
		{
			result = spc3_check_config();
         if (result != 1) break;
		}
	}

	if (NET.PROFRD.INT_HI & 0x08)	 // Parameter message received
	{
      // Do parameter check up to 3 times
		for (i=0; i < 3; i++)
		{
		   result = spc3_check_params();
         if (result != 1) break;
		}
	}


   if (NET.PROFRD.INT_HI & 0x10)	 // Diagnostics message received
	{
      // Don't need to do anything for this
      NET.PROFWR.INTACK_HI = 0x10;
	}


   if (NET.PROFRD.INT_HI & 0x20)	 // Data exchange message received
	{
      // Set event bit that we have received data from master
      event |= EVENT_PROF_DATA_MSG;
      NET.PROFWR.INTACK_HI = 0x20;
	}


	// Set End Of Interrupt (EOI)
   NET.PROFWR.MODE1_S = 0x02;

}




//-----------------------------------------------------------------------------
// Responds to an interrupt for the Atmel CPU UART
// Indicating a char arrived (RI) or a char has been sent (TI)
//-----------------------------------------------------------------------------
void uart_interrupt(void) interrupt 4
{
	char ch;

	if (RI)
	{
		RI = 0;
		ch = (char)SBUF;

	  	// Put received character into buffer
		rs232_rcve_buf[rs232_rcve_idx] = ch;
		if (rs232_rcve_idx < (RS232BUFSIZE - 1)) rs232_rcve_idx++;
      if (ch == RETURN) event |= EVENT_RS232_MSG;
	}

	if (TI)
	{
		TI = 0;
		if (rs232_send_idx < (RS232BUFSIZE - 1)) rs232_send_idx++;
		if (rs232_send_buf[rs232_send_idx] != 0)
		{
		   // Send next char
     		SBUF = (UCHAR)rs232_send_buf[rs232_send_idx];
		}
	}
}


