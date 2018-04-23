/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- New I2CBus routines ---*

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

#include "i2cbus.h"
#include "kernel.e"

#define     i2c_hp_delay    Yield
#define     i2c_qp_delay    Yield

#define SetSCLSDA\
            cbi     I2C_DIR_PORT,SDAP\
            cbi     I2C_DIR_PORT,SCLP

#define	SetSDA\
            cbi     I2C_DIR_PORT,SDAP

#define	ClearSDA\
            sbi     I2C_DIR_PORT,SDAP
			
#define	SetSCL\
            cbi     I2C_DIR_PORT,SCLP

#define	ClearSCL\
            sbi     I2C_DIR_PORT,SCLP

#define	BitSDA\
            brts    loc1@\
            ClearSDA\
            rjmp    loc2@\
loc1@:      SetSDA\
loc2@:


        seg removable flash.code.I2CBus

/******************************
 * SendBit(val)
 *        T flag
 *
 * C flag set if error occured
 *
 * Send a Bit
 *
 * USED regs: none
 */
SendBit:
            BitSDA
            rcall   i2c_qp_delay
            SetSCL
            rcall   i2c_qp_delay
            cpnePortBit(I2C_IN_PORT,SDAP,sbit_ok)
            sec
            ret
sbit_ok:    rcall   i2c_qp_delay
            ClearSCL
            rcall   i2c_qp_delay
            clc
            ret

/******************************
 *  bit = RecBit()
 * T flag
 *
 * Receive a Bit
 *
 * USED regs: none
 */
RecBit:
            SetSDA
            rcall   i2c_qp_delay
            SetSCL
            rcall   i2c_qp_delay
            clt
            sbic    I2C_IN_PORT,SDAP
            set
            rcall   i2c_qp_delay
            ClearSCL
            rcall   i2c_qp_delay
            ret


;***************************************************************************
;*
;* FUNCTION
;*    i2c_start
;*
;* DESCRIPTION
;*    Generates start condition and sends slave address.
;*
;* USAGE
;*    i2cadr - Contains the slave address and transfer direction.
;*
;* RETURN
;*    Carry flag - Cleared if a slave responds to the address.
;*
;* NOTE
;*    IMPORTANT! : This funtion must be directly followed by SendByte
;*
;***************************************************************************
public i2c_rep_start:
public i2c_start:
            SetSCLSDA
            rcall   i2c_hp_delay
            ClearSDA
            rcall   i2c_hp_delay
            ClearSCL
            rcall   i2c_qp_delay
            /* ret omitted, after i2c_start we always call SendByte */

            mov     i2cdata,i2cadr

/******************************
 * SendByte(val)
 *        i2cdata
 *
 * C flag set if error occured or No ack from slave
 *
 * Send a Byte and receive ack
 *
 * USED regs: i2cdata
 */
SendByte:
            sec                         ; set carry flag
            rol     i2cdata             ; shift in carry and out bit one
sby_loop:   clt
            brcc    sby_1
            set
sby_1:      rcall   SendBit
            brcs    sby_end
            lsl     i2cdata
            brne    sby_loop
sby_getack:
            rcall   RecBit
            brts    sby_noack
            clc
sby_end:    ret
sby_noack:  sec
            ret


/***************************************************************************
 *
 * FUNCTION
 *    i2c_do_transfer
 *
 * DESCRIPTION
 *    Executes a transfer on bus. This is only a combination of SendByte
 *    and RecByte for convenience.
 *
 * USAGE
 *    i2cadr - Must have the same direction as when i2c_start was called.
 *    see RecByte and SendByte for more information.
 *
 *
 * NOTE
 *    IMPORTANT! : This funtion must be directly followed by RecByte
 *
 */
public i2c_do_transfer:
            sbrs    i2cadr,b_dir        ; if dir = write
            rjmp    SendByte			;    goto write data


/******************************
 *    val = RecByte(ack)
 * i2cdata           T flag
 *
 * Receive a Byte and send ack
 *
 * USED regs: bit 7 of i2cadr, i2cdata
 */
RecByte:    bld     i2cadr,7            ; store ack
            ldi     i2cdata,0x01
rby_loop:   rcall   RecBit
            lsl     i2cdata
            brtc    rby_0
            sbr     i2cdata,1
rby_0:      brcc    rby_loop
rby_putack:
            bst     i2cadr,7
            rcall   SendBit
            ret


;***************************************************************************
;*
;* FUNCTION
;*    i2c_stop
;*
;* DESCRIPTION
;*    Assert stop condition.
;*
;* USAGE
;*    No parameters.
;*
;* RETURN
;*    None.
;*
;***************************************************************************

public i2c_stop:
            ClearSDA
            rcall   i2c_hp_delay
            SetSCL
            rcall   i2c_hp_delay
            SetSDA
            rcall   i2c_hp_delay
            ret
			

;***************************************************************************
;*
;* FUNCTION
;*    i2c_init
;*
;* DESCRIPTION
;*    Initialization of the I2C bus interface.
;*
;* USAGE
;*    Call this function once to initialize the I2C bus. No parameters
;*    are required.
;*
;* RETURN
;*    None
;*
;***************************************************************************

public i2c_init:
            cbi     I2C_DIR_PORT,SCLP       ;direction = Input (Hi-Z)
            cbi     I2C_DIR_PORT,SDAP

            cbi     I2C_OUT_PORT,SCLP       ;no pullup resistor
            cbi     I2C_OUT_PORT,SDAP
            ret
      