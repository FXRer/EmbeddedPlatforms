/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- RealTimeClock routines ---*

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

#include "config.h"
#include "macro.h"
#include "vars.h"

#include "kernel.e"
#include "i2cbus.e"
#include "canbus.e"
#include "eeprom.e"


/*************************
 * PCF8583 RELATED
 *************************/

#define     PCF8583ADDR  0xA0        /* RTC I2CBus address */

#define     CMD_STARTCONVERSION 0xEE
#define     CMD_STOPCONVERSION  0x22

#define     CMD_ACCESSCONFIG    0xAC
#define     CMD_ACCESS_TH       0xA1
#define     CMD_ACCESS_TL       0xA2

#define     CMD_READTEMP        0xAA
#define     CMD_READCOUNTER     0xA8
#define     CMD_READSLOPE       0xA9

/** Command structure
 * STANDALONE COMMAND (CMD_STARTCONVERSION, CMD_STOPCONVERSION)
 * Start + Address(W) + Command + Stop
 *
 * WRITE A SINGLE-BYTE REGISTER (CONFIGURATION)
 * Start + Address(W) + Command + Data + Stop
 *
 * WRITE TO A TWO-BYTE REGISTER (TH, TL)
 * Start + Address(W) + Command + MSB Data + LSB Data + Stop
 *
 * READ FROM A SINGLE-BYTE REGISTER (CONFIGURATION, SLOPE, COUNTER)
 * Start + Address(W) + Command + Start + Address(R) + Data + Stop
 *
 * READ FROM A TWO-BYTE REGISTER (TH, TL, TEMPERATURE)
 * Start + Address(W) + Command + Start + Address(R) + MSB Data + LSB Data + Stop
 */


/*========================
 * RTC ROUTINES
 *========================*/

/*=================================
 * THREAD 3  RealTimeClock
 *=================================*/
        seg removable flash.code.Thread3
public Thr3_entry:
            /* put initialization code here */

            /* Init I2CBus AND RTC (PCF8583) */
            ;rcall   i2c_init

thr_loop:   ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            mov     Reg1,Res1

            rjmp    thr_loop

