/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- EEProm Exports ---*

 This program is free software; you can redistribute it and/or           //
 modify it under the terms of the GNU  General Public License            //
 as published by the Free Software Foundation; either version2 of        //
 the License, or (at your option) any later version.                     //
                                                                         //
 This program is distributed in the hope that it will be useful,         //
 but WITHOUT ANY WARRANTY; without even the implied warranty of          //
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
 General Public License for more details.                                //
                                                                         //
 You should have received a copy of the GNU  General Public License      //
 along with this program (see COPYING);     if not, write to the         //
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. //
***************************************************************************/

#ifndef _EEPROM_E
#define _EEPROM_E

        extern  EEReadByte
        extern  EEReadWord
        extern  EEWriteByte
        extern  EEWriteWord

        extern  e2_InputMask_a:
        extern  e2_InputMask_b:
        extern  e2_InputMask_c:
        extern  e2_NodeAddress:

        extern  e2_EventTable:
        extern  e2_KeyChange:
        extern  e2_RC5code:
        extern  e2_ThermChange:
        extern  e2_SendTherm:
        extern  e2_SendInput:
        extern  e2_HelloMsg:
        extern  e2_EndOfEventTable:

        extern e2_AssociationTable:

        extern  e2_RC5Table:
        extern  e2_EndOfRC5Table:

#endif
