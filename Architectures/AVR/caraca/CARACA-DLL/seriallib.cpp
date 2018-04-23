//=========================================================================//
//-------------------------------------------------------------------------//
// seriallib.cpp -- DLL APIs                                               //
// This file is part of CARACA                                             //
//
// CARACA serial library work with CANDONGLE interface connected to the
// PC COM port
// Based on work of Konrad Riedel
//-------------------------------------------------------------------------//
//                                                                         //
//  PonyProg - Serial Device Programmer                                    //
//                                                                         //
//  Copyright (C) 1999-2000   Claudio Lanconelli                           //
//                                                                         //
//  e-mail: lanconel@cs.unibo.it                                           //
//  http://caraca.sourceforge.net                                          //
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
//                                                                  `      //
// You should have received a copy of the GNU  General Public License      //
// along with this program (see COPYING);     if not, write to the         //
// Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. //
//                                                                         //
//-------------------------------------------------------------------------//
//=========================================================================//


#include "seriallib.h"
#include "errcode.h"

RS232Interface slib;

static int timeout_val;

BOOL WINAPI DllMain (HANDLE hInst, 
                     ULONG ul_reason_for_call,
                     LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		//
		timeout_val = 10;
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		//
		break;
    }
    return TRUE;
}

long WINAPI CAN_Init(long port)
{
	long rv = slib.OpenSerial(port);
	if (rv == OK)
	{
		slib.SetSerialTimeouts(50, 0);	//SetTimeout to 50 msec
		rv = slib.SetSerialParams(38400, 8, 'N', 1, 0);
	}

	return rv;
}

long WINAPI CAN_Timeout(long n)
{
	long rv = BADPARAM;

	if (n >= 0)
	{
		timeout_val = n;
		rv = OK;
	}

	return rv;
}

void WINAPI CAN_End()
{
	slib.CloseSerial();
}

long WINAPI CAN_RxMsg(unsigned char *rx_typep, long *rx_idp, short *rx_lengthp, long *rx_flagsp, unsigned char *rx_datap)
{
	unsigned char start, id1, id2;
	int i;

	if (rx_idp == NULL || rx_lengthp == NULL || rx_flagsp == NULL || rx_datap == NULL)
		return BADPARAM;

	for (i = 0; i < timeout_val; )
	{
		if ( slib.ReadSerial(&start, 1) == 1 )
		{
			//if (start == 'C') printf("\nC>"); else printf("%x,",(int)start);
			if (start == 'C' || start == 'E')
				break;
			/**
			else
			{
				printf("%0x,",(int)start);
				if (i++ > 20)
				{
					printf("\n");
					i=0;
				}
			}
			**/
		}
		else
			i++;
	}

	if (i == timeout_val)
	{
		return IICERR_TIMEOUT;
	}
	else
	{
		*rx_typep = start;

		if (*rx_typep == 'C')
		{
			//gettimeofday(&(rx->timestamp), &tz);
			slib.ReadSerial(&id1, 1);
			slib.ReadSerial(&id2, 1);
			*rx_idp = ((long)id1 << 3) + (id2 >> 5);
			*rx_lengthp = (id2 & 0x0f);
			*rx_flagsp = 0;
			if ( (id2 >> 4) & 1 )
			{
				*rx_flagsp |= MSG_RTR;
				//RTR msg have NO data bytes
			}
			else
			{
				/* read Message */
				slib.ReadSerial(rx_datap, *rx_lengthp);
			//	rx->data[rx->length] = 0;
			}
		}
		else
		{
			*rx_lengthp = 4;
			slib.ReadSerial(rx_datap, *rx_lengthp);
		}

		return OK;
	}
}

long WINAPI CAN_TxMsg(unsigned char tx_typep, long tx_id, short tx_length, long tx_flags, unsigned char *tx_datap)
{
	//extern int errno;
	long rv;
	int n;
	unsigned char buf[20];
	unsigned char *ptr = buf;

	if (tx_datap == NULL || tx_length > 8 || tx_typep != 'C')
		return BADPARAM;

	*ptr++ = tx_typep;
	*ptr++ = (unsigned char)(tx_id >> 3);
	rv = ((tx_id & 7) << 5) + (tx_length & 0x0f);
	if (tx_flags & MSG_RTR)
		rv |= 0x10;
	*ptr++ = (unsigned char)rv;
//	if ( !(tx_flags & MSG_RTR) )
	{	//RTR msg have NO data bytes
		memcpy(ptr,tx_datap,tx_length);
		ptr += tx_length;
	}
	*ptr++ = 0xff;

	n = ptr - buf;
	rv = slib.WriteSerial(buf, n);
	if (rv == n)
		return OK;
	else
		return rv;
}

// ID
// X X X X X             function code
//           X X X X X X node select (0 invalid)
long WINAPI CAN_Address2ID(long *idp, short node, short function)
{
	if (idp == NULL || node < 0 || node > 63 || function < 0 || function > 31)
		return BADPARAM;

	*idp = ((function & 0x1f) << 6) | (node & 0x3f);

	return OK;
}

long WINAPI CAN_ID2Address(long id, short *nodep, short *functionp)
{
	if (nodep == NULL || functionp == NULL)
		return BADPARAM;

	*nodep = id & 0x3f;
	*functionp = (id >> 6) & 0x1f;

	return OK;
}
