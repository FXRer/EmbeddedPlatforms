/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1999-2000 by Konrad Riedel
 * e-mail: k.riedel@gmx.de 
 * WWW: http://caraca.sourceforge.net
 *
 *--- NODE: Initialization, Dimmer, and Event dispatcher ---*
 *    based on work by Claudio Lanconelli
 $Id: node_skr.s,v 1.14 2000/12/07 11:09:59 skr Exp $
 
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

            extern e2_Bright_Tbl
            extern ActionTable
            extern canSendSerial
            extern e2_SendDebug
            extern SetOutput
            extern GetOutputStatus
            extern SetDimmer

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
            sbi     LED_DIR_PORT,LedDebugP
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
            cbi     LED_OUT_PORT,LedDebugP
            sbrc    Arg4,1
            sbi     LED_OUT_PORT,LedDebugP
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

/** Timer 1 (16bit) is used for the Dimmer
         * TCCR1A zero (default)
         * TCCR1B
         * prescaler = 1 (1) */

#define MAINS_FREQ   50
#define T1_PRESCALE   1
#define MAINS_CYCLE  (XTAL_FREQUENCY/T1_PRESCALE/(MAINS_FREQ*2)/2)
/* we fire the triac twice (one for the positive and one for the negative)
   we toggle the output pin OC1 and use a Capacitor 100nF to
   generate the short pulse for the triac for every LH change */
             
            ldi     r31,BV(CS10)|BV(CTC1)
            ;ldi     r31,BV(CS11)|BV(CTC1)
            out     TCCR1B,r31
            clr     TriacBits     /* Dimmer state (off) */
            ldi     r31,high(MAINS_CYCLE)         
            out     OCR1AH,r31
            ldi     r31,low(MAINS_CYCLE)
            out     OCR1AL,r31
            ldi     r31,(1<<bSE)|(0<<bSM) | (1<<bISC11)|(1<<bISC10) | (1<<bISC01)|(0<<bISC00)
            out     MCUCR,r31    /* enable sleep idle mode + int0 falling
                                  * +int1 on rising edge */

            /* enable Timer0 overflow int */
            ldi     r31, (0<<bTOIE1)|(1<<bOCIE1A)|(0<<bTICIE)|(1<<bTOIE0)
            out     TIMSK, r31    /* enable timer0,1 interrupt (overflow) */

            ldi     r31, (1<<bINT1)|(1<<bINT0)
            out     GIMSK, r31    /* enable external interrupt 0,1 */

            rjmp    KernelInit

/***************************************************************************
 * INTERRUPTS
 ***************************************************************************/

/*--------------
 * Timer0 overflow interrupt handler routine
 * The timer serves as a time base reference for the CAN and RC5 routines
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
#ifdef RC5DECODER
            ldi     ZL,RC5DECODER       /* destination thread */
            ldi     r31,(1<<Timer0Sig)  /* signal mask */
            IsrSignal(r31,Z,IntTmp)
#endif
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
 * External Interrupt1 Vector
 * Zero crossing detection (conn. to mains via 2x10MOhm)
 * It fires on every rising edge. 
 *         _     _
 *        / \   / \
 *           \_/
 *                
 *        |     |         <-Int1
 *
 * To correct phase shift between internal oscillator and mains frequency, we 
 * simply reset the timing constant of Timer1 to a precalculated value.
 * We can do this because interrupt response time is sufficiently stable. (I've 
 * tried "smarter" algorithms, which look at the timer value every ZCDint and
 * adjust the timer1 cycle lenght, but this used more regs and didn't work...
 *
 * We also have a counter based on this. (Register name Timer)
 * We also generate a Timer1Sig on every zero crossing.
 *--------------*/

public Int1_drv:    
            in      SaveStatus,SREG
            cbr     TriacBits,BV(TOV1Cnt)|BV(TOV1Cnt+1)|BV(TOV1Cnt+2)  
            sbrs    TriacBits,TSwitchOn
            rjmp    to_timer

/* we received a TSwitchOn request */
            out     TCNT1H,TriZCnew        /* set new timer constant */
            clr     IntTmp
            out     TCNT1L,IntTmp       
            sbr     TriacBits,BV(TSynced)  /* signal this to Oc1 int   */
            sbrc    TriacBits,TBrightBit
            sbr     TriacBits,BV(TBrightChange)
            rjmp    int1_timer             /* so we can switch Light on */ 
                                           /* without flickering */
to_timer:   out     TCNT1H,TriZC           /* rewrite timer constant */
            clr     IntTmp                 
            out     TCNT1L,IntTmp          /* this eliminates drift */

#ifdef EXTRA_BRIGHT_CHECK
/* it checks the state of TBrightBit during zero crossing, was usable for debugging */
check_pinL: sbrc    TriacBits,TBrightBit
            rjmp    check_pinH  
            sbis    TRIAC_IN_PORT,TriacDIM  /* reading 1 means lamp is darker */
            sbr     TriacBits,BV(TBrightChange)
            rjmp    pin_done
check_pinH: sbic    TRIAC_IN_PORT,TriacDIM  /* reading 0 means lamp is brighter */
            sbr     TriacBits,BV(TBrightChange)
            rjmp    pin_done

#endif
pin_done: 
#ifdef STRESS_CAN_BUS
            /* First check if the buffer is available */
            sbrc    UserBits,TxLockBit
            rjmp    can_busy
	    push    Arg1                /* This is ZH */
            push    Arg2                /* ZL */
            ldi     ZL,TxId         /* Message over CAN */
            ldi     r31,0x55
            st      Z+,r31          /* TxId in Arg1 */
 
            ldi     r31,3
            st      Z+,r31          /* TxLen in Arg4 */

            st      Z+,TriZC
            
            ;in      IntTmp,TCNT1L
            ;st      Z+,IntTmp
            ;in      IntTmp,TCNT1H
            ;st      Z+,IntTmp
            in      IntTmp,TRIAC_IN_PORT
            st      Z+,IntTmp
	    st      Z+,TBright
 
            sbr     UserBits,(1<<TxLockBit)
            ldi     ZL,CANBUSDRV        /* destination thread */
            ldi     r31,(1<<CanReqSig)  /* signal mask */
            IsrSignal(r31,Z,IntTmp)
	    pop     Arg2
            pop     Arg1
can_busy:
#endif ;STRESS_CAN_BUS
int1_timer:  
 
	    inc     Timer
;           ldi     IntTmp, 5           ;10 TimerTicks per second - base for Keytest
;	    cp      Timer,IntTmp
;            brne    int1_end
;            clr     Timer

            push    ZL      /* ZL = r30 = Arg2 */

#if (CANTX_TEST | HARDWARE_TEST)
            ldi     ZL,EDISPATCHER          ;destination thread
            ldi     IntTmp1,(1<<Timer1Sig)  ;signal mask
            IsrSignal(IntTmp1,Z,IntTmp2)
#endif
            ldi     ZL,RC5DECODER           ;destination thread
            ldi     IntTmp1,(1<<Timer1Sig)  ;signal mask
            IsrSignal(IntTmp1,Z,IntTmp2)

            pop     ZL

     
int1_end:   
#ifdef TOGGLE
; this is a try to implement "dimming for heating": x waves on, then y waves off
; but i don't need this at the moment 

            ;
            in      IntTmp,PORTC
            sbrs    IntTmp,HeatingP
            sbi     PORTC, HeatingP
	    sbrc    IntTmp,HeatingP
            cbi     PORTC, HeatingP
#else
            in      IntTmp,LED4_OUT_PORT
            sbrs    IntTmp,HeatingFlag
            sbi     PORTC, HeatingP
	    sbrc    IntTmp,HeatingFlag
            cbi     PORTC, HeatingP
#endif
	    out     SREG,SaveStatus
            reti

/*--------------
 * interrupt handler routine
 * currently used to change phase for Lamp-Brightness
 * 
 * we toggle the TriacDIM pin on every timer overflow, the triac gets
 * fired with every LH-change, so counter overflows TWICE every mains half-wave
 * so status of TriacDIM on zero crossing is important. Because we can't
 * set the pin directly, we generate a short extra cycle, which toggles
 * the pin.
 *--------------*/
public Oc1_drv:     /* Output Compare1 Interrupt Vector */
            in      SaveStatus,SREG
            inc     TriacBits                   /* count OV1's during mains wave,
                                                   every ZCDint resets 3bit counter  */
tst_change: sbrs    TriacBits,TBrightChange
            rjmp    no_change
            sbrs    TriacBits,TOV1Cnt+2         /* syncing - see next comment */
            rjmp    no_change

/* The TriacDIM pin gets toogles here - this has to be in sync with the
 * new timing constant, which got updated during ZCD int, so we toggle during
 * the LAST OV1int BEFORE the next ZCDint (This is the 4th)
 */
            ldi     IntTmp,high(MAINS_CYCLE)    /* short extra cycle */
            out     TCNT1H,IntTmp
            ldi     IntTmp,low(MAINS_CYCLE)-3
            out     TCNT1L,IntTmp
            cbr     TriacBits,BV(TBrightChange) /* changing done, reset bit */
no_change:  mov     TriZC,TriZCnew
            sbrc    TriacBits,TOn
            rjmp    Oc1_end
state_off:  sbrs    TriacBits,TSynced
            rjmp    switch_off
            /* Timer is in sync, if high Brightness, Lamp on */
            ;sbrs    TriacBits,TBrightBit
            ;rjmp    switch_on
            ;sbrs    TriacBits,TOV1Cnt+1      /* wait for next OV */
            ;rjmp    Oc1_end
switch_on:  ldi     IntTmp,BV(bCOM1A0)
            out     TCCR1A,IntTmp            /* connect to pin OC1 */
            sbr     TriacBits,BV(TOn)
            cbr     TriacBits,BV(TSwitchOn) 
            rjmp    Oc1_end
           
switch_off: clr     IntTmp 
            out     TCCR1A,IntTmp           /* disconnect from pin OC1 */
	    cbi     TRIAC_OUT_PORT,TriacDIM  
Oc1_end:    out     SREG,SaveStatus
            reti


/*--------------
 * Other unused interrupt vector
 *--------------*/
public Ovf1_drv:    /* Overflow1 Interrupt Vector */
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
        seg removable flash.code.Sleep1s
Sleep1s:    ;Sleep for 1 second
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
 *
 * local vars:
 * Reg2 - Signal mask
 *=================================*/
        seg removable flash.code.Thread2 


public Thr2_entry:
            /* put thread initialization code here */

            in      Arg1,LED_DIR_PORT
            sbr     Arg1,BV(Led1P)|BV(Led2P)|BV(Led3P)
            out     LED_DIR_PORT,Arg1
#ifdef Led4P
            in      Arg1,LED4_DIR_PORT
            sbr     Arg1,BV(Led4P)
            out     LED4_DIR_PORT,Arg1
#endif
            in      r31,TRIAC_DIR_PORT
            sbr     r31,BV(TriacSW)|BV(TriacDIM)
            out     TRIAC_DIR_PORT,r31
            in      r31,DDRC
            sbr     r31,BV(HeatingP)
            out     DDRC,r31
	    
            ldi     r31,MAX_BRIGHTNESS
            mov     TBright,r31
	    rcall   SetDimmer

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
            ;rcall   Sleep1s

            /*** Send Hello message ***/
            rcall   canSendSerial
#if CANTX_TEST
            ldi     Reg1,0x00      /* TxId (8 msb) */
            ldi     Arg1,0x10
            mov     Reg2,Arg1      /* TxId (3 lsb) + RTR + TxLen */
cantest_loop:
            rcall   Sleep1s

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
#else
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
            ld      Arg3,Y+             ;TxId
            ld      Arg2,Y+             ;TxLen
/** I use the following addressing scheme for CAN-Adressing
  *         
  * 15   14   13  12  11  10   9   8   7   6   5   4  3  2  1  0
  * Id11 Id10 Id9 Id8 Id7 Id6 Id5 Id4 Id3 Id2 Id1  R N4 N3 N2 N1
  * +---   Function  _--+ +----- Node ID --------+
  * +------------- TxId ------------++--------- TxLen ---------+
  */
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
	    ;Activity Led  
            ldi     Arg2,(1<<LedDebugP)
            in      Arg1,LED_OUT_PORT
            eor     Arg1,Arg2
            out     LED_OUT_PORT,Arg1
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
#endif
