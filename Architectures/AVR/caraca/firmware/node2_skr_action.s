/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- Event and Action Table and Action routines ---*
 $Id: node2_skr_action.s,v 1.9 2000/12/07 11:09:59 skr Exp $

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
#include "vars.h"
#include "macro.h"

#include "kernel.e"
#include "canbus.e"
#include "eeprom.e"

#define DEFNODE_ADDR    0x3f   /* default node address for unconfigured nodes */

/* This macro transform the human readable 1,2,3,4 notation to a binary coded ID */
#define BUILD_MSGID(fcode, naddr, rtr, len) ( (((fcode)&0x1f)<<11) | (((naddr)&0x3f)<<5) | (((rtr)&1)<<4) | (((len)&0xf)<<0) )

/* Event:  input state change
 * Action: ouput state change
 *
 * The eeprom store both Action and Event table.
 * When we receive a msgID from the CAN bus we check in the Action table to
 * look if we have to do something. Every entry store up to 2 msgID, unused
 * entries are left 0. The first ID must be unique for the node, while the
 * other may be common to other nodes. Every ID is in the form 1.2.3.4, then
 * the macro BUILD_MSGID will build the binary ID code from these four field:
 * 1) function code (1-31)
 * 2) node address (1-63)
 * 3) RTR bit (always 0)
 * 4) len of the message in bytes (0-4), this must be 0 only in Action table
 *
 * For example the entry of Temperature request may be:
 * Node A
 * - 10,2
 * - 10,60
 * Node B
 * - 10,3
 * - 10,60
 * You may reserve as many address as you want for multicast addresses. In this
 * example we reserve address 60 for a multicast address for both Node A
 * and Node B. If we transmit on the bus the message 10,2,0,0 reply only Node A,
 * but if we tranmit the message 10,60,0,0 reply both Node A and Node B.
 * Multicast may be useful to address all the NODEs in a floor, or all the
 * NODEs in room. Broadcast address (0) is used to address all NODEs
 * on the network.
 *
 * On event raise we look at the Event table and transmit its msgID
 * on the CAN bus.
 *
 */

/**========================**
 ** EEprom structure
 **========================**/
        seg    eeprom
e2_pad1:    ds.b    1   /* first byte reserved for the AVR eeprom brown-out bug */

public e2_SoftVersion:
            dc.b    SOFTWARE_VERSION
public e2_SoftRevision:
            dc.b    SOFTWARE_REVISION

public e2_InputMask_a:            /* key port 1 mask */
            dc.b    KEY_INIT_MASK
public e2_InputPullUpMask_a:      /* key port 1 mask */
            dc.b    KEY_INIT_MASK
             
public e2_InputMask_b:      /* key port 2 mask (not used) */
            dc.b    0
public e2_InputMask_c:      /* thermostat output port mask */
            dc.b    0

public e2_NodeAddress:
            ;The address is shifted left by 1 to save some instructions
            dc.b    (DEFNODE_ADDR << 1)    ;primary unique node address
            dc.b    0       ;secondary multicast address, 0 if unused (broadcast address)

   /* We reserve 2 bytes for every ID, the byte order is Little Endian
    * (LSB at low address)
    */

   /* Event Table (1 msgID for every event) */
public e2_EventTable:
public e2_KeyChange:
            dc.w    BUILD_MSGID(14,DEFNODE_ADDR,0,2)
public e2_RC5code:
            dc.w    BUILD_MSGID(15,DEFNODE_ADDR,0,2)
public e2_ThermChange:
            dc.w    BUILD_MSGID(16,DEFNODE_ADDR,0,2)

/* events on request */
public e2_SendOutput:
            dc.w    BUILD_MSGID(17,DEFNODE_ADDR,0,1)
public e2_SendInput:
            dc.w    BUILD_MSGID(19,DEFNODE_ADDR,0,2)
public e2_SendTherm:
            dc.w    BUILD_MSGID(20,DEFNODE_ADDR,0,3)
public e2_SendEEprom:
            dc.w    BUILD_MSGID(27,DEFNODE_ADDR,0,3)
public e2_SendDebug:
            dc.w    BUILD_MSGID(28,DEFNODE_ADDR,0,3)
public e2_HelloMsg:
            dc.w    BUILD_MSGID(30,DEFNODE_ADDR,0,3)
public e2_EndOfEventTable:


    /* RC5 Table: remote control unit RC5 key codes
     * this table changes with different remote units */
public e2_RC5Table:
            dc.w    0x07D9
            dc.w    0x07F3
            dc.w    0x07F2
            dc.w    0x07F0
            dc.w    0x07DE
            dc.w    0x07DF
            dc.w    0x07EE
            dc.w    0x07EF
public e2_EndOfRC5Table:

/* Brightness table for Dimmer, we use it to translate Brightness values into
   timing constants. Timer1 gets loaded every zero crossing with that to fire the triac
   on overflow.
   
   TODO: make a better lookup algorithm to have a 0-100 range, which is linear
*/

#define D1 0x44
#define D2 D1+16*SD1
#define D3 D2+16*SD2
#define D4 D3+16*SD3
#define D5 D4+16*SD4
#define D6 D5+16*SD5
#define D7 D6+16*SD6
#define SD1 1
#define SD2 1
#define SD3 1
#define SD4 2
#define SD5 3
#define SD6 0
#define SD7 0

#define B1 0x3
#define B2 B1+16*SB1
#define B3 B2+16*SB2
#define B4 B3+16*SB3
#define B5 B4+16*SB4
#define B6 B5+16*SB5
#define B7 B6+16*SB6
#define SB1 2
#define SB2 3
#define SB3 3
#define SB4 5
#define SB5 0
#define SB6 0
#define SB7 0

public e2_Bright_Tbl:	 dc.b  D1,SD1,D2,SD2,D3,SD3,D4,SD4,D5,SD5
                 dc.b  B1,SB1,B2,SB2,B3,SB3,B4,SB4
 


/*--------------------------*
 * Action Table
 *--------------------------*
 * There's a call for every function code.
 * Entry for events just execute a RET.
 *--------------------------*/
        seg removable flash.code.ActionTable
public ActionTable:
    /*** 0 - reserved ***/
            dc.W    dummyFunction >> 1
    /*** 1 - Set Outputs ***/
            dc.W    canSetOutput >> 1
    /*** 2 - Set Dimmer ***/
            dc.W    canSetDimmer >> 1
    /*** 3 - Set Input mask ***/
            dc.W    canSetInputMask >> 1
    /*** 4 - Set thermometer ***/
            dc.W    canSetTempThr >> 1
    /*** 5 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 6 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 7 - Request output ***/
            dc.W    canSendOutput >> 1
    /*** 8 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 9 - Request Input ***/
            dc.W    canReadInput >> 1
    /*** 10 - Request thermometer ***/
            dc.W    canReadTherm >> 1
    /*** 11 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 12 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 13 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 14 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 15 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 16 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 17 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 18 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 19 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 20 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 21 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 22 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 23 - Not Used ***/
            dc.W    dummyFunction >> 1
    /*** 24 - Not Used ***/
            dc.W    dummyFunction >> 1
    /*** 25 - Write eeprom ***/
            dc.W    canWriteEEPROM >> 1
    /*** 26 - Request eeprom ***/
            dc.W    canSendEEPROM >> 1
    /*** 27 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 28 - Send Debug Message ***/
            dc.W    debugFunction >> 1
    /*** 29 - Set Address ***/
            dc.W    canSetAddress >> 1
    /*** 30 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 31 - Request Hello message ***/
            dc.W    canSendSerial >> 1
/*----------------------------------------------*/

        seg removable flash.code.ActionRoutines

/* Note: All following function are called by EDISPATCHER thread.
 *       they find the pointer to the RxBuffer in Arg4 (Y)
 */

dummyFunction:
            ret


debugFunction:
            ldi     Arg1,e2_SendDebug
            rjmp    CanTxFrame3Queue


/* Action: Send hello message
 *
 * Used regs: Arg1(ZH),Arg2(ZL),Arg3,Arg4,Reg1,Reg2
 */
public canSendSerial:
            ;BigEndian format
            ldi     ZL,low(SerialNumber)   /* ZL = Arg2 */
            ldi     ZH,high(SerialNumber)  /* ZH = Arg1 */
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg3,Reg2           ;MSB
            adiw    ZL,1                ;ZL = Arg2 (r30)
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg4,Reg2           ;LSB
            clr     Reg1                ;device type 0 --> NODE

            ldi     Arg1,e2_HelloMsg
            rjmp    CanTxFrame3Queue
            ;rcall  CanTxFrame3Queue
            ;ret


/* Action: Set address
 *
 * Used regs: Arg1(ZH),Arg2(ZL),Arg3,Arg4(YL),Reg1,Reg2
 */
canSetAddress:
            ;First verify serial number (bigendian format)
            ldi     ZL,low(SerialNumber)   /* ZL = Arg2 */
            ldi     ZH,high(SerialNumber)  /* ZH = Arg1 */
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            ld      Arg3,Y+
            cp      Arg3,Reg2           ;MSB
            brne    csa_end

            adiw    ZL,1                ;ZL = Arg2 (r30)
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            ld      Arg3,Y+
            cp      Arg3,Reg2           ;LSB
            brne    csa_end

            ld      Arg2,Y+             ;primary address
            andi    Arg2,0x3F
            breq    csa_end             ;avoid address 0 (broadcast)
            lsl     Arg2                ;keep node address shifted left by 1

            ld      Arg3,Y+             ;secondary address
            lsl     Arg3                ;keep node address shifted left by 1

            mov     Arg4,Arg2
            ldi     Arg1,e2_NodeAddress
            rcall   EEWriteWord

            swap    Arg4                ;adjust data
            mov     Arg3,Arg4

            /* Loop to update EventTable */
            ldi     Reg1,e2_EventTable
csa_etloop:
            mov     Arg1,Reg1
            rcall   EEReadWord          ;read EventTable entry (word)
            cbr     Arg1,0x07           ;clear current address
            cbr     Arg2,0xE0

            andi    Arg3,0x07           ;mask node address
            andi    Arg4,0xE0

            or      Arg3,Arg1           ;modify current address
            or      Arg4,Arg2

            mov     Arg1,Reg1
            rcall   EEWriteWord         ;write EventTable entry (word)

            addi(Reg1,2)                ;point to next entry
            cpi     Reg1,e2_EndOfEventTable
            brne    csa_etloop

            rjmp    canSendSerial       ;send Hello msg with new address before
csa_end:                                ; to return
            ret



/* Action: Set Dimmer 
   get 1 Byte as Brightness value - use a lookup table to get 2 Byte time constant
   for Counter 1 
   local vars: Reg1      - copy of TBright to use ldi and cpi
               Arg1,Arg2 - to read brigtness table
               Arg3 

*/ 

canSetDimmer:
            ld      TBright,Y+     ;YL = Arg4 (r28)
public SetDimmer:
            tst     TBright
            breq    LightOff
	    mov     Reg1,TBright       /* copy to use ldi and cpi */
            cpi     Reg1,MAX_BRIGHTNESS
            skipcs
            ldi     Reg1,MAX_BRIGHTNESS
            mov     Arg2,Reg1
            swap    Arg2               /* get pointer to table entry */
            andi    Arg2,0x0f
            lsl     Arg2
            ldi     Arg1,e2_Bright_Tbl
            add     Arg1,Arg2
            rcall   EEReadWord         /* get Counter Offset(Res2)/Step(Res1) */
            mov     Arg3,Reg1
            andi    Arg3,0x0f
step_add:   add     Res2,Arg3
            dec     Res1               /* steps */
            brne    step_add
                                       /* Counter value in Res2 */
            mov     Reg3,TriacBits            
            cli                              
            sbr     TriacBits,BV(TBrightBit)
            cpi     Reg1,0x50
            skipcc  
            cbr     TriacBits, BV(TBrightBit)
            eor     Reg3,TriacBits          /* test for change of Brightness */
            sbrc    Reg3,TBrightBit
            sbr     TriacBits, BV(TBrightChange)
            mov     TriZCnew,Res2           /* new Timer Constant */
            sbrs    TriacBits,TOn
            sbr     TriacBits,BV(TSwitchOn)
            sei
            ret
LightOff:   cbr     TriacBits,BV(TOn)|BV(TSynced)
            ret


/* Action: Set Temperature Thermostats
 *
 * Used regs: Arg1,Arg2,Arg3,Arg4(YL)
 */
canSetTempThr:
#if ENABLE_THERMOSTAT==1
            sbrc    UserBits,ThermoLockBit  ;if there's another request pending discard it
            ret
            ld      ThermoSelect,Y+     ;YL = Arg4 (r28)
            cbr     ThermoSelect,0xF0
            sbr     ThermoSelect,0x20
            rjmp    crt_ok
#else
            ret
#endif

/* Action: Read Temperature */
/* Action: Get Temperature Thermostats */
canReadTherm:    /* Send the Temperature request signal to Thermometer thread */
#ifdef THERMOMETER
            sbrc    UserBits,ThermoLockBit  ;if there's another request pending discard it
            ret
            ld      ThermoSelect,Y+     ;YL = Arg4 (r28)
crt_ok:
#if ENABLE_THERMOSTAT==1
            ld      ThermoTL,Y+
            ld      ThermoTH,Y+
#endif
            sbr     UserBits,(1<<ThermoLockBit)

            ldi     Arg1,(1<<ThermoReqSig)
            ldi     Arg2,THERMOMETER
            rcall   Signal
#endif 
            ret



/* Action: Send EEPROM data
 *
 * Used regs: Arg1,Arg2,Arg3,Arg4(YL)
 */
public canSendEEPROM:
            ld      Arg1,Y+	            ;get Address from CAN Rx
            mov     Arg3,Arg1           ;send back address as 2nd byte
            rcall   EEReadWord
            mov     Arg4,Res2           ;Send MSB as first Byte
            mov     Reg1,Res1           ;Send LSB as second Byte

            ldi     Arg1,e2_SendEEprom
            rjmp    CanTxFrame3Queue
            ;rcall  CanTxFrame3Queue
            ;ret



/* Action: Write EEPROM data
 *
 * Used regs: Arg1,Arg2,Arg3,Arg4(YL)
 */
canWriteEEPROM:
            dec     YL                  ;point to RxId
            ld      Arg3,Y+             ;get Len
            andi    Arg3,0x07
            dec     Arg3                ;don't count address byte
            ld      Arg1,Y+             ;get Address
wreep_loop: ld      Arg2,Y+
            rcall   EEWriteByte
            inc     Arg1
            dec     Arg3
            brne    wreep_loop
            ret



/* Action: Read Input state
 * Action: Read Input mask
 *
 * Used regs: Arg1,Arg2,Arg3,Arg4(YL)
 */
canReadInput:       /* Function selected with the first data byte */
            ld      Arg1,Y          ; YL = Arg4 (r28)
            tst     Arg1
            brne    ri_getmask

            /* Get Input Status */

            clr     Arg3
            clr     Arg4
#ifdef  Key4P
            sbic    KEY_IN_PORT,Key4P
            sbr     Arg4,BV(3)
#endif
#ifdef  Key3P
            sbic    KEY_IN_PORT,Key3P
            sbr     Arg4,BV(2)
#endif
#ifdef  Key2P
            sbic    KEY_IN_PORT,Key2P
            sbr     Arg4,BV(1)
#endif
            sbic    KEY_IN_PORT,Key1P
            sbr     Arg4,BV(0)

#if ENABLE_THERMOSTAT==1
            sbic    TOUT_IN_PORT,TOUTP
            sbr     Arg4,BV(7)
#endif
            rjmp    ri_sendframe

ri_getmask: /* Get Input mask */
            ldi     Arg1,e2_InputMask_a
            rcall   EEReadByte

            ldi     Arg3,1
            clr     Arg4
#ifdef  Key4P
            sbrc    Arg1,Key4P
            sbr     Arg4,BV(3)
#endif
#ifdef  Key3P
            sbrc    Arg1,Key3P
            sbr     Arg4,BV(2)
#endif
#ifdef  Key2P
            sbrc    Arg1,Key2P
            sbr     Arg4,BV(1)
#endif
            sbrc    Arg1,Key1P
            sbr     Arg4,BV(0)

#if ENABLE_THERMOSTAT==1
            ldi     Arg1,e2_InputMask_c
            rcall   EEReadByte

            sbrc    Arg1,TOUTP
            sbr     Arg4,BV(7)
#endif

ri_sendframe:
            ldi     Arg1,e2_SendInput
            rjmp    CanTxFrame3Queue
            ;rcall  CanTxFrame3Queue
            ;ret


/* Action: Set Input Mask
 *
 * Used regs: Arg1,Arg2,Arg3
 */
canSetInputMask:            /* new mask  in first data byte */
            ld      Arg3,Y          ; YL = Arg4 (r28)
            clr     Arg2

            sbrc    Arg3,0
            sbr     Arg2,BV(Key1P)

            sbrc    Arg3,1
            sbr     Arg2,BV(Key2P)

            sbrc    Arg3,2
            sbr     Arg2,BV(Key3P)

            sbrc    Arg3,3
            sbr     Arg2,BV(Key4P)
            ;ldi     Arg1,e2_InputMask_a
            ldi     Arg1,e2_InputPullUpMask_a

            in      Arg1,KEY_OUT_PORT       ;pullup
            or      Arg1,Arg2               ;set new pullup mask
            out     KEY_OUT_PORT,Arg1
            rcall   EEWriteByte

#if ENABLE_THERMOSTAT==1
            clr     Arg2
            sbrc    Arg3,7
            sbr     Arg2,BV(TOUTP)

            ldi     Arg1,e2_InputMask_c
            rcall   EEWriteByte
#endif
            ret



/* Action: Set Output
 *
 * Format: OpByte + Value
 * OpByte
 *      0 = clear mask
 *      1 = set mask
 *      2 = toggle mask
 *      3 = assign
 *
 * Used regs: Arg1, Arg2, Arg3, Arg4 (YL), Reg3, Reg4
 */
canSetOutput:
            ld      Arg1,Y+         ; YL = Arg4 (r28)
            ld      Arg2,Y

public SetOutput:
            ;Expect the arguments in Arg1, Arg2 (same format of canSetOutput)
            ;May be called from user code (not by ActionTable)
	
            in      Reg3,LED_OUT_PORT
            in      Reg4,LED4_OUT_PORT

            ;;Check for operation
            cpi     Arg1,0          ;;clear
            brne    csout_test3

            ;;clear byte A
            sbrc    Arg2,0
            cbr     Reg3,(1<<Led1P)
            sbrc    Arg2,1
            cbr     Reg3,(1<<Led2P)
            sbrc    Arg2,2
            cbr     Reg3,(1<<Led3P)
            ;;clear byte B
            sbrc    Arg2,3
            cbr     Reg4,(1<<Led4P)
            sbrc    Arg2,4
            cbr     Reg4,(1<<HeatingFlag)
            ;;clear Dimmer (Light Off)
            sbrc    Arg2,5
            rcall   LightOff
            rjmp    csout_end

csout_test3:
            cpi     Arg1,2          ;;toggle
            brne    csout_test0

            clr     Arg1
            ;;toggle byte A
            sbrc    Arg2,0
            sbr     Arg1,(1<<Led1P)
            sbrc    Arg2,1
            sbr     Arg1,(1<<Led2P)
            sbrc    Arg2,2
            sbr     Arg1,(1<<Led3P)
            eor     Reg3,Arg1

            clr     Arg1
            ;;toggle byte B
            sbrc    Arg2,3
            sbr     Arg1,(1<<Led4P)
            sbrc    Arg2,4
            sbr     Arg1,(1<<HeatingFlag)
            eor     Reg4,Arg1
            clr     Arg1
            sbrs    Arg2,5
            rjmp    csout_end
            ;;toggle Dimmer
            bst     TriacBits,TOn
            brtc    toggle_on
            cbr     TriacBits,BV(TOn)|BV(TSynced) ;; LightOff
            rjmp    csout_end
toggle_on:  sbr     TriacBits,BV(TSwitchOn)
            rjmp    csout_end

csout_test0:
            cpi     Arg1,3          ;;assign
            brne    csout_test1
            cbr     Reg3,(1<<Led1P)|(1<<Led2P)|(1<<Led3P)
            cbr     Reg4,(1<<Led4P)|(1<<HeatingFlag)
            ;;for dimmer not available

csout_test1:
            ;; 1 set
            ;;set byte A
            sbrc    Arg2,0
            sbr     Reg3,(1<<Led1P)
            sbrc    Arg2,1
            sbr     Reg3,(1<<Led2P)
            sbrc    Arg2,2
            sbr     Reg3,(1<<Led3P)
            ;;set byte B
            sbrc    Arg2,3
            sbr     Reg4,(1<<Led4P)
            sbrc    Arg2,4
            sbr     Reg4,(1<<HeatingFlag)
            sbrc    Arg2,5
            sbr     TriacBits,BV(TSwitchOn)
csout_end:
            out     LED_OUT_PORT,Reg3
            out     LED4_OUT_PORT,Reg4
            ret

/* Read back the status of outputs
 *
 * Used regs: Res1
 */
ReadOutput:
            clr     Res1

            ;;set byte A
            sbic    LED_OUT_PORT,Led1P
            sbr     Res1,(1<<0)
#ifdef  Led2P
            sbic    LED_OUT_PORT,Led2P
            sbr     Res1,(1<<1)
#endif
#ifdef  Led3P
            sbic    LED_OUT_PORT,Led3P
            sbr     Res1,(1<<2)
#endif
#ifdef  Led4P
            sbic    LED4_OUT_PORT,Led4P
            sbr     Res1,(1<<3)
#endif

            ret


/* Action: Send Output
 *
 * Used regs: Arg1,Arg2,Arg3
 */
canSendOutput:
            rcall   ReadOutput
            mov     Arg3,Res1

            ldi     Arg1,e2_SendOutput
            rjmp    CanTxFrame3Queue
            ;rcall  CanTxFrame3Queue
            ;ret



