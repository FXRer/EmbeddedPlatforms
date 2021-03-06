//=========================================================================//
//-------------------------------------------------------------------------//
// rs232int.h -- Header for RS232Interface class                           //
// This file is part of PonyProg.                                          //
//-------------------------------------------------------------------------//
//                                                                         //
//  PonyProg - Serial Device Programmer                                    //
//                                                                         //
//  Copyright (C) 1997-1999   Claudio Lanconelli                           //
//                                                                         //
//  e-mail: lanconel@cs.unibo.it                                           //
//  http://www.cs.unibo.it/~lanconel                                       //
//                                                                         //
//-------------------------------------------------------------------------//
//                                                                         //
// This program is free software; you can redistribute it and/or           //
// modify it under the terms of the GNU  General Public License            //
// as published by the Free Software Foundation; either version2 of        //
// the License, or (at your option) any later version.                     //
//                                                                         //
// This program is distributed in the hope that it will be useful,         //
// but WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
// General Public License for more details.                                //
//                                                                         //
// You should have received a copy of the GNU  General Public License      //
// along with this program (see COPYING);     if not, write to the         //
// Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. //
//                                                                         //
//-------------------------------------------------------------------------//
//=========================================================================//

#ifndef	_RS232INTERFACE_H
#define	_RS232INTERFACE_H

//#ifdef	vDEBUG
//#include <v/vdebug.h>
//#endif

#ifdef	_LINUX_
#include <termios.h>
#endif

#ifdef	_WINDOWS
#include <windows.h>
#endif

#include "types.h"
//#include "e2profil.h"

#define	MAX_COMPORTS	4

class RS232Interface
{
  public:		//------------------------------- public

	RS232Interface(int com_no = 0);
	virtual ~RS232Interface();

	int OpenSerial(int no);
	void CloseSerial();

	void SerialFlushRx();
	void SerialFlushTx();
	void WaitForTxEmpty();
	long ReadSerial(BYTE *buffer, DWORD len);
	long WriteSerial(BYTE *buffer, DWORD len, int wait_end_tx = 1);
	int SetSerialParams(long speed = -1, int bits = -1, int parity = -1, int stops = -1, int flow_control = -1);
	int SetSerialTimeouts(long init_read = -1, long while_read = -1);
	void SetSerialEventMask(long mask);
	int SetSerialBreak(int state);

	int SetSerialDTR(int dtr);
	int SetSerialRTS(int rts);
	int GetSerialDSR() const;
	int GetSerialCTS() const;
	int SetSerialRTSDTR(int state);

  protected:		//------------------------------- protected

  private:		//------------------------------- private

	long read_total_timeout, read_interval_timeout;

	long actual_speed;
	int actual_bits, actual_parity, actual_stops;
	int actual_flowcontrol;

#ifdef	_WINDOWS
	HANDLE hCom;

	DWORD old_mask;
	COMMTIMEOUTS old_timeout;
	DCB old_dcb;
#else
# ifdef	_LINUX_
	int fd; 
	struct termios old_termios;
	char lockname[256]; 

//	int OpenPort(int no); 
# endif
#endif
};

#endif
