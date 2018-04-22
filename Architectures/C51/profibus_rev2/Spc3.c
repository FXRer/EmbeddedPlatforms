//-----------------------------------------------------------------------------
// SPC3.C                                         	
// Jim Brady Dec 2005
// Source code for Profibus test board
// Target: Atmel AT89C52 11.0592MHz with 64K RAM and 64K flash 
// Compiler: Keil C51 version 7 with large memory model
//
// This module contains functions for Siemens SPC3 chip
// The SPC3 is mapped at address 0xF800
//-----------------------------------------------------------------------------

#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include "AT89C52.h"
#include "main.h"
#include "util.h" 
#include "spc3.h"


// Global variables
// Use volatile to prevent compiler from optimizing these out
extern volatile union
{
  	PROFREADTYPE   		PROFRD;
	PROFWRITETYPE  		PROFWR;
	PROFORGTYPE				PROF;
} NET;

extern volatile UCHAR NETBRD_PORTA;
extern volatile UCHAR NETBRD_PORTB;
extern volatile UCHAR NETBRD_PORTC;
extern volatile UCHAR NETBRD_CONTROL;



// Variables local to this module
// The xdata here may seem redundant with using large memory model, 
// but the Keil compiler saves generic pinters as 3 byte values
// that could cause problem making a pointer from an integer.  
static UCHAR xdata *spc3_read_ptr;
static UCHAR xdata *spc3_write_ptr;





//-----------------------------------------------------------------------------
// Exchange SPC3 read buffers and return a pointer to the user buffer
// 
//-----------------------------------------------------------------------------
UCHAR *spc3_exchange_read_bufs(void)
{
	UCHAR idx;
	UCHAR xdata *ptr;

	// Exchange buffers and get index to user buffer
	idx = NET.PROFRD.NEXTDOUT_CMD & 0x03;
   
	switch(idx)
	{
		case 1:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DOUTBUF1 << 3));
		break;

		case 2:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DOUTBUF2 << 3));
		break;

		case 3:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DOUTBUF3 << 3));
      break;

		default:
		ptr = NULL;
		break;
	}
     
	return ptr;
}



//-----------------------------------------------------------------------------
// Exchange SPC3 write buffers and return a pointer to the user buffer
// 
//-----------------------------------------------------------------------------
UCHAR *spc3_exchange_write_bufs(void)
{
	UCHAR idx;
	UCHAR xdata *ptr;

	// Exchange buffers and find out where to write data to SPC3
	idx = NET.PROFRD.NEWDIN_CMD & 0x03;

   switch(idx)
	{
		case 1:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF1 << 3));
		break;

		case 2:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF2 << 3));
		break;

		case 3:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF3 << 3));
		break;

		default:
		ptr = NULL;
		break;

	}
 
 	return ptr;
}


//-----------------------------------------------------------------------------
// Find out where to write first outgoing data to SPC3
// This does not exchange buffers
//-----------------------------------------------------------------------------
UCHAR *spc3_get_first_write_ptr(void)
{
	UCHAR idx;
	UCHAR xdata *ptr;

	idx = (NET.PROFRD.DINBUF_STATE >> 4) & 0x03;

 	switch(idx)
	{
		case 1:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF1 << 3));
		break;

		case 2:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF2 << 3));
		break;

		case 3:
		ptr = (UCHAR xdata *)(UINT)(0xF800 + (NET.PROF.PTR_DINBUF3 << 3));
		break;

		default:
		ptr = NULL;
		break;
   }

	return ptr;
}



//-----------------------------------------------------------------------------
// Load the configuration bytes into the SPC3
// This is fixed at 4 bytes in length
//-----------------------------------------------------------------------------
void spc3_load_config(void)
{
	UINT temp;
	UCHAR xdata *ptr;

	// Get pointer to config data area
	temp = (UINT)NET.PROF.PTR_RDCONFIGBUF;
	temp = temp << 3;
	ptr = (UCHAR xdata *)(UINT)(0xF800 + temp);
	
   *ptr = 0x1F; ptr++;
   *ptr = 0x2F;	
}



//-----------------------------------------------------------------------------
// Read incoming Profibus data into buffer
//-----------------------------------------------------------------------------
void spc3_read_data(UCHAR *buf)
{
	UCHAR i, state;
		
	// If not in data exchange state, do not read data
	state = 0x03 & (NET.PROFRD.STATUS_LO >> 4);
	if (state != 2) return;
	
	// Find out where to read from SPC3
	spc3_read_ptr = spc3_exchange_read_bufs();

	// If read pointer is NULL, do not read data
	if (spc3_read_ptr == NULL) return;

   // Put 16 bytes incoming Profibus data into buffer
  	for (i=0; i < SPC3BUFSIZE; i++)
	{
		buf[i] = *spc3_read_ptr;
		spc3_read_ptr++;
	}
}



//-----------------------------------------------------------------------------
// Updates outgoing data buffer in SPC3
//-----------------------------------------------------------------------------
void spc3_write_data(UCHAR *buf)
{
	UCHAR i;

	if (spc3_write_ptr == NULL) return;
   	
	// Write 16 bytes of data to SPC3 chip
	for (i=0; i < SPC3BUFSIZE; i++)
   {
	   *spc3_write_ptr = buf[i];
		spc3_write_ptr++;
   }

   // Exchange buffers and find out where to next write data to SPC3
	spc3_write_ptr = spc3_exchange_write_bufs();
}




//-----------------------------------------------------------------------------
// Setup registers in Siemens SPC3 Profibus chip
//-----------------------------------------------------------------------------
void spc3_init(void)
{
	UCHAR address, temp, units, tens, hundreds;
	UINT i, utemp;
	UCHAR xdata *ptr;

   // Reset the SPC3 by writing a 1 to the bit2 of the 82C55 chip
   NETBRD_PORTC |= 0x04;
	wait_millisecond(1);
	NETBRD_PORTC &= (~0x04);
	wait_millisecond(1);

   // Get the network address from the switches on the board
   // Each switch is 1 BCD nibble with range 0-9
   temp = NETBRD_PORTA;
	units = bit_reverse(0x0F & temp);         // units digit
	tens = bit_reverse(0x0F & (temp >> 4));  // tens digit
	
	// Get hundreds digit from third switch
	temp = NETBRD_PORTB;
	hundreds = bit_reverse(0x0F & temp);  // hundreds digit
   	
   address = 100 * hundreds + 10 * tens + units;
   if (address > 125) address = 125;

	// Clear SPC3 memory starting at register address 0x16
	ptr = (UCHAR xdata *)(UINT)NET.PROF.BASE;		// Address of SPC3 chip
	for (i=0; i <= 0x5FF; i++)
	{
		if (i >= 0x16) *ptr = 0;
		ptr++;
	}

	// Initialize the write parameter area of SPC3 chip
	NET.PROFWR.INTREQ_LO 		= 0x00;
	NET.PROFWR.INTREQ_HI 		= 0x00;
	NET.PROFWR.INTACK_LO 		= 0x00;
	NET.PROFWR.INTACK_HI 		= 0x00;
	NET.PROFWR.INTMASK_LO 		= 0xF1;

   // A low bit enables the interrupt
	NET.PROFWR.INTMASK_HI 		= 0xC0;

	NET.PROFWR.MODE0_LO 			= 0xC0;
	NET.PROFWR.MODE0_HI 			= 0x05;
	NET.PROFWR.MODE1_S 			= 0x00;
	NET.PROFWR.MODE1_R 			= 0x00;
	NET.PROFWR.BAUD_CONTROL 	= 0x1E;
	NET.PROFWR.MIN_TSDR 			= 0x0B;		// 11 bit times


	// Setup organizational parameter area of SPC3 chip
	// The amount of I/O data is set to 16 bytes for this test program
	NET.PROF.ADDRESS 				= address;
	NET.PROF.PTR_SAPLIST 		= 0x79;
	NET.PROF.WDOG_LO 				= 0x20;			 // 20,000
	NET.PROF.WDOG_HI 				= 0x4E;
	NET.PROF.LEN_DOUTBUFS 		= 0x10;			 // 16 bytes of input data
	NET.PROF.PTR_DOUTBUF1		= 0x08;			 
	NET.PROF.PTR_DOUTBUF2		= 0x0A;
	NET.PROF.PTR_DOUTBUF3		= 0x0C;
	NET.PROF.LEN_DINBUFS 		= 0x10;			 // 16 bytes of output data
	NET.PROF.PTR_DINBUF1 		= 0x0E;
	NET.PROF.PTR_DINBUF2 		= 0x10;			 // @ 0x20
	NET.PROF.PTR_DINBUF3 		= 0x12;
	NET.PROF.LEN_DDBOUTBUF 		= 0x00;
	NET.PROF.PTR_DDBOUTBUF 		= 0x00;
	NET.PROF.LEN_DIAGBUF1 		= 0x06;
	NET.PROF.LEN_DIAGBUF2 		= 0x06;			 // @ 0x25
	NET.PROF.PTR_DIAGBUF1 		= 0x65;
	NET.PROF.PTR_DIAGBUF2 		= 0x69;
	NET.PROF.LEN_CTRLBUF1 		= 0x18;
	NET.PROF.LEN_CTRLBUF2 		= 0x00;
	NET.PROF.AUXBUF_SEL 			= 0x00;			 // @ 0x2A
	NET.PROF.PTR_AUXBUF1 		= 0x76;
	NET.PROF.PTR_AUXBUF2 		= 0x79;
	NET.PROF.LEN_SSADATA 		= 0x00;
	NET.PROF.PTR_SSADATA 		= 0x00;
	NET.PROF.LEN_PARAMBUF 		= 0x14;
	NET.PROF.PTR_PARAMBUF		= 0x73;			 // @ 0x30
	NET.PROF.LEN_CONFIGBUF		= 0x0A;
	NET.PROF.PTR_CONFIGBUF 		= 0x6D;
   NET.PROF.LEN_RDCONFIGBUF	= 0x02; 	       // 2 bytes of config data
   NET.PROF.PTR_RDCONFIGBUF	= 0x70;
	NET.PROF.LEN_DDBPARAMBUF	= 0x00;			 // @ 0x35
	NET.PROF.PTR_DDBPARAMBUF	= 0x00;
	NET.PROF.SCORE_EXPBYTE		= 0x00;
	NET.PROF.SCORE_ERRORBYTE	= 0x00;
	NET.PROF.ADDRESS_CHANGE		= 0xFF;
	NET.PROF.IDENT_LO				= 0x08;			 // @ 0x3A
	NET.PROF.IDENT_HI				= 0x00;
	NET.PROF.GC_COMMAND			= 0x00;
	NET.PROF.LEN_SPECPARAMBUF	= 0x00;			 // @ 0x3D

	// Load 0xFF into location pointed to by the sap list pointer
	utemp = (UINT)NET.PROF.PTR_SAPLIST;
	utemp = utemp << 3;
	ptr = (UCHAR xdata *)(UINT)(0xF800 + utemp);
	*ptr = 0xFF;

	// Load the two byte read-configuration
	spc3_load_config();

	// Find out where to write first outgoing data to SPC3
	// without exchanging buffers
	spc3_write_ptr = spc3_get_first_write_ptr();
}

