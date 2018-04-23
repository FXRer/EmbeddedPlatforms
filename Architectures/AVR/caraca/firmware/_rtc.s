/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2002 by Claudio Lanconelli
 * e-mail: lancos@libero.it
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

#define     PCF8583ADDR  0xA2        /* RTC I2CBus address */

#define     PCF8583ADDR_W    (PCF8583ADDR & 0xFE)
#define     PCF8583ADDR_R    (PCF8583ADDR | 0x01)

/*========================
 * RTC ROUTINES
 *========================*/

/********
 * val     = ReadPCFByte(addr)
 * i2cdata               Arg1
 *
 * Returns the value in i2cdata
 *
 * READ A SINGLE-BYTE LOCATION
 * Start + Address(W) + Command + Start + Address(R) + Memory Address byte + Stop
 ********/
        seg removable flash.code.ReadPCFByte
ReadPCFByte:
            ldi     i2cadr,PCF8583ADDR_W        ; Set device address and write
            rcall   i2c_start                   ; Send start condition and address
            brcs    rconf_end

            mov     i2cdata,Arg1
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    rconf_end
            
            ldi     i2cadr,PCF8583ADDR_R        ; Set device address and read
            rcall   i2c_rep_start               ; Send repeated start condition and address
            brcs    rconf_end

            set                                 ; Set no acknowledge (read is followed by a stop condition)
            rcall   i2c_do_transfer             ; Execute transfer (read)
            rcall   i2c_stop                    ; Send stop condition - releases bus
rconf_end:
            ret


/********
 * WritePCFByte( addr, val )
 *               Arg1  Arg2
 *
 * Carry set if error
 *
 * WRITE A SINGLE-BYTE LOCATION
 * Start + Address(W) + Memory address byte + Data byte + Stop
 ********/
        seg removable flash.code.WritePCFByte
WritePCFByte:
            ldi     i2cadr,PCF8583ADDR_W        ; Set device address and write
            rcall   i2c_start                   ; Send start condition and address
            brcs    wrcfg_end

            mov     i2cdata,Arg1                ; Memory address byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    wrcfg_end

            mov     i2cdata,Arg2                ; Data byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    wrcfg_end

            rcall   i2c_stop                    ; Send stop condition - releases bus
wrcfg_end:
            ret


/*=================================
 * THREAD 3  RealTimeClock
 *=================================*/
        seg removable flash.code.Thread3
public Thr3_entry:
            /* put initialization code here */

            /* Wait for CanBus thread */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait

            /* Init I2CBus AND RTC (PCF8583) */
            rcall   i2c_init
            cbr     UserBits,(1<<RtcLockBit)

            /* Initialize PCF8583 */
            /* ... */

thr_loop:   ldi     Arg1,(1<<RtcReqSig)
            rcall   Wait

            /* Process the request: test the RtcOpBit to know if we have to Read or Write */
            sbrc    UserBits,RtcOpBit
            rjmp    rtc_readop
rtc_writeop:

            rjmp    thr_loop

rtc_readop:
            rjmp    thr_loop

