/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- DS1621 routines ---*

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
 * DS1621 RELATED
 *************************/
#define     DS1621ADDR  0x90        /* First temperature I2CBus address */

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

/** Configuration register bits **/
#define DONE_BIT    7     /* Conversion done bit (0 = conversion in progress) */
#define THF_BIT     6     /* Temperature high flag */
#define TLF_BIT     5     /* Temperature low flag */
#define NVB_BIT     4     /* EEPROM busy flag (1 = write in progress) */
#define POL_BIT     1     /* Output polarity (1 = active high) */
#define ONESHOT_BIT 0     /* One shot mode (0 = free running) */


/*========================
 * DS1621 ROUTINES
 *========================*/

/* Wait for EEPROM write finish */
#define WaitNVB\
loop@:      rcall   ReadConfiguration\
            brcs    end@\
            sbrc    i2cdata,NVB_BIT\
            rjmp    loop@\
end@:

/* Wait for Conversion finish */
#define WaitConversion\
loop@:      rcall   ReadConfiguration\
            brcs    end@\
            sbrs    i2cdata,DONE_BIT\
            rjmp    loop@\
end@:

/******
 * StopConversion(addr)
 *                Arg1
 *
 * STANDALONE COMMAND
 * Start + Address(W) + Command + Stop
 ******/
        seg removable flash.code.StopConversion
StopConversion:
            mov     i2cadr,Arg1
            cbr     i2cadr,(1<<b_dir)           ; Op: Write

            rcall   i2c_start                      ; Send start condition and address
            brcs    stconv_end

            ldi     i2cdata,CMD_STOPCONVERSION     ; Write command byte
            rcall   i2c_do_transfer                ; Execute transfer
            brcs    stconv_end

            rcall   i2c_stop
stconv_end:
            ret


/********
 * val     = ReadConfiguration(addr)
 * i2cdata                     Arg1
 *
 * Returns the value in i2cdata
 *
 * READ FROM A SINGLE-BYTE REGISTER
 * Start + Address(W) + Command + Start + Address(R) + Data + Stop
 ********/
        seg removable flash.code.ReadConfiguration
ReadConfiguration:
            mov     i2cadr,Arg1                 ; Set device address and write
            cbr     i2cadr,(1<<b_dir)           ; Op: Write

            rcall   i2c_start                   ; Send start condition and address
            brcs    rconf_end

            ldi     i2cdata,CMD_ACCESSCONFIG    ; Read status/configuration register
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    rconf_end
            
            sbr     i2cadr,(1<<b_dir)           ; Set device address and read
            rcall   i2c_rep_start               ; Send repeated start condition and address
            brcs    rconf_end

            set                                 ; Set no acknowledge (read is followed by a stop condition)
            rcall   i2c_do_transfer             ; Execute transfer (read)
            rcall   i2c_stop                    ; Send stop condition - releases bus
rconf_end:
            ret


/********
 * val      = Read2Register(addr, cmd)
 * Arg1:Arg2                Arg1  Arg2
 *
 * READ FROM A TWO-BYTE REGISTER (TH, TL, TEMPERATURE)
 * Start + Address(W) + Command + STart + Address(R) + MSB Data + LSB Data + Stop
 ********/
        seg removable flash.code.Read2Register
Read2Register:
            mov     i2cadr,Arg1                 ; Set device address and write
            cbr     i2cadr,(1<<b_dir)           ; Op: Write

            rcall   i2c_start                   ; Send start condition and address
            brcs    r2reg_end

            mov     i2cdata,Arg2                ; Write command byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    r2reg_end

            sbr     i2cadr,(1<<b_dir)           ; Set device address and read
            rcall   i2c_rep_start               ; Send repeated start condition and address
            brcs    r2reg_end

            clt
            rcall   i2c_do_transfer             ; Execute transfer (read)
            mov     Arg1,i2cdata                ; MS data Byte

            set                                 ; Set no acknowledge (read is followed by a stop condition)
            rcall   i2c_do_transfer             ; Execute transfer (read)
            mov     Arg2,i2cdata                ; LS data Byte

            rcall   i2c_stop                    ; Send stop condition - releases bus
r2reg_end:
            ret


/********
 * Write2Register(addr, cmd, val)
 *                Arg1  Arg2 Arg3:Arg4
 *
 * WRITE A TWO-BYTE REGISTER
 * Start + Address(W) + Command + MSB Data + LSB Data + Stop
 ********/
        seg removable flash.code.Write2Register
Write2Register:
            mov     i2cadr,Arg1                 ; Set device address and write
            cbr     i2cadr,(1<<b_dir)           ; Op: Write

            rcall   i2c_start                   ; Send start condition and address
            brcs    w2reg_end

            mov     i2cdata,Arg2                ; Write COMMAND
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    w2reg_end

            mov     i2cdata,Arg3                ; Write MS data Byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    w2reg_end

            mov     i2cdata,Arg4                ; Write LS data Byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    w2reg_end

            rcall   i2c_stop                    ; Send stop condition - releases bus
w2reg_end:
            ret

/********
 * WriteConfiguration( addr, val )
 *                     Arg1  Arg2
 *
 * Carry set if error
 *
 * WRITE A SINGLE-BYTE REGISTER
 * Start + Address(W) + Command + Data + Stop
 ********/
        seg removable flash.code.WriteConfiguration
WriteConfiguration:
            mov     i2cadr,Arg1
            cbr     i2cadr,(1<<b_dir)           ; Op: Write

            rcall   i2c_start                   ; Send start condition and address
            brcs    wrcfg_end

            ldi     i2cdata,CMD_ACCESSCONFIG    ; Write status/configuration register
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    wrcfg_end

            mov     i2cdata,Arg2                ; Data Byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    wrcfg_end

            rcall   i2c_stop                    ; Send stop condition - releases bus
wrcfg_end:
            ret

/******
 * StartConversion(addr)
 *                 Arg1
 *
 * STANDALONE COMMAND
 * Start + Address(W) + Command + Stop
 ******/
        seg removable flash.code.StartConversion
StartConversion:
            mov     i2cadr,Arg1                 ; Set device address and write
            cbr     i2cadr,(1<<b_dir)

            rcall   i2c_start                   ; Send start condition and address
            brcs    sconv_end

            ldi     i2cdata,CMD_STARTCONVERSION ; Write command byte
            rcall   i2c_do_transfer             ; Execute transfer
            brcs    sconv_end

            rcall   i2c_stop
sconv_end:
            ret


/*=================================
 * THREAD 4  Thermometer/Thermostat
 *=================================*/
        seg removable flash.code.Thread4
public Thr4_entry:
            /* put initialization code here */

#if ENABLE_THERMOSTAT==1
         /**   cbi     TOUT_DIR_PORT,TOUTP     ; input  ** input by default after reset **/
            sbi     TOUT_OUT_PORT,TOUTP     ; pullup
#endif

            /* Wait for CanBus thread */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait

            /* Init I2CBus AND digital thermometer (DS1621) */
            rcall   i2c_init
            cbr     UserBits,(1<<ThermoLockBit)

            /* Set active high output polarity and free running mode */
            /* ldi     Arg2,(1<<POL_BIT)|(0<<ONESHOT_BIT)  ** not needed */

            /* Initialize all DS1621s */
            ldi     Arg1,DS1621ADDR
init_loop:
            /* rcall   WriteConfiguration ** not needed */
            rcall   StartConversion
            inc     Arg1
            inc     Arg1
            cpi     Arg1,DS1621ADDR+(8*2)
            brlo    init_loop

/* Note we can receive only one request every Wait because
 * there is only one ThermoSelect variable.
 */
            /* Main loop */
thr_loop:   ldi     Arg1,(1<<ThermoReqSig)
            rcall   Wait

            /* Temperature sensor select (0 - 7) up to 8 sensors */
            mov     Arg1,ThermoSelect
            andi    Arg1,7
            lsl     Arg1
            addi(Arg1,DS1621ADDR)   /* Arg1 = DS1621 address */

            mov     Arg2,ThermoSelect
            andi    Arg2,0xF0       /* test for request type */
            breq    tempreq_test

            cpi     Arg2,0x10
            breq    getthr_test

            /* Set thermostat thresholds */
setthr_test:
#if ENABLE_THERMOSTAT==1
            WaitNVB

            /* Write TL with TempRegL:0 */
            ldi     Arg2,CMD_ACCESS_TL
            mov     Arg3,ThermoTL
            clr     Arg4
            rcall   Write2Register
            brcs    getthr_test

            WaitNVB

            /* Write TH with TempRegH:0 */
            ldi     Arg2,CMD_ACCESS_TH
            mov     Arg3,ThermoTH
            clr     Arg4
            rcall   Write2Register
#endif            
            cbr     UserBits,(1<<ThermoLockBit)
            rjmp    thr_loop

            /* Get current temperature */
tempreq_test:
            cbr     UserBits,(1<<ThermoLockBit)

            ldi     Arg2,CMD_READTEMP
            rcall   Read2Register
            brcs    tempreq_end
            mov     Arg3,ThermoSelect   ;4 LSB select sensor; 4 MSB 0 for temperature, 1 for thermostat thresholds
            mov     Arg4,Arg1   ;Temp H
            mov     Reg1,Arg2   ;Temp L

            ldi     Arg1,e2_SendTherm
            rcall   CanTxFrame3Queue
tempreq_end:
            rjmp    thr_loop

            /* Get thermostat thresholds */
getthr_test:
            cbr     UserBits,(1<<ThermoLockBit)
#if ENABLE_THERMOSTAT==1
            WaitNVB

            mov     Reg1,Arg1       /* save DS1621 address */

            /* Read TL in Arg4 */
            ldi     Arg2,CMD_ACCESS_TL
            rcall   Read2Register
            brcs    end_test
            mov     Arg4,Res1

            /* Read TH in Reg1 */
            mov     Arg1,Reg1
            ldi     Arg2,CMD_ACCESS_TH
            rcall   Read2Register
            brcs    end_test
            mov     Reg1,Res1

            mov     Arg3,ThermoSelect   ;4 LSB select sensor; 4 MSB 0 for temperature, 1 for thermostat thresholds

            ldi     Arg1,e2_SendTherm
            rcall   CanTxFrame3Queue
#endif
end_test:
            rjmp    thr_loop
