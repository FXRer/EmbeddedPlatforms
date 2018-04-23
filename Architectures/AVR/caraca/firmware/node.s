/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- NODE: Initialization, Common ISR, and Event dispatcher ---*
 $Id: node.s,v 1.18 2000/12/06 05:53:31 skr Exp $
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
#include "ds1621.e"
#include "eeprom.e"

            extern ActionTable
            extern canSendSerial
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
            
            /* enable watchdog */
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
#if RESET_BLINK==1
            sbi     LED_DIR_PORT,LedDrvP
#endif
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
#if RESET_BLINK==1
            cbi     LED_OUT_PORT,LedDrvP
            sbrc    Arg4,1
            sbi     LED_OUT_PORT,LedDrvP
#endif
            dec     Arg4
            brne    boot_loop

        /* Timer 0 (8bit) */
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

        /* Analog comparator off */
            ldi     Arg1,(1<<bACD)|(0<<bACI)|(0<<bACIE)|(0<<bACIC)|(0<<bACIS1)|(0<<bACIS0)
            out     ACSR,Arg1

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
            out     MCUCR,Arg1    /* enable sleep idle mode + ext int on falling edge */

            /* enable Timer0 overflow int and Timer1 compare match int */
            ldi     Arg1, (0<<bTOIE1)|(1<<bOCIE1A)|(0<<bTICIE)|(1<<bTOIE0)
            out     TIMSK, Arg1    /* enable timers interrupt */

#if CANRxInterrupt==1
            ldi     Arg1, (1<<bINT1)|(0<<bINT0)
#else
            ldi     Arg1, (0<<bINT1)|(1<<bINT0)
#endif
            out     GIMSK, Arg1    /* enable external interrupt */

            rjmp    KernelInit


/*--------------
 * Timer0 overflow interrupt handler routine
 * The timer serves as a time base reference for the CAN and RC5 routines
 *--------------*/
public Ovf0_drv:
            in      SaveStatus,SREG

            in      IntTmp1,TCNT0
            addi(IntTmp1,TIM0_RELOAD)
            out     TCNT0,IntTmp1

            /* sample CAN Rx Pin, copy to Rxbit in CanBits reg */
            cbr     CanBits, (1 << Rxbit)
            sbic    CAN_RXIN_PORT,CanRxP
            sbr     CanBits, (1 << Rxbit)

            sbrs    KernelBits,SleepStatus
            push    ZL                  ;ZL = r30 = Arg2

            ldi     ZL,CANBUSDRV            ;destination thread
            ldi     IntTmp1,(1<<Timer0Sig)  ;signal mask
            IsrSignal(IntTmp1,Z,IntTmp2)

            ldi     ZL,RC5DECODER           ;destination thread
            ldi     IntTmp1,(1<<Timer0Sig)  ;signal mask
            IsrSignal(IntTmp1,Z,IntTmp2)

#if INTERRUPT_SCHEDULE==0
            sbrs    KernelBits,SleepStatus
            pop     ZL

            out     SREG,SaveStatus
            reti
#else
            sbrc    KernelBits,SleepStatus
            rjmp    ovf0_loc2

            pop     ZL
            out     SREG,SaveStatus
            reti
ovf0_loc2:
            IsrSchedule
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

            ldi     ZL,RC5DECODER           ;destination thread
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

/**
 * Wait1Sec: go sleep fo a second
 *
 * USED Regs: Arg1, Arg2, Arg3, Arg4
 **/
Wait1Sec:   ;Wait for 1 second
            ldi     Arg4,TIMER1_FREQUENCY
slp_loop:   ldi     Arg1,(1<<Timer1Sig)
            rcall   Wait
            dec     Arg4
            brne    slp_loop
            ret


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
            in      Arg1,RELEA_DIR_PORT
            sbr     Arg1,REL_INIT_MASKA
            out     RELEA_DIR_PORT,Arg1

            in      Arg1,RELEB_DIR_PORT
            sbr     Arg1,REL_INIT_MASKB
            out     RELEB_DIR_PORT,Arg1

#ifdef  LedLoadP
            sbi     LED_DIR_PORT,LedLoadP
            sbi     LED_OUT_PORT,LedLoadP   ;switch LED OFF on power-up
#endif
#ifdef  LedBusRxP
            sbi     LED_DIR_PORT,LedBusRxP
            sbi     LED_OUT_PORT,LedBusRxP  ;switch LED OFF on power-up
#endif
#ifdef  LedBusTxP
            sbi     LED_DIR_PORT,LedBusTxP
            sbi     LED_OUT_PORT,LedBusTxP  ;switch LED OFF on power-up
#endif
#ifdef  LedDebugP
            sbi     LED_DIR_PORT,LedDebugP
            sbi     LED_OUT_PORT,LedDebugP  ;switch LED OFF on power-up
#endif

            /*
             * We have to be sure CanBus thread is initialized.
             * Furthermore we wait a little before to send the first msg,
             * so the canbus thread can Ack other nodes that start transmit
             * when there is no listener yet, so retry again and again.
             *
             * If all the nodes start transmit and no one Ack their msg,
             * they loop forever trying to retransmit the same msg. This
             * happen only when all nodes try to transmit the same ID,
             * so all nodes win arbitration (it should happen ONLY when
             * they are all unconfigured with the same address 63).
             */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait
            rcall   Wait1Sec

            /*** Send Hello message ***/
            rcall   canSendSerial

#if HARDWARE_TEST==1
            rcall   Wait1Sec

            /* Rolling relays */
            ldi     Arg1,0          ;Clear mask
            ldi     Arg2,0xff
            rcall   SetOutput       ;switch all relays off

            ldi     Arg1,1
            mov     Reg2,Arg1
            ldi     Reg1,7
hdw_test_loop:
            rcall   Wait1Sec

            ldi     Arg1,2          ;Toggle mask
            mov     Arg2,Reg2
            rcall   SetOutput

            lsl     Reg2
            dec     Reg1
            brne    hdw_test_loop

            rcall   Wait1Sec

            ldi     Arg1,3
            ldi     Arg2,0
            rcall   SetOutput

            /* Request temperature */
            sbrc    UserBits,ThermoLockBit  ;if there's another request pending discard it
            rjmp    xtt_loc1

            ldi     ThermoSelect,7
            ldi     Arg2,THERMOMETER
            ldi     Arg1,(1<<ThermoReqSig)
            rcall   Signal
xtt_loc1:
#else
            ;Restore output status from eeprom
            rcall   GetOutputStatus
            mov     Arg2,Res1
            ldi     Arg1,3
            rcall   SetOutput
#endif

#if CANTX_TEST==1
            rcall   Wait1Sec

            ldi     Reg1,0x00      /* TxId (8 msb) */
            ldi     Arg1,0x10
            mov     Reg2,Arg1      /* TxId (3 lsb) + RTR + TxLen */
cantest_loop:
            rcall   Wait1Sec

#ifdef  LedDebugP
            in      Arg1,LED_OUT_PORT
            ldi     Arg2,(1<<LedDebugP)
            eor     Arg1,Arg2
            out     LED_OUT_PORT,Arg1
#endif
            ;Send a different Msg Id every Timer interrupt (1 sec)
            mov     Arg1,Reg1
            mov     Arg2,Reg2
            clr     Arg3
            rcall   CanTxFrame4

cantest_loc1:
            inc     Reg1            /* change Id */
            rjmp    cantest_loop
#endif


            /* Main LOOP */
thr_loop:
            /* We wait for a signal from CANBus, InputPinChange 
             * RC5 decoder sends also CAN-Messages */
            ldi     Arg1,BV(CanDataSig)|BV(KeyDataSig)
            rcall   Wait

can_test:   sbrs    Res1,CanDataSig
            rjmp    key_test

            /* Event:   CAN message received
             * Action:  look up action table
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
            rjmp    key_test

            sbr     RxBufIdx,(1<<PRI_EDISPATCHER)
            ldi     Arg4,RxId_2         ;Arg4 = YL (r28)
#else
            sbrs    UserBits,RxLockBit1
            rjmp    key_test
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

            ldi     Arg1,e2_NodeAddress ;read our address from EEPROM
            rcall   EEReadWord
 
            cp      Res1,Arg4           ;check it out
            breq    nd_addrok

            cp      Res2,Arg4           ;secondary address
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
            ;note that some Action routine don't preserve Reg1,Reg2,Reg3,Reg4
            icall                       ;Arg4 --> RxBuffer pointer
nf_test:
#ifdef  LedBusRxP
            sbi     LED_OUT_PORT,LedBusRxP      ;switch CAN-bus Rx led off
#endif

#if RX_DOUBLE_BUFFER==1
            /* Clear RxLockBit  (signal the end of processing to CANBus thread: buffer available) */
            sbrs    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit1)
            sbrc    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit2)

            rjmp    ct_buf1check        ;check if the other buffer is ready before to give up
#else
            cbr     UserBits,(1<<RxLockBit1)
#endif


key_test:
            /**
             * Res1 is not preserved. No need to test it. No problem if
             * key_test is executed even if no KeyDataSig received, since
             * KeyChange will be 0 and no event sent.
             **/
            ;sbrs    Res1,KeyDataSig
            ;rjmp    thr_loop

            /* Event:   Keystroke change detected
             * Action:  send CAN msg (ID in event table)
             */

            /*****
            Only one key may be pressed at the same time

            We can handle several type of key events:
             1) keypress - when the key is pressed
             2) keyrelease - when the key is released
             3) click - when the key is pressed and then released after a defined slice of time (i.e. 800msec)
             4) keyrepeated - after a click if the key is not released we send a keyrepeated every N msec

            At the moment we handle only keypress/keyrelease (others discarded)
            We send one data byte in the format:
             4 msb = key type (keypress = 1, keyrelease = 2, keyrepeated = 3, click = 4)
             4 lsb = key code (1, 2, 3, 4)
            *****/
#if KEY_DUMP==1
            tst     KeyChange
            breq    kt_end
            mov     Arg3,KeyChange
            mov     Arg4,OldKeyStatus
#else
            ;Test for key press
            ldi     Arg3,1      ;key press
#ifdef  Key4P
            sbrc    KeyChange,Key4P
            sbrc    OldKeyStatus,Key4P
            rjmp    kt_presstst3
            ldi     Arg4,4
            rjmp    kt_send
#endif
kt_presstst3:
#ifdef  Key3P
            sbrc    KeyChange,Key3P
            sbrc    OldKeyStatus,Key3P
            rjmp    kt_presstst2
            ldi     Arg4,3
            rjmp    kt_send
#endif
kt_presstst2:
#ifdef  Key2P
            sbrc    KeyChange,Key2P
            sbrc    OldKeyStatus,Key2P
            rjmp    kt_presstst1
            ldi     Arg4,2
            rjmp    kt_send
#endif
kt_presstst1:
            sbrc    KeyChange,Key1P
            sbrc    OldKeyStatus,Key1P
            rjmp    kt_reltest
            ldi     Arg4,1
            rjmp    kt_send
kt_reltest:
            ;Test for key release
            ldi     Arg3,2      ;key release
#ifdef  Key4P
            sbrc    KeyChange,Key4P
            sbrs    OldKeyStatus,Key4P
            rjmp    kt_reltst3
            ldi     Arg4,4
            rjmp    kt_send
#endif
kt_reltst3:
#ifdef  Key3P
            sbrc    KeyChange,Key3P
            sbrs    OldKeyStatus,Key3P
            rjmp    kt_reltst2
            ldi     Arg4,3
            rjmp    kt_send
#endif
kt_reltst2:
#ifdef  Key2P
            sbrc    KeyChange,Key2P
            sbrs    OldKeyStatus,Key2P
            rjmp    kt_reltst1
            ldi     Arg4,2
            rjmp    kt_send
#endif
kt_reltst1:
            sbrc    KeyChange,Key1P
            sbrs    OldKeyStatus,Key1P
            rjmp    kt_end
            ldi     Arg4,1
            rjmp    kt_send
#endif
kt_send:
            ldi     Arg1,e2_KeyChange
            rcall   CanTxFrame3Queue

            clr     KeyChange
kt_end:
            rjmp    thr_loop
