/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- ISRoutines and Event dispatcher for CANDONGLE---*
      by Konrad Riedel k.riedel@gmx.de 
      $Id: dongle.s,v 1.12 2002/01/04 18:01:07 lancos Exp $

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

#include "eeprom.e"
#include "kernel.e"
#include "canbus.e"

        seg eram.data
/**
sTxId:          ds.b  1           ; RS232 Buffer TxMsg Id (8 msb)
sTxLen:         ds.b  1           ; number of bytes to transmit
sTxBuf:         ds.b  MAX_CANMSG_SIZE
**/
sTxMsgBuf:      ds.b    MAX_CANMSG_SIZE+2

/**
sRxId:          ds.b  1           ; RxMsg Id (8 mbs)
sRxLen:         ds.b  1           ; number of received bytes
sRxBuf:         ds.b  MAX_CANMSG_SIZE
**/
sRxMsgBuf:      ds.b    MAX_CANMSG_SIZE+2


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
 
#ifdef  LedYellowP
            sbi     LED_DIR_PORT,LedBusTxP
#if RESET_BLINK==1
/* The test loop blink the LED 3 times to show a reset occurs */
            ldi     r28,6
test_loop1:
            ldi     r31,255
            ldi     r30,255
            ldi     r29,5
test_loop:
            wdr
            dec     r31
            brne    test_loop
            dec     r30
            brne    test_loop
            dec     r29
            brne    test_loop
            cbi     LED_OUT_PORT,LedBusTxP
            sbrc    r28,0
            sbi     LED_OUT_PORT,LedBusTxP
            dec     r28
            brne    test_loop1
#endif
#endif

        /* Timer 0 (8bit)
        /* prescaler = 2 (0: stop, 1:CK, 2:CK/8, 3:CK/64, 4:CK/256, 5:CK/1024) */
            /* frequency = 10MHz / 8 / 50 = 25000 Hz */
            /* frequency = 10MHz / 8 / 65 = 19230,77 Hz */
            /* frequency = 10MHz / 8 / 80 = 15625 Hz */
#ifdef CANDONGLE_DEBUG_SLOW
            ldi     r31,3
#else        
            ldi     r31,2        
#endif
            out     TCCR0,r31

        /* Timer 1 (16bit)
         * TCCR1A zero (default)
         * TCCR1B
         * prescaler = 1024 (5) */
            ldi     r31,4
            out     TCCR1B,r31

#if CANRxInterrupt==1
            ldi     r31,(1<<bSE)|(0<<bSM) | (1<<bISC11)|(0<<bISC10) | (0<<bISC01)|(0<<bISC00)
#else
            ldi     r31,(1<<bSE)|(0<<bSM) | (0<<bISC11)|(0<<bISC10) | (1<<bISC01)|(0<<bISC00)
#endif
            out     MCUCR,r31    /* enable sleep idle mode + ext int0
                                  * on falling edge */

            /* enable Timer0 overflow int */
            ldi     r31, (0<<bTOIE1)|(0<<bOCIE1A)|(0<<bTICIE)|(1<<bTOIE0)
            out     TIMSK, r31    /* enable timer0 interrupt (overflow) */

#if CANRxInterrupt==1
            ldi     r31, (1<<bINT1)|(0<<bINT0)
#else
            ldi     r31, (0<<bINT1)|(1<<bINT0)
#endif
            out     GIMSK, r31    /* enable external interrupt 0 */

       
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
 * UART Receive Complete Interrupt Vector
 *--------------*/
public Urxc_drv:
            in      SaveStatus,SREG
/*
 * Loading incoming UART data into the serial input buffer
 */        
               
            in      IntTmp1,UDR               /* Get the char */
#ifdef DEBUG_ECHO
            out     UDR,IntTmp1               /* echo back */
#endif
            cpi     sRxMode, 0x00
            brne    Check_C
            mov     sRxMode, IntTmp1
            rjmp    Ser_Done
Check_C:    cpi     sRxMode, 'C'
            breq    Mode_C
            clr     sRxMode                 /* no valid mode found */
            rjmp    Ser_Done                /* try to resync next char */

Mode_C:
            push    YL
            cpi     sRxPos,1                 /* 2nd byte - 4Bits MsgLen */
            brne    Store_Ser_Data        
            mov     sRxMsgLen, IntTmp1
            andi    sRxMsgLen, 0x0f
Store_Ser_Data:
            ldi     YL,low(sRxMsgBuf)         /* Stored pointer to tail */
            add     YL,sRxPos
            st      Y,IntTmp1                 /* Store the byte */

            inc     sRxPos
            cpi     sRxPos,3                /* receiving header */ 
            brlo    Store_Ser_Done          /* so don't dec sRxMsgLen */
 
            dec     sRxMsgLen
            brpl    Store_Ser_Done
        /* Message is complete here, sRxMsgLen = 0xff */
            sbi     UART_OUT_PORT, SerCTS    /* I'm full */
            clr     sRxPos             /* for the next message */
            clr     sRxMode
#ifdef DEBUG_ECHO
            ldi     IntTmp1, '.'
            out     UDR, IntTmp1              /* echo back */
#endif
            /* TxBuffer available? (TxLen = FF) */
            ldi     YL,EDISPATCHER          ;destination thread
            ldi     IntTmp1,BV(RSDataSig)   ;signal mask
            IsrSignal(IntTmp1,Y,IntTmp2)

Store_Ser_Done:
            pop     YL
Ser_Done:   out     SREG,SaveStatus
            reti


/*--------------
 * UART Transmit Complete Interrupt Vector
 *--------------*/
public Utxc_drv:
            in      SaveStatus,SREG
            cpi     sTxMsgLen,0xff           /* Something to transmit? */
            breq    RSTx_No 
/*
 * Ready to transmit
 */        
            mov     IntTmp2,YL              ;save YL in IntTmp2 register
            ldi     YL,low(sTxMsgBuf)            /* Stored pointer to tail */
        
            add     YL,sTxPos
            ld      IntTmp1,Y                 /* Get the byte */
       
            out     UDR,IntTmp1
            inc     sTxPos
            dec     sTxMsgLen
            brpl    RSTx_Done
         /* Message is complete here, sTxMsgLen = 0xff */
            clr     sTxPos
RSTx_Done:  mov     YL,IntTmp2              ;restore YL from IntTmp2 register
RSTx_No:    out     SREG,SaveStatus
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

/*--------------
 * Timer1 output compare interrupt handler routine
 * Freerun long delay timer
 *--------------*/
public Oc1_drv:     /* Output Compare1 Interrupt Vector */
/*          in      SaveStatus,SREG
            out     SREG,SaveStatus
            reti
*/


/*---------
 * CANBus can use Interrupt0 or Interrupt1
 * Other unused interrupt is here
 *---------*/
#if CANRxInterrupt
public Int0_drv:    ;External Interrupt0 Vector
#else
public Int1_drv:    ;External Interrupt1 Vector
#endif


/*--------------
 * Other unused interrupt vector
 *--------------*/
public Udre_drv:    /* UART Data Register Empty Interrupt Vector */
public Icp1_drv:    /* Input Capture1 Interrupt Vector */
public Aci_drv:     /* Analog Comparator Interrupt Vector */
#ifdef  AT90S4433
public Spi_drv:     /* SPI Interrupt Vector */
public Adc_drv:     /* ADC Interrupt Vector */
public EErdy_drv:   /* EEProm ready Interrupt Vector */
#endif
           reti


/**========================**
 **  EEprom                **
 **========================**/
        seg    eeprom
e2_pad1:         ds.b  1   /* first byte reserved for the AVR eeprom brown-out bug */

   /* We don't need filters because CANDONGLE has to transmit everything
    * ID not used by now
    */

e2_Dongle_ID:     dc.b  0x01


        seg flash.code
/*=================================
 * THREAD 2  Event and Action dispatcher
 * EDISPATCHER
 *=================================*/
public Thr2_entry:
            /* put initialization code here */
            clr     sRxPos
            clr     sRxMsgLen
            clr     sTxPos
            clr     sRxMode
            ldi     sTxMsgLen,0xff
        
            ldi     r31,0x90    /* 10010000 Comparator Disabled
                                   1XXXXXXX Comparator disable, 1=off
                                   XXX1XXXX Int flag, writing 1 clears it */
            out     ACSR,r31

            /* UART initialization */
            ldi     r31,0x0D8   /* 1XXXXXXX RX complete int enable
                                   X1XXXXXX TX complete int enable
                                   XX0XXXXX DR Empty int enable 
                                   XXX1XXXX RX Enable
                                   XXXX1XXX TX Enable
                                   XXXXX0XX 9 bit chars 1=enable
                                   XXXXXX0X RX DB8
                                   XXXXXXX0 TX DB8 */
            out     UCR,r31
            ldi     r31,((XTAL_FREQUENCY * 10) / (16 * BAUDRATE) + 5) / 10 - 1
            out     UBRR,r31    /* Set the baud rate (38400) */
            sbi     UART_DIR_PORT, SerCTS

            /* Trace port initialization */
            ldi     Arg1,BV(TraceRxBitP)|BV(TraceInt0P)|BV(TraceBitErrP)|BV(TraceStuffErrP)|BV(TraceFormErrP)
            out     TRACE_DIR_PORT,Arg1

            clr		Arg1
            out     TRACE_OUT_PORT,Arg1


            /* Led port initialization */
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


/* Action:  Send a Message over serial Line */
/* Boot Message: Own ID from EEPROM */
/**
            ldi     ZL,low(sRxMsgBuf)       ;Arg2 = ZL (r30)
            ldi     r31,e2_Dongle_ID 
            rcall   EEReadByte
            st      Z+,r31
            ldi     r31,2
            st      Z+,r31
            ldi     r31,'V'
            st      Z+,r31
            ldi     r31,'1'
            st      Z+,r31

            ldi     Arg2,EDISPATCHER
            ldi     Arg1,BV(RSDataSig) 
            rcall   Signal
**/
            /* Wait for CANBus trhead initialized */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait


/* Action:  Send a CAN msg with ID = 0x5500 and LEN = 0 (for testing purpose) */
#ifdef BOOT_MSG
#define    PrID    0x2f
            ldi     ZL,TxId
            ldi     r31,((PrID<<5) >> 8)
            st      Z+,r31          /* TxId in Arg1 */
            ldi     r31,((PrID<<5) & 0xFF)
            ori     r31,2
            st      Z+,r31          /* TxLen in Arg4 */

            ldi     r31,0x5c
            st      Z+,r31
            ldi     r31,0x33
            st      Z+,r31

            rcall   CanTxSend
#endif


            /* Main LOOP */
thr_loop:   
            /* We wait for a signal from RS232, CANBus */
            ldi     Arg1,BV(RSDataSig)|BV(CanDataSig)|BV(CanErrSig)
            rcall   Wait
            mov     Reg2,Res1

            sbrs    Reg2,RSDataSig
            rjmp    can_test
            /* Message over RS232 received - copy to CAN and send signal */

            /* Test, if CAN-TxBuffer is free HOWTO    ???? */
            /* Copy to CAN-Buffer */
            ldi     Reg4,10             ;Reg4 = XH (r27)
            ldi     XL,low(sRxMsgBuf)   ;Reg3 = XL (r26)
            ldi     ZL,TxId             ;Arg2 = ZL (r30)
copy2CANTx: ld      Arg1,X+
            st      Z+,Arg1          /* TxId in Arg1 */
            dec     Reg4
            brne    copy2CANTx


/* set MSGLEN ?*/
#define RS232_LOOPno
#ifdef RS232_LOOP
            sbr     UserBits,(1<<TxLockBit) /* Message ready in CAN Tx buffer */
            ldi     Arg2,EDISPATCHER
            ldi     Arg1,BV(CanDataSig)
            rcall   Signal
#else
            rcall   CanTxSend               ;Send message ready in CAN Tx buffer
#endif
            cbi     UART_OUT_PORT, SerCTS    /* I'm ready for next msg */

can_test:   sbrs    Reg2,CanDataSig
            rjmp    can_err_test

            /* Event:   CAN message received
             * Action:  copy to RS232Tx and start transmitting
             */

            /* no Test, if RS232-TxBuffer is free - Receiver has to be fast...*/
            /* Copy to RS232-Buffer */
#ifdef RS232_LOOP
            ldi     XL,TxId
#else
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
            ldi     XL,RxId_1         ;Reg3 = XL (r26)
            rjmp    ct_copyloc

ct_buf2check:
            sbrs    UserBits,RxLockBit2
            rjmp    can_err_test

            sbr     RxBufIdx,(1<<PRI_EDISPATCHER)
            ldi     XL,RxId_2         ;Reg3 = XL (r26)
#else
            sbrs    UserBits,RxLockBit1
            rjmp    can_err_test
            ldi     XL,RxId_1         ;Reg3 = XL (r26)
#endif
ct_copyloc:
#endif
            ldi     ZL,low(sTxMsgBuf)
            ldi     Reg4,10           ;Reg4 = XH (r27)

copy2RSTx:  ld      Arg1,X+
            st      Z+,Arg1
            cpi     Reg4,9          /* 2nd byte: MsgLen */
            brne    RSTnext
            andi    Arg1, 0x0f
            inc     Arg1           /* to INCLUDE Header in MsgLen */
            mov     sTxMsgLen, Arg1
RSTnext:    dec     Reg4
            brne    copy2RSTx
            clr     sTxPos
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
            ldi     Arg1,'C'
            out     UDR,Arg1        /* first Byte out */
#ifdef  LedBusRxP
            sbi     LED_OUT_PORT,LedBusRxP      ;switch CAN-bus Rx led off
#endif

can_err_test:
            sbrs    Reg2,CanErrSig
            rjmp    thr_loop

            /* Event:   CAN Error received
             * Action:  copy Errcounter to  RS232Tx 
             * (transmitting has started already - with errcode in canbus thread)
             */
            ldi     ZL,low(sTxMsgBuf)
            
            st      Z+,RxErrcounter
            st      Z+,TxErrcounter
            st      Z+,CanBits
#ifdef CanErrCode
            st      Z+,CanErrCode
            ldi     sTxMsgLen,4-1
#else
            ldi     sTxMsgLen,3-1
#endif
            clr     sTxPos
/** No need to Clear RxLockBit since we don't access the RxBuf for a received msg

            ;Clear RxLockBit  (signal the end of processing to CANBus thread: buffer available)
            sbrs    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit1)
            sbrc    RxBufIdx,PRI_EDISPATCHER
            cbr     UserBits,(1<<RxLockBit2)
**/
            ldi     Arg1,'E'
            out     UDR,Arg1        /* first Byte out */

            rjmp    thr_loop
