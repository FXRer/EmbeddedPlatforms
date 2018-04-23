/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- Event and Action Table and Action routines ---*

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
 * - 10,2,0,0
 * - 10,60,0,0
 * Node B
 * - 10,3,0,0
 * - 10,60,0,0
 * You may reserve as many address as you want for multicast addresses. In this
 * example we reserve address 60 for a multicast address for both Node A
 * and Node B. If we transmit on the bus the message 10,2,0,0 reply only Node A,
 * but if we tranmit the message 10,60,0,0 reply both Node A and Node B.
 * Multicast may be useful to address all the NODEs in a floor, or all the
 * NODEs on the network.
 *
 * On event raise we look at the Event table and transmit its msgID
 * on the CAN bus.
 *
 */

/**========================**
 ** EEprom structure
 **========================**/
        seg    eeprom
e2_pad1:    ds.b    1       /* first byte reserved for the AVR eeprom brown-out bug */

public e2_SoftVersion:
            dc.b    SOFTWARE_VERSION
public e2_SoftRevision:
            dc.b    SOFTWARE_REVISION

public e2_InputMask_a:      /* key port 1 mask */
            dc.b    0

public e2_NodeAddress:
            dc.b    (DEFNODE_ADDR << 1)    ;unique node address
            dc.b    0               ;multicast address, 0 if unused (broadcast address)

   /* We reserve 2 bytes for every ID, the byte order is Little Endian
    * (LSB at low address)
    */

   /* Event Table (1 msgID for every event) */
public e2_EventTable:

/* events on request */
public e2_SendOutput:
            dc.w    BUILD_MSGID(17,DEFNODE_ADDR,0,1)
public e2_SendEEprom:
            dc.w    BUILD_MSGID(27,DEFNODE_ADDR,0,3)
public e2_HelloMsg:
            dc.w    BUILD_MSGID(30,DEFNODE_ADDR,0,3)
public e2_EndOfEventTable:


/* Every entry of the association table have the following format:
 * - EVENT  msg mask  (2 bytes: ID & Len)
 * - EVENT  msg entry (2 bytes ID & Len + 4 bytes data)
 * - ACTION msg entry (2 bytes ID & Len + 4 bytes data)
 * - Delay in unit of 200msec (0 means no delay between event and action)
 *
 * Last entry is all 0.
 */
public e2_AssociationTable:
            dc.w    BUILD_MSGID(31,63,1,7)      ;event ID+Len mask

            dc.w    BUILD_MSGID(15,5,0,2)       ;event ID+Len key
            dc.b    0x00        ;data counter to test
            dc.b    0x00        ;event data 1
            dc.b    0x00        ;event data 2
            dc.b    0x00        ;event data 3

            dc.w    0          ;delay (200msec * 10 = 2 sec delay)

            ;dc.w    BUILD_MSGID(10,5,0,1)      ;action ID+Len
            dc.w    BUILD_MSGID(1,5,0,2)
            dc.b    0x02        ;action data 1
            dc.b    0x01        ;action data 2
            dc.b    0x00        ;action data 3
            dc.b    0x00        ;action data 4


            dc.w    BUILD_MSGID(31,63,1,7)      ;event ID+Len mask

            dc.w    BUILD_MSGID(15,5,0,2)       ;event ID+Len key
            dc.b    0x00        ;data counter to test
            dc.b    0x00        ;event data 1
            dc.b    0x00        ;event data 2
            dc.b    0x00        ;event data 3

            dc.w    25          ;delay (200msec * 10 = 2 sec delay)

            dc.w    BUILD_MSGID(1,5,0,2)
            dc.b    0x02        ;action data 1
            dc.b    0x01        ;action data 2
            dc.b    0x00        ;action data 3
            dc.b    0x00        ;action data 4


            ;Last entry ALL zero
            dc.w    0x0000      ;event ID+Len mask
public e2_EndOfAssociationTable:


        seg removable flash.code.StoreOutput
/* Store Output status to eeprom
 * StoreOutputStatus(val1)
 *                   Arg1
 *
 * Used Regs: Arg1,Arg2,Arg3,Arg4
 */
StoreOutputStatus:
            rcall   ReadOutput
            mov     Arg3,Res1
            clr     Arg2
            clr     Arg4
            ;...
            ret



/* Retrieve Output status from eeprom
 * val = GetOutputStatus()
 * Res1
 *
 * Used Regs: Arg1,Arg2
 */
public GetOutputStatus:
            ;...
            clr     Res1
            ret



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
            dc.W    dummyFunction >> 1
    /*** 3 - Set Input mask ***/
            dc.W    dummyFunction >> 1
    /*** 4 - Set thermometer ***/
            dc.W    dummyFunction >> 1
    /*** 5 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 6 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 7 - Request output ***/
            dc.W    canSendOutput >> 1
    /*** 8 - Not used ***/
            dc.W    dummyFunction >> 1
    /*** 9 - Request Input ***/
            dc.W    dummyFunction >> 1
    /*** 10 - Request thermometer ***/
            dc.W    dummyFunction >> 1
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
    /*** 28 - Not Used ***/
            dc.W    dummyFunction >> 1
    /*** 29 - Set Address ***/
            dc.W    canSetAddress >> 1
    /*** 30 - Event ***/
            dc.W    dummyFunction >> 1
    /*** 31 - Request Hello message ***/
            dc.W    canSendHello >> 1
/*----------------------------------------------*/

        seg removable flash.code.ActionRoutines
dummyFunction:
            ret


/* Note: All following function are called by EDISPATCHER thread.
 *       they find the pointer to the RxBuffer in Arg4 (Y)
 */


/* Action: Set address
 *
 * Used regs: Arg1(ZH),Arg2(ZL),Arg3,Arg4(YL),Reg1,Reg2
 */
canSetAddress:
            ;First verify serial number
            ldi     ZL,low(SerialNumber)   /* ZL = Arg2 */
            ldi     ZH,high(SerialNumber)  /* ZH = Arg1 */
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            ld      Arg3,Y+
            cp      Arg3,Reg2           ;low byte
            brne    csa_end

            adiw    ZL,1                ;ZL = Arg2 (r30)
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            ld      Arg3,Y+
            cp      Arg3,Reg2
            brne    csa_end

            ld      Arg2,Y+             ;primary address
            andi    Arg2,0x3F
            breq    csa_end             ;avoid address 0 (broadcast)
            lsl     Arg2                ;keep node address shifted left by 1

            ld      Arg3,Y+             ;secondary address
            lsl     Arg3

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
            cbr     Arg1,0x07
            cbr     Arg2,0xE0

            andi    Arg3,0x07
            andi    Arg4,0xE0

            or      Arg3,Arg1           ;modify the node address
            or      Arg4,Arg2

            mov     Arg1,Reg1
            rcall   EEWriteWord         ;write EventTable entry (word)

            addi(Reg1,2)
            cpi     Reg1,e2_EndOfEventTable
            brne    csa_etloop

            rjmp    canSendHello        ;send Hello msg with new address before
csa_end:                                ; to return
            ret


/* Action: Send hello message
 *
 * Used regs: Arg1(ZH),Arg2(ZL),Arg3,Arg4,Reg1,Reg2
 */
public canSendHello:
            ;BigEndian format
            ldi     ZL,low(SerialNumber)   /* ZL = Arg2 */
            ldi     ZH,high(SerialNumber)  /* ZH = Arg1 */
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg3,Reg2
            adiw    ZL,1                ;ZL = Arg2 (r30)
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg4,Reg2
            ldi     Reg1,1              ;device type 1 --> STAR

            ldi     Arg1,e2_HelloMsg
            rjmp    CanTxFrame3Queue
            ;rcall   CanTxFrame3Queue
            ;ret


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
            ld      Arg2,Y+

public SetOutput:
            in      Reg3,RELEA_OUT_PORT
            in      Reg4,RELEB_OUT_PORT

            ;;Check for operation
            cpi     Arg1,0          ;;clear
            brne    csout_test3

            ;;clear byte A
            sbrc    Arg2,0
            cbr     Reg3,(1<<ReleA1P)
#ifdef  ReleA2P
            sbrc    Arg2,1
            cbr     Reg3,(1<<ReleA2P)
#endif
#ifdef  ReleA3P
            sbrc    Arg2,2
            cbr     Reg3,(1<<ReleA3P)
#endif
            ;;clear byte B
#ifdef  ReleB5P
            sbrc    Arg2,7
            cbr     Reg4,(1<<ReleB5P)
#endif
#ifdef  ReleB4P
            sbrc    Arg2,6
            cbr     Reg4,(1<<ReleB4P)
#endif
#ifdef  ReleB3P
            sbrc    Arg2,5
            cbr     Reg4,(1<<ReleB3P)
#endif
#ifdef  ReleB2P
            sbrc    Arg2,4
            cbr     Reg4,(1<<ReleB2P)
#endif
            sbrc    Arg2,3
            cbr     Reg4,(1<<ReleB1P)
            rjmp    csout_end

csout_test3:
            cpi     Arg1,2          ;;toggle
            brne    csout_test0

            clr     Arg1
            ;;toggle byte A
            sbrc    Arg2,0
            sbr     Arg1,(1<<ReleA1P)
#ifdef  ReleA2P
            sbrc    Arg2,1
            sbr     Arg1,(1<<ReleA2P)
#endif
#ifdef  ReleA3P
            sbrc    Arg2,2
            sbr     Arg1,(1<<ReleA3P)
#endif
            eor     Reg3,Arg1

            clr     Arg1
            ;;toggle byte B
#ifdef  ReleB5P
            sbrc    Arg2,7
            sbr     Arg1,(1<<ReleB5P)
#endif
#ifdef  ReleB4P
            sbrc    Arg2,6
            sbr     Arg1,(1<<ReleB4P)
#endif
#ifdef  ReleB3P
            sbrc    Arg2,5
            sbr     Arg1,(1<<ReleB3P)
#endif
#ifdef  ReleB2P
            sbrc    Arg2,4
            sbr     Arg1,(1<<ReleB2P)
#endif
            sbrc    Arg2,3
            sbr     Arg1,(1<<ReleB1P)
            eor     Reg4,Arg1
            rjmp    csout_end

csout_test0:
            cpi     Arg1,3          ;;assign
            brne    csout_test1
            cbr     Reg3,REL_INIT_MASKA
            cbr     Reg4,REL_INIT_MASKB
csout_test1:
            ;; 1 set
            ;;set byte A
            sbrc    Arg2,0
            sbr     Reg3,(1<<ReleA1P)
#ifdef  ReleA2P
            sbrc    Arg2,1
            sbr     Reg3,(1<<ReleA2P)
#endif
#ifdef  ReleA3P
            sbrc    Arg2,2
            sbr     Reg3,(1<<ReleA3P)
#endif
            ;;set byte B
#ifdef  ReleB5P
            sbrc    Arg2,7
            sbr     Reg4,(1<<ReleB5P)
#endif
#ifdef  ReleB4P
            sbrc    Arg2,6
            sbr     Reg4,(1<<ReleB4P)
#endif
#ifdef  ReleB3P
            sbrc    Arg2,5
            sbr     Reg4,(1<<ReleB3P)
#endif
#ifdef  ReleB2P
            sbrc    Arg2,4
            sbr     Reg4,(1<<ReleB2P)
#endif
            sbrc    Arg2,3
            sbr     Reg4,(1<<ReleB1P)
csout_end:
            out     RELEA_OUT_PORT,Reg3
            out     RELEB_OUT_PORT,Reg4

            rjmp    StoreOutputStatus
            ;rcall   StoreOutputStatus
            ;ret



/* Read back the status of outputs
 *
 * Used regs: Res1
 */
ReadOutput:
            clr     Res1

            ;;set byte A
            sbic    RELEA_OUT_PORT,ReleA1P
            sbr     Res1,(1<<0)
#ifdef  ReleA2P
            sbic    RELEA_OUT_PORT,ReleA2P
            sbr     Res1,(1<<1)
#endif
#ifdef  ReleA3P
            sbic    RELEA_OUT_PORT,ReleA3P
            sbr     Res1,(1<<2)
#endif
            ;;set byte B
#ifdef  ReleB5P
            sbic    RELEB_OUT_PORT,ReleB5P
            sbr     Res1,(1<<7)
#endif
#ifdef  ReleB4P
            sbic    RELEB_OUT_PORT,ReleB4P
            sbr     Res1,(1<<6)
#endif
#ifdef  ReleB3P
            sbic    RELEB_OUT_PORT,ReleB3P
            sbr     Res1,(1<<5)
#endif
#ifdef  ReleB2P
            sbic    RELEB_OUT_PORT,ReleB2P
            sbr     Res1,(1<<4)
#endif
            sbic    RELEB_OUT_PORT,ReleB1P
            sbr     Res1,(1<<3)

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
