/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- STAR: Initialization, Common ISR, and Event dispatcher ---*

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

            extern ActionTable:
            extern canSendHello
            extern SetOutput
            extern GetOutputStatus

/***************************************************************************
 * ROUTINES
 ***************************************************************************/
        seg flash.code
/***************************************************************************
 * INITIALIZATION
 **************************************************************************/
public Reset_drv:
        /* do not use the stack here!!! It's initialized within KernelInit() */

        /* enable watchdog here */
            ldi     Arg1,(0<<bWDTOE)|(1<<bWDE)|(1<<bWDP2)|(1<<bWDP1)|(1<<bWDP0)
            out     WDTCR,Arg1

/* This loop blink the LED and delay the startup. The amount of the delay
 * depend on the serial number, so every node have a different startup delay.
 * It's useful for sending the Hello message on power-up without bus contention
 */
            ldi     ZL,low(SerialNumber+1)   /* ZL = Arg2 */
            ldi     ZH,high(SerialNumber+1)  /* ZH = Arg1 */
            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg4,Reg2

            sbi     LED_DIR_PORT,LedDrvP
boot_loop:
            ldi     Arg1,255
            ldi     Arg2,200
            ldi     Arg3,2
delay_loop:
            wdr
            dec     Arg1
            brne    delay_loop
            dec     Arg2
            brne    delay_loop
            dec     Arg3
            brne    delay_loop
#if RESET_BLINK
            cbi     LED_OUT_PORT,LedDrvP
            sbrc    Arg4,1
            sbi     LED_OUT_PORT,LedDrvP
#endif
            dec     Arg4
            brne    boot_loop


        /* Timer 0 (8bit)
        /* prescaler = 2 (0: stop, 1:CK, 2:CK/8, 3:CK/64, 4:CK/256, 5:CK/1024) */
#if TIMER0_PRESCALER==1
            ldi     Arg1,1
#endif
#if TIMER0_PRESCALER==8
            ldi     Arg1,2
#endif
#if TIMER0_PRESCALER==64
            ldi     Arg1,3
#endif
#if TIMER0_PRESCALER==256
            ldi     Arg1,4
#endif
#if TIMER0_PRESCALER==1024
            ldi     Arg1,5
#endif            
            out     TCCR0,Arg1

        /* Timer 1 (16bit)
         * TCCR1A zero (default)
         * TCCR1B
         * prescaler = 1024 (CS=101) clear on compare match ON */
            ldi     Arg1,(0<<bICNC1)|(0<<bICES1)|(1<<bCTC1)|(1<<bCS12)|(0<<bCS11)|(1<<bCS10)
            out     TCCR1B,Arg1

            ;Set timer1 match every 1/x seconds
            ldi     Arg1,(TIMER1_RELOAD >> 8)
            ldi     Arg2,(TIMER1_RELOAD & 0xff)
            out     OCR1AH,Arg1
            out     OCR1AL,Arg2

#if CANRxInterrupt==1
            ldi     Arg1,(1<<bSE)|(0<<bSM) | (1<<bISC11)|(0<<bISC10) | (0<<bISC01)|(0<<bISC00)
#else
            ldi     Arg1,(1<<bSE)|(0<<bSM) | (0<<bISC11)|(0<<bISC10) | (1<<bISC01)|(0<<bISC00)
#endif
            out     MCUCR,Arg1    /* enable sleep idle mode + ext int
                                   * on falling edge */

            /* enable Timer0 overflow int and Timer1 compare match int */
            ldi     Arg1, (0<<bTOIE1)|(1<<bOCIE1A)|(0<<bTICIE)|(1<<bTOIE0)
            out     TIMSK,Arg1

#if CANRxInterrupt==1
            ldi     Arg1, (1<<bINT1)|(0<<bINT0)
#else
            ldi     Arg1, (0<<bINT1)|(1<<bINT0)
#endif
            out     GIMSK,Arg1    /* enable external interrupt */

            rjmp    KernelInit


/*--------------
 * Timer0 overflow interrupt handler routine
 * The timer serves as a time base reference for the CAN driver
 *--------------*/
public Ovf0_drv:
            in      SaveStatus,SREG

            in      IntTmp,TCNT0
            addi(IntTmp,TIM0_RELOAD)
            out     TCNT0,IntTmp

            /* sample CAN Rx Pin, copy to Rxbit in CanBits reg */
            cbr     CanBits, (1 << Rxbit)
            sbic    CAN_RXIN_PORT,CanRxP
            sbr     CanBits, (1 << Rxbit)

            sbrc    KernelBits,SleepStatus
            rjmp    ovf0_loc1
            push    r31
            push    ZL
ovf0_loc1:
            ldi     ZL,CANBUSDRV        /* destination thread */
            ldi     r31,(1<<Timer0Sig)  /* signal mask */
            IsrSignal(r31,Z,IntTmp)

            sbrc    KernelBits,SleepStatus
            rjmp    ovf0_loc2
            pop     ZL
            pop     r31

#if INTERRUPT_SCHEDULE
            out     SREG,SaveStatus
            reti
ovf0_loc2:
            IsrSchedule
#else
ovf0_loc2:
            out     SREG,SaveStatus
            reti
#endif

/*--------------
 * Timer1 output compare interrupt handler routine
 * Freerun long delay timer
 *--------------*/
public Oc1_drv:     /* Output Compare1 Interrupt Vector */
            in      SaveStatus,SREG
            push    ZL      /* ZL = r30 = Arg2 */

            ldi     ZL,EDISPATCHER          ;destination thread
            ldi     IntTmp1,(1<<Timer1Sig)  ;signal mask
            IsrSignal(IntTmp1,Z,IntTmp2)

            pop     ZL
            out     SREG,SaveStatus
            reti


/*--------------
 * Timer1 overflow interrupt handler routine
 * Freerun long delay timer
 *--------------*/
public Ovf1_drv:    /* Overflow1 Interrupt Vector */
/*          in      SaveStatus,SREG
            out     SREG,SaveStatus
            reti
*/

/*---------
 * CANBus can use Interrupt0 or Interrupt1
 * Other unused interrupt is here
 *---------*/
#if CANRxInterrupt==1
public Int0_drv:    ;External Interrupt0 Vector
#else
public Int1_drv:    ;External Interrupt1 Vector
#endif


/*--------------
 * Other unused interrupt vector
 *--------------*/
public Icp1_drv:    /* Input Capture1 Interrupt Vector */
public Urxc_drv:    /* UART Receive Complete Interrupt Vector */
public Udre_drv:    /* UART Data Register Empty Interrupt Vector */
public Utxc_drv:    /* UART Transmit Complete Interrupt Vector */
public Aci_drv:     /* Analog Comparator Interrupt Vector */
#ifdef  AT90S4433
public Spi_drv:     /* SPI Interrupt Vector */
public Adc_drv:     /* ADC Interrupt Vector */
public EErdy_drv:   /* EEProm ready Interrupt Vector */
#endif
            reti



/* Arg1: start of table in EEprom
 * Arg2: table size in word
 * Reg3: key (msb)
 * Reg4: key (lsb)
 *
 * Return: Carry set if Not found
 *         Carry clear if found and index in Res1
 *
 * USED Regs: Arg1, Arg2, Arg3, Arg4
 */
        seg removable flash.code.TableLookup
public TableLookup:
            push    Reg1            ;TableLookup uses Reg1

            mov     Arg4,Arg1
            mov     Reg1,Arg2
            clr     Arg3
tb_loop:
            mov     Arg1,Arg4
            rcall   EEReadWord
            /* Res1:Res2 store the entry */
            cp      Res2,Reg4       /* compare the entry against the key */
            cpc     Res1,Reg3
            breq    tb_found
            addi(Arg4,2)
            inc     Arg3
            dec     Reg1
            brne    tb_loop

            /* not found */
            sec
            rjmp    tb_end

tb_found:   /* found */
            mov     Res1,Arg3
            clc
tb_end:
            pop     Reg1
            ret


/*=================================
 * THREAD 2  Event and Action dispatcher
 * EDISPATCHER
 *=================================*/
        seg removable flash.code.Thread2
public Thr2_entry:
            /* put initialization code here */

#if (TRACERX_BIT) | (TRACEBITERR)
            ldi     Arg2,BV(TraceRxBitP)|BV(TraceInt0P)|BV(TraceBitErrP)|BV(TraceStuffErrP)|BV(TraceFormErrP)
            in      Arg1,TRACE_DIR_PORT
            or      Arg1,Arg2
            out     TRACE_DIR_PORT,Arg1

            com     Arg2
            in      Arg1,TRACE_OUT_PORT
            and     Arg1,Arg2
            out     TRACE_OUT_PORT,Arg1
#endif
            sbi     LED_OUT_PORT,LedDrvP    ;switch LED OFF on power-up

            in		Arg1,RELEA_DIR_PORT
            sbr		Arg1,REL_INIT_MASKA
            out		RELEA_DIR_PORT,Arg1

            in		Arg1,RELEB_DIR_PORT
            sbr		Arg1,REL_INIT_MASKB
            out		RELEB_DIR_PORT,Arg1

            /* Wait for CanBus thread */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait

            rcall   InitAlarmTable

            /*** Send Hello message ***/
            rcall   canSendHello

#if HARDWARE_TEST==1
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait

            /* Rolling relays */
            ldi     Arg1,0          ;Set mask
            ldi     Arg2,0xff
            rcall   SetOutput       ;switch all relays off

            ldi     Arg1,1
            mov     Reg2,Arg1
            ldi     Reg1,8
hdw_test_loop:
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait

#ifdef  LedDrvP
            in      Arg1,LED_OUT_PORT
            ldi     Arg2,(1<<LedDrvP)
            eor     Arg1,Arg2
            out     LED_OUT_PORT,Arg1
#endif
            ldi     Arg1,2          ;Toggle mask
            mov     Arg2,Reg2
            rcall   SetOutput

            lsl     Reg2
            dec     Reg1
            brne    hdw_test_loop
#endif

            ;Restore output status from eeprom
            rcall   GetOutputStatus
            mov     Arg2,Res1
            ldi     Arg1,3
            rcall   SetOutput


#if CANTX_TEST==1
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait

            ldi     Reg1,0x00      /* TxId (8 msb) */
            ldi     Arg1,0x10
            mov     Reg2,Arg1      /* TxId (3 lsb) + RTR + TxLen */
cantest_loop:
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait

            in      Arg1,LED_OUT_PORT
            ldi     Arg2,(1<<LedDrvP)
            eor     Arg1,Arg2
            out     LED_OUT_PORT,Arg1

            mov     Arg3,Reg1
            mov     Arg4,Reg2
            rcall   CanTxFrame4

cantest_loc1:
            inc     Reg1            /* change Id */
            rjmp    cantest_loop
#endif


            /* Main LOOP */
thr_loop:
            /* We wait for a signal from CANBus */
            ldi     Arg1,(1<<CanDataSig)|(1<<Timer1Sig)
            rcall   Wait
            mov     Reg2,Res1

can_test:
            sbrs    Reg2,CanDataSig
            rjmp    alarm_test

            /* Event:   CAN message received
             * Action:  look up action table and association table
             */

            /**
             * Current RxBuffer (1 or 2) is stored in RxBufIdx(PRI_EDISPATCHER).
             * Test first RxBuffer1, if it's empty switch to RxBuffer2.
             * Store current RxBuffer used.
             **/
ct_buf1check:
#if RX_DOUBLE_BUFFER==1
            sbrs    UserBits,RxLockBit1
            rjmp    ct_buf2check

            cbr     RxBufIdx,(1<<PRI_EDISPATCHER)
            ldi     Arg4,RxId_1         ;Arg4 = YL (r28)
            rjmp    ct_copyloc

ct_buf2check:
            sbrs    UserBits,RxLockBit2
            rjmp    alarm_test

            sbr     RxBufIdx,(1<<PRI_EDISPATCHER)
            ldi     Arg4,RxId_2         ;Arg4 = YL (r28)
#else
            sbrs    UserBits,RxLockBit1
            rjmp    alarm_test
            ldi     Arg4,RxId_1         ;Arg4 = YL (r28)
#endif
ct_copyloc:
            ld      Arg3,Y+
            ld      Arg2,Y+

            mov     Arg4,Arg3
            andi    Arg4,0x07
            andi    Arg2,0xE0
            or      Arg4,Arg2
            swap    Arg4
            /* Arg4 <-- (Node Address) << 1 */

            tst     Arg4                ;check for broadcast address
            breq    nd_addrok

            ldi     Arg1,e2_NodeAddress
            rcall   EEReadWord

            cp      Arg1,Arg4
            breq    nd_addrok

            cp      Arg2,Arg4
            brne    nf_test             ;no matching address

nd_addrok:
            /* Arg3 <-- Function Code (5 msb of ID) */
            lsr     Arg3
            lsr     Arg3
            ;lsl     Arg3               ;ActionTable: every entry is 2 bytes size
            cbr     Arg3,1              ;Arg3 --> CallTable index

            ldi     ZL,low(ActionTable)   /* ZL = Arg2 */
            ldi     ZH,high(ActionTable)  /* ZH = Arg1 */
            add     ZL,Arg3
            clr     Arg3
            adc     ZH,Arg3

            lpm                         ; Reg2 <-- (Z); r0 = Reg2
            mov     Arg3,Reg2
            adiw    ZL,1                ;ZL = Arg2 (r30)
            lpm
            mov     ZL,Reg2             ;ZL = Arg2 (r30)
            mov     ZH,Arg3             ;ZH = Arg1 (r31)

            ldi     Arg4,RxBuf_1        ;Arg4 = YL (r28)
#if RX_DOUBLE_BUFFER==1
            sbrc    RxBufIdx,PRI_EDISPATCHER
            ldi     Arg4,RxBuf_2
#endif
            icall                       ;Arg4 --> RxBuffer pointer

nf_test:    /* Look for Association Table */

#if ENABLE_ASSTABLE==1
            ldi     Reg1,e2_AssociationTable
at_loop:
            mov     Arg1,Reg1
            addi(Reg1,2)
            rcall   EEReadWord      ;read ID Mask
            clr     Arg3
            cp      Arg2,Arg3       ;Zero mask identify last entry
            cpc     Arg1,Arg3
            breq    at_endloop

            mov     KeyMask1,Arg1
            mov     KeyMask2,Arg2

            mov     Arg1,Reg1
            addi(Reg1,2)
            rcall   EEReadWord      ;Read ID entry

            and     Arg1,KeyMask1
            and     Arg2,KeyMask2

            ldi     Arg4,RxId_1        ;Arg4 = YL (r28)
#if RX_DOUBLE_BUFFER==1
            sbrc    RxBufIdx,PRI_EDISPATCHER
            ldi     Arg4,RxId_2
#endif
            ld      Reg3,Y+
            and     Reg3,KeyMask1
            ld      Reg4,Y+
            and     Reg4,KeyMask2

            cp      Arg2,Reg4       ;compare Entry ID with received ID
            cpc     Arg1,Reg3
            breq    at_entrytest2
            addi(Reg1,12)           ;skip remaining bytes
            rjmp    at_loop
at_entrytest2:
            mov     Arg1,Reg1
            inc     Reg1
            rcall   EEReadByte
            mov     Reg3,Arg1
/**
            ld      Arg2,Y+         ;Rx len
            andi    Arg2,0x0F
            cp      Arg2,Reg3       ;Compare the RecvLen with number of bytes to compare
            brcc    at_entrytest3
            mov     Reg3,Arg2
at_entrytest3:
**/
            tst     Reg3
            brne    at_entrytest4
            addi(Reg1,3)            ;no data to test: test passed
            rjmp    at_testok
at_entrytest4:
            ldi     Arg3,3          ;Arg3 data byte counter
at_testloop:
            dec     Arg3
            mov     Arg1,Reg1
            inc     Reg1
            rcall   EEReadByte
            ld      Arg2,Y+
            cp      Res1,Arg2
            breq    at_entrytest5
            add     Reg1,Arg3       ;jump to next entry
            addi(Reg1,8)
            rjmp    at_loop
at_entrytest5:
            dec     Reg3
            brne    at_testloop
            add     Reg1,Arg3       ;skip data bytes not tested
at_testok:
            ;Test passed: check delay
            mov     Arg1,Reg1
            addi(Reg1,2)
            rcall   EEReadWord
            clr     Arg3
            cp      Arg2,Arg3       ;check for no delay
            cpc     Arg1,Arg3
            brne    at_addqueue
at_sendaction:
            ;Send Action
            mov     Arg1,Reg1
            addi(Reg1,6)
            rcall   CanTxEEFrame3Queue
            rjmp    at_loop
at_addqueue:
            ;Insert Alarm in alarm table
            mov     Arg3,Reg1
            addi(Reg1,6)
            rcall   AddAlarmQueue
            rjmp    at_loop
at_endloop:
#endif

#ifdef  LedBusRxP
            sbi     LED_OUT_PORT,LedBusRxP      ;switch CAN-bus Rx led off
#endif

#if RX_DOUBLE_BUFFER==1
            /* Clear RxLockBit  (signal the end of processing to CANBus thread: buffer available) */
            sbrs    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit1)
            sbrc    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit2)

            rjmp    ct_buf1check
#else
            cbr     UserBits,(1<<RxLockBit1)
#endif

alarm_test:
            sbrs    Reg2,Timer1Sig
            rjmp    thr_loop

            /* Event:   Timer Alarm received
             * Action:  look up Alarm table
             */

            ;Increment Alarm counter
            clr     Arg1
            ldi     Arg2,1
            add     AlarmCountL,Arg2
            adc     AlarmCountH,Arg1

            ldi     XL,AlarmTable     ;Reg3 = XL = r26
alarm_loop:
            mov     Reg4,XL
            ld      Arg1,X+
            ld      Arg2,X+
            ld      Arg3,X+
            tst     Arg1
            breq    alarm_skip
            cp      Arg3,AlarmCountL
            cpc     Arg2,AlarmCountH
            brne    alarm_skip

            rcall   CanTxEEFrame3Queue

            ;Free the entry in alarmtable
            mov     ZL,Reg4
            clr     Arg1
            st      Z,Arg1
alarm_skip:
            cpi     XL,EndOfAlarmTable
            brne    alarm_loop

            rjmp    thr_loop



/* Arg1-Arg2: delay
 * Arg3:      EEPROM address that store the msg to send
 *
 * Return: Carry set if the table is full
 *         Carry clear if ok
 *
 * USED Regs: Arg4, KeyMask1
 */
        seg removable flash.code.AddAlarmQueue
AddAlarmQueue:
            ;first check if the alarm is already scheduled
            ldi     YL,AlarmTable     ;Arg4 = YL = r28
alr_loop1:
            ld      KeyMask1,Y+
            cp      KeyMask1,Arg3
            breq    alr_found
            addi(YL,2)
            cpi     YL,EndOfAlarmTable
            brne    alr_loop1

            ;now look for an empty entry
            ldi     YL,AlarmTable     ;Arg4 = YL = r28
alr_loop:
            ld      KeyMask1,Y
            tst     KeyMask1
            breq    alr_ok
            addi(YL,3)
            cpi     YL,EndOfAlarmTable
            brne    alr_loop
            sec
            ret
alr_ok:
            st      Y+,Arg3
alr_found:
            add     Arg2,AlarmCountL
            adc     Arg1,AlarmCountH

            st      Y+,Arg1
            st      Y+,Arg2

            clc
            ret


/***
 ***/
InitAlarmTable:
            ldi     YL,AlarmTable     ;Arg4 = YL = r28
            clr     Arg1
ialr_loop:
            st      Y+,Arg1
            addi(YL,2)
            cpi     YL,EndOfAlarmTable
            brne    ialr_loop
            ret


/*******************************************************
 *******************************************************/
        seg    eram.data
AlarmTable:
            ds.b    3*8          /* max 8 entries of 3 bytes each */
EndOfAlarmTable:

