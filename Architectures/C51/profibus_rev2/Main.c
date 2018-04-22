//-----------------------------------------------------------------------------
// MAIN.C  
// Jim Brady Dec 2005
// Source code for Profibus test board
// Target: Atmel AT89C52 11.0592MHz with 64K RAM and 64K flash 
// Compiler: Keil C51 version 7 with large memory model
//
// This module contains main()
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "AT89C52.h"
#include "main.h"
#include "util.h"
#include "spc3.h"



// Global variables
// For this test program, the SPC3 is set to exchange 16 bytes of  
// input and 16 bytes of output.  So SPC3BUFSIZE = 16 
UCHAR spc3_rcve_buf[SPC3BUFSIZE], spc3_send_buf[SPC3BUFSIZE];
char rs232_rcve_buf[RS232BUFSIZE], rs232_send_buf[RS232BUFSIZE];
UCHAR event, rs232_rcve_idx, rs232_send_idx;
TIMER timer;


// These are declared volatile to make sure compiler does not
// optimize out accesses to I/O 
volatile union
{
  	PROFREADTYPE   		PROFRD;
	PROFWRITETYPE  		PROFWR;
	PROFORGTYPE				PROF;
} NET _at_ 0xF800;

volatile UCHAR NETBRD_PORTA		_at_ 0xF600;
volatile UCHAR NETBRD_PORTB		_at_ 0xF601;
volatile UCHAR NETBRD_PORTC		_at_ 0xF602;
volatile UCHAR NETBRD_CONTROL		_at_ 0xF603;



//-----------------------------------------------------------------------------
// This is where execution starts on powerup
// after initializing global and static variables
//-----------------------------------------------------------------------------
void main(void)
{
   UCHAR i, temp;
   UINT idx;
   
   // This is just to make sure these get initiaized
   memset(spc3_rcve_buf, 0, SPC3BUFSIZE);
   memset(spc3_send_buf, 0, SPC3BUFSIZE);
   memset(rs232_rcve_buf, 0, RS232BUFSIZE);
   memset(rs232_send_buf, 0, RS232BUFSIZE);  
	memset(&timer, 0, sizeof(TIMER));
   event = 0;
	rs232_rcve_idx = 0;
   rs232_send_idx = 0;


   
	// Setup RS232 UART in the Atmel CPU for 19.2K baud,N,8,1
	PCON = 0x80;   // SMOD bit = 1
	SCON = 0x70;
	TMOD = 0x20;
	TH1 = 0xFD;    	
	TR1 = 1;


   // Setup periodic timer 
	// Setup Timer 0 for 50.0mS, mode 1, based on 11.0592MHz
	TH0 = 0x4C;
	TL0 = 0;
	TMOD |= 0x01;
	TR0 = 1;

   // Setup 82C55A parallel I/O chip on Network board
	NETBRD_CONTROL = 0x92;	// port A = IN, B = IN, C = OUT
	NETBRD_PORTC  = 0x03;   // LEDs OFF on Network board, other bits zero
	
     
   // Initialize SPC3 chip
   spc3_init();

   // Set timers
   timer.send_banner = 10;
   timer.half_second = 10;      

   ES = 1;  	// Enable UART in 8052 chip
	ET0 = 1;		// Enable timer interrupt
	EX0 = 1;		// Enable Profibus SPC3 interrupt
	EA = 1;  	// Global interurupt enable
   
   // Put Profibus SPC3 chip online
	NET.PROFWR.MODE1_S = 0x01;

   // Write first data to SPC3 even though it is all zeros
   spc3_write_data(spc3_send_buf);  

   // "event" is a global event word - particular bits in the
   // word represent events that may be set in an interrupt handler
   // so access to "event" is protected by disabling interrupts
   while (1)   
   {
	   // Kick watchdog in SPC3 Profibus chip
      NET.PROFWR.MODE1_S = 0x20;
	   
 	   // See if data exchange message has been received
      EA = 0;
 	  	if (event & EVENT_PROF_DATA_MSG)
		{
			event &= (~EVENT_PROF_DATA_MSG); // Clear the event bit
		   EA = 1; 
         spc3_read_data(spc3_rcve_buf);
      }
           
	   // See if SPC3 chip should be hardware reset
		EA = 0;
		if (event & EVENT_PROF_RST_REQUEST)
		{
		   event &= (~EVENT_PROF_RST_REQUEST);  // Clear the event bit
		  	// Leave interruupts off during this
         spc3_init();
			wait_millisecond(1);
			EA = 1; 
         NET.PROFWR.MODE1_S = 0x01;  // put SPC3 back online
		}
    
		// See if RS232 message is in from UART 
 	  	EA = 0;
 	  	if (event & EVENT_RS232_MSG)
		{
			event &= (~EVENT_RS232_MSG); // Clear the event bit
		   EA = 1;
         temp = (UCHAR)strtol(rs232_rcve_buf, NULL, 16);
         
         // Copy value into all bytes to send to master
         // These bytes are sent every half second below
         for (i=0; i < SPC3BUFSIZE; i++) spc3_send_buf[i] = temp;
         
         // Clear RS232 receive buffer for next input
         memset(rs232_rcve_buf, 0, RS232BUFSIZE);
         rs232_rcve_idx = 0;

         
         // Move incoming Profibus buffer to RS232 buffer and
         // send out to serial port
         idx = 0;
         for (i=0; i < SPC3BUFSIZE; i++)
         {
            idx += sprintf(rs232_send_buf + idx, "%02X ", (UINT)spc3_rcve_buf[i]);
         }
         strcat(rs232_send_buf, "\r");
         rs232_send_idx = 0;
         SBUF = rs232_send_buf[0];  // Start sending
      }
      
      
      EA = 0;
      if (event & EVENT_HALF_SECOND)
      {
         event &= (~EVENT_HALF_SECOND); // Clear the event bit
         timer.half_second = 10;  // Reset timer for 0.5 sec 
         EA = 1;
         
         // Write to SPC3 every half second.  It is important to
         // write regularly to prevent the watchdog from timing out
         spc3_write_data(spc3_send_buf);

         // Blink green LED on network board
         if (NETBRD_PORTC & 0x02) NETBRD_PORTC = 0x01; // Green on
 		   else NETBRD_PORTC = 0x03; // LEDs off
      }


      EA = 0;
      if (event & EVENT_SEND_BANNER)
      {
         event &= (~EVENT_SEND_BANNER); // Clear the event bit
         EA = 1;
         
         display_banner();    
      }
            
      // Idle for a while with interrupts enabled
      EA = 1;
      wait_millisecond(10);

   }
}




