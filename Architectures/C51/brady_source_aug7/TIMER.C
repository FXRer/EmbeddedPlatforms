//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net TIMER.C
//
// This module sets up the timer and handles the timer interrupt
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include "C8051f.h"
#include "net.h"
#include "timer.h"


extern UINT volatile event_word;
extern ULONG idata initial_sequence_nr;


//--------------------------------------------------------------------------
// Timer 2 interrupt service routing. Intr vector location is
// address 0x002B. The timer generates an interrupt every 25 msec
// It is set to auto reload so do not need to reload it.
//--------------------------------------------------------------------------
void timer2_interrupt(void) interrupt 5
{
	static UINT count1 = 0;
	static UINT count2 = 0;
	
	TF2 = 0;		// Clear interrupt flag

	
	// Advance the initial sequence number
	initial_sequence_nr += 6250;
   // Keep it some distance from rolling over
   if (initial_sequence_nr & 0x10000000L) initial_sequence_nr = 1;
	
	count1++;
	// These events happens every 0.50 seconds, not simultaneously
	if (count1 == 5) event_word |= EVENT_ARP_RETRANSMIT;
	
	if (count1 == 10) event_word |= EVENT_TCP_INACTIVITY;
	
	if (count1 == 15) event_word |= EVENT_READ_ANALOG;
	
	if (count1 == 20)
	{
	 	count1 = 0;
	 	event_word |= EVENT_TCP_RETRANSMIT;
	}
	
	count2++;
	if (count2 == 2401)
	{
		// This happens every 60.025 seconds, not simultaneous
		// with above tasks
		count2 = 0;
		event_word |= EVENT_AGE_ARP_CACHE;
	}
}





//--------------------------------------------------------------------------
// Setup Timer2 in mode 1 which is 16 bit auto reload
// Intr rate = 22.1184 MHz /(12 * (65536 - reload value))
// For 25 msec reload value = 19456 = 0x4C00
//--------------------------------------------------------------------------
void init_timer2(void)
{
	T2CON = 0x00;
	RCAP2H = 0x4C; 		// Reload value
	RCAP2L = 0x00;
	TH2 = 0x4C;				// Initial load value
	TL2 = 0x00;
	TR2 = 1;					// Timer 2 run
}


