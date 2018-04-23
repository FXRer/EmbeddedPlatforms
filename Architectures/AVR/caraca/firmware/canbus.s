/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2002 by Claudio Lanconelli
 * e-mail: lancos@libero.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- CAN bus software driver ---*
 * $Id: canbus.s,v 1.15 2002/01/04 18:05:30 lancos Exp $

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
#include "canbus.h"

#include "kernel.e"
#include "eeprom.e"

/** CAN Bus **/

/*----------------
 * Tx nbits 1s\
 *----------------*/
#define CANTxRecessive(nbits)\
          set\
loop@:\
          rcall CANTxBit\
          brcs  stop@\
          dec   nbits\
          brne  loop@\
stop@:

/*---------------- 
 * Tx nbits 0s 
 *----------------*/ 
#define CANTxDominant(nbits)\
          clt\
loop@:\
          rcall    CANTxBit\
          dec      nbits\
          brne  loop@

/*--------
 * Wait for nbits consecutive recessive bits
 * use reg as scratch register
 * Returns T = 1 if we got it, else T = 0
 *--------*/
#define    WaitRecessiveBits(reg,nbits)\
reload@:    ldi     reg,nbits\
            sbr     CanBits,(1<<HardSync)\
loop@:      rcall   CANRxBit\
            brtc    reload@\
            dec     reg\
            brne    loop@\

/*--------
 * End of Frame, wait for 7 consecutive recessive bits
 *--------*/
#define EndOfFrame(reg)\
            ldi     reg,7\
loop@:      rcall   CANRxBit\
            brtc    CANFormError\
            dec     reg\
            brne    loop@\


/*-------
 * Decrement TxErrorCounter by 1
 *-------*/
#define DecTxError\
            tst     TxErrcounter\
            breq    dterr_ok@\
            dec     TxErrcounter\
dterr_ok@:\


/*-------
 * Decrement RxErrorCounter by 1
 *-------*/
#define DecRxError(reg)\
            tst     RxErrcounter\
            breq    drerr_tst@\
            dec     RxErrcounter\
            cpi     RxErrcounter,128\
            brcc    drerr_ok@\
drerr_tst@: ldi     reg,128\
            cp      TxErrcounter,reg\
            brcc    drerr_ok@\
            cbr     CanBits,(1<<ErrorPassive)\
drerr_ok@:\


/*-----------------
 * InitCRC()
 *-----------------*/
#define InitCRC\
            clr     CrcRegL\
            clr     CrcRegH


/*******************************************************
 * UTILITIES ROUTINES
 *******************************************************/

/********
 * Send a CANBus frame up to 4 data bytes
 * result = CanTxFrame4(Arg1, Arg2, Arg3, Arg4, Reg1, Reg2)
 * Arg1
 * (0 if success, FF if error)
 *
 * Arg1:Arg2 store the MsgID+RTR+DLC packet in a word (see above)
 * Arg3 store first data byte to send
 * Arg4 store second data byte to send
 * Reg1 store third data byte to send
 * Reg2 store fourth data byte to send
 *
 * Used Regs: Arg1, Arg2, Arg3
 * Arg4, Reg1, Reg2 are preserved
 ********/
        seg removable flash.code.CanTxFrame4
public CanTxFrame4:
            /* First check if the buffer is available */
            sbrc    UserBits,TxLockBit
            rjmp    txfrm_err
txfrm_start:
#ifdef  LedBusTxP
            cbi     LED_OUT_PORT,LedBusTxP    ;switch CAN-bus Rx led on
#endif
            push    Reg3
            ldi     Reg3,TxId       /* XL (Reg3 = r26) */
            st      X+,Arg1         /* TxId (8 msb) */
            st      X+,Arg2         /* TxId (3 lsb) + RTR + TxLen */

            st      X+,Arg3         /* Data bytes */
            st      X+,Arg4
            st      X+,Reg1
            st      X+,Reg2
            pop     Reg3

            sbr     UserBits,(1<<TxLockBit)

            ldi     Arg2,CANBUSDRV
            ldi     Arg1,(1<<CanReqSig)
            rjmp    Signal          /* Arg1 = 0 */
txfrm_err:
            ser     Arg1
            ret

/********
 * Send a CANBus frame from EEProm up to 3 data bytes
 * result = CanTxEEFrame3Queue(Arg1)
 * Res1
 * (0 if success, FF if error)
 *
 * Arg1 store the eeprom location from which read the message
 * MsgID+RTR+DLC word
 * Data byte 1
 * Data byte 2
 * Data byte 3
 *
 * Used Regs: Arg1, Arg2, Arg3, Arg4
 ********/
        seg removable flash.code.CanTxEEFrame3Queue
public CanTxEEFrame3Queue:
#if TX_QUEUE==1
            /* First check if the buffer is available */
            sbrs    UserBits,TxLockBit
            rjmp    txef3_start
            tst     EqueueIDptr
            brne    txef3_err       /* queue not empty: discard event! (we'll need to trace it) */
txef3_queue:
            mov     EqueueIDptr,Arg1
            mov     Arg2,Arg1
            addi(Arg2,2)
            rcall   EEReadByte
            mov     Equeue1,Arg1

            inc     Arg2
            mov     Arg1,Arg2
            rcall   EEReadByte
            mov     Equeue2,Arg1

            inc     Arg2
            mov     Arg1,Arg2
            rcall   EEReadByte
            mov     Equeue3,Arg1
            clr     Res1
            ret
#else
            /* First check if the buffer is available */
            sbrc    UserBits,TxLockBit
            rjmp    txef3_err
#endif
txef3_start:
#ifdef  LedBusTxP
            cbi     LED_OUT_PORT,LedBusTxP    ;switch CAN-bus Rx led on
#endif
            mov     Arg3,Arg1
            ldi     Arg4,TxId       /* YL (Arg4 = r28) */

            rcall   EEReadWord
            st      Y+,Arg1         ;ID+Len
            st      Y+,Arg2

            addi(Arg3,2)
            mov     Arg1,Arg3
            rcall   EEReadByte
            st      Y+,Arg1         ; Data bytes

            inc     Arg3
            mov     Arg1,Arg3
            rcall   EEReadByte
            st      Y+,Arg1

            inc     Arg3
            mov     Arg1,Arg3
            rcall   EEReadByte
            st      Y+,Arg1
txef3_ok:
            sbr     UserBits,(1<<TxLockBit)

            ldi     Arg2,CANBUSDRV
            ldi     Arg1,(1<<CanReqSig)
            rjmp    Signal          /* Arg1 = 0 */
txef3_err:
            ser     Res1
            ret


/********
 * Send a CANBus frame up to 3 data bytes
 * result = CanTxFrame3Queue(Arg1, Arg3, Arg4, Reg1)
 * Res1
 * (0 if success, FF if error)
 *
 * Arg1 store the eeprom location from which read the MsgID+RTR+DLC word
 * Arg3 store first data byte to send
 * Arg4 store second data byte to send
 * Reg1 store third data byte to send
 *
 * Used Regs: Arg1, Arg2, Arg3
 * Arg4, Reg1, Reg2 are preserved
 ********/
        seg removable flash.code.CanTxFrame3Queue
public CanTxFrame3Queue:
#if TX_QUEUE==1
            /* First check if the buffer is available */
            sbrs    UserBits,TxLockBit
            rjmp    txf3_start
            tst     EqueueIDptr
            brne    txf3_err       /* queue not empty: discard event! (we'll need to trace it) */
txf3_queue:
            mov     EqueueIDptr,Arg1
            mov     Equeue1,Arg3
            mov     Equeue2,Arg4
            mov     Equeue3,Reg1
            clr     Res1
            ret
#else
            /* First check if the buffer is available */
            sbrc    UserBits,TxLockBit
            rjmp    txf3_err
#endif
txf3_start:
#ifdef  LedBusTxP
            cbi     LED_OUT_PORT,LedBusTxP    ;switch CAN-bus Rx led on
#endif
            ldi     Arg2,TxBuf      /* ZL (Arg2 = r30) */
            st      Z+,Arg3         /* Data bytes */
            st      Z+,Arg4
            st      Z+,Reg1
txf3_ok:
            rcall   EEReadWord
            sts     TxId,Arg1       /* TxId (8 msb) */
            sts     TxId+1,Arg2     /* TxId (3 lsb) + RTR + TxLen */

            sbr     UserBits,(1<<TxLockBit)

            ldi     Arg2,CANBUSDRV
            ldi     Arg1,(1<<CanReqSig)
            rjmp    Signal          /* Arg1 = 0 */
txf3_err:
            ser     Res1
            ret

/********
 * Commit a CANBus msg for transmission
 * The msg is prepared in the TxBuffer
 *
 * Used Regs: Arg1, Arg2, Arg3
 ********/
        seg removable flash.code.CanTxSend
public CanTxSend:
#ifdef  LedBusTxP
            cbi     LED_OUT_PORT,LedBusTxP    ;switch CAN-bus Tx led on
#endif
            sbr     UserBits,(1<<TxLockBit)
            ldi     Arg2,CANBUSDRV
            ldi     Arg1,(1<<CanReqSig)
            rjmp    Signal         /* return to caller, don't waste stack */


/*******************************************************
 *******************************************************/

        seg    eram.data
public TxId:    ds.b    1               ;TxMsg Id (8 msb)
public TxLen:   ds.b    1               ;number of bytes to transmit
public TxBuf:   ds.b    MAX_CANMSG_SIZE    ;Tx buffer

public RxId_1:  ds.b    1               ;RxMsg Id (8 mbs)
public RxLen_1: ds.b    1               ;number of received bytes
public RxBuf_1: ds.b    MAX_CANMSG_SIZE    ;Rx buffer

#if RX_DOUBLE_BUFFER==1
public RxId_2:  ds.b    1
public RxLen_2: ds.b    1
public RxBuf_2: ds.b    MAX_CANMSG_SIZE
#endif
/** Note that msgID (11 bits) and msgLen (4 bits) are packed together
  * in TxId + TxLen and RxId + RxLen
  * 15   14   13  12  11  10   9   8   7   6   5   4  3  2  1  0
  * Id11 Id10 Id9 Id8 Id7 Id6 Id5 Id4 Id3 Id2 Id1  R N4 N3 N2 N1
  * +------------- TxId ------------++--------- TxLen ---------+
  * R is 1 for RemoteFrame
  **/
#define RemoteFrameBIT    4

/*******************************************************
 * ROUTINES
 *******************************************************/
        seg flash.code

/*---------------
 * External interrupt handler routine
 * This interrupt is activated by the CAN Rx line (falling edge),
 *   it can means a SOF (hard resync) or a soft resync
 *---------------*/
#if CANRxInterrupt==0
public Int0_drv:
#else
public Int1_drv:
#endif
            in      SaveStatus,SREG

#if TRACERX_BIT==1
            sbi     TRACE_OUT_PORT,TraceInt0P
#endif
#ifdef CANDONGLE_DEBUG_TIMING
            in      IntTmp,TCNT1L
            out     UDR,IntTmp
            in      IntTmp,MCUCR
            cpi     IntTmp,0x23
            breq    rising
            ldi     IntTmp,(1<<bSE)|(0<<bSM) | (0<<bISC11)|(0<<bISC10) | (1<<bISC01)|(1<<bISC00)
            out     MCUCR,IntTmp
            ;out     UDR,IntTmp
#endif
            sbrc    CanBits,HardSync
            rjmp    hardsync0

            sbrc    CanBits,CanMaster
            rjmp    int0drvok

softsync0:  /* soft syncronization (resyncronization on falling edge) */
#if SOFT_RESYNC==1
            in      IntTmp1,TCNT0
            in      IntTmp2,TCNT0

            /* where is located the edge respect to Sync_Seg? */
            ;Phase_Err = EDGE(IntTmp1) - Sync_Seg

            ;If Phase_Err > 0 lengthen Phase1_Seg
            cpi     IntTmp1,EDGE2     ;EDGE2 = EndOf Sync_Seg
            brcs    ss_Test2
#if TRACERESYNC==1
            sbi     TRACE_OUT_PORT,TraceResyncPos
            cbi     TRACE_OUT_PORT,TraceResyncPos
#endif
            ;Phase_Err = IntTmp1 - EDGE2
            subi    IntTmp1,EDGE2

            ;Correct_Value = Phase_Err > RJW ? RJW : Phase_Err
            cpi     IntTmp1,RJW
            brcs    ss_Loc1
            ldi     IntTmp1,RJW
ss_Loc1:    ;lengthen Phase1_Seg == reload the timer with a smaller value
            sub     IntTmp2,IntTmp
            rjmp    ss_GoResync

ss_Test2:   ;If Phase_Err < 0 shorten Phase2_Seg
            cpi     IntTmp1,EDGE1     ;EDGE1 = StartOf Sync_Seg
            brcc    ss_NoReSync
#if TRACERESYNC==1
            sbi     TRACE_OUT_PORT,TraceResyncNeg
            cbi     TRACE_OUT_PORT,TraceResyncNeg
#endif
            ;Phase_Err = EDGE1 - IntTmp1
            subi    IntTmp1,EDGE1
            neg     IntTmp1

            ;Correct_Value = Phase_Err > RJW ? RJW : Phase_Err
            cpi     IntTmp1,RJW
            brcs    ss_Loc2
            ldi     IntTmp1,RJW
ss_Loc2:    ;shorten Phase2_Seg == reload the timer with a greater value
            add     IntTmp2,IntTmp1
ss_GoResync:
            out     TCNT0,IntTmp2
ss_NoReSync:
#endif
            rjmp    int0drvok

hardsync0:  /* hard syncronization (Start Of Frame) */
            ldi     IntTmp1,TIM0_SYNC+1
            out     TCNT0,IntTmp1
            cbi     CAN_OUT_PORT,CanSleepP   /* activate CAN driver */
            cbr     CanBits,(1<<HardSync)    /* next time perform a soft sync */

            push    ZL                  /* ZL = Arg2 = r30 */

            ldi     ZL,CANBUSDRV        /* destination thread */
            ldi     IntTmp1,(1<<CanSOFSig)  /* signal mask */
            IsrSignal(IntTmp1,Z,IntTmp2)

            /* Reset Timer0Sig signal, that was pending and I always got 
               the 1, that belongs to IFS (Interframe space)*/
            ldi     ZL,CANBUSDRV
            IsrFlushSignal(((1<<Timer0Sig)),Z,IntTmp1)

            pop     ZL

int0drvok:  
#if TRACERX_BIT==1
            cbi     TRACE_OUT_PORT,TraceInt0P
#endif
            out     SREG,SaveStatus
            reti
#ifdef CANDONGLE_DEBUG_TIMING
rising:     
            ldi     IntTmp,(1<<bSE)|(0<<bSM) | (0<<bISC11)|(0<<bISC10) | (1<<bISC01)|(0<<bISC00)
            out     MCUCR,IntTmp
            ;out     UDR,IntTmp
            rjmp    int0drvok
#endif


/*---------
 *---------*/
        seg removable flash.code.Thread1
CANRxError:
#if TRACERXERR==1
            sbi     TRACE_OUT_PORT,TraceRxErrP
#endif
            rjmp    OverloadFrame


/*=================================
 * THREAD 1  CANBus driver
 * CANBUSDRV
 *=================================*/
public Thr1_entry:
            /* put initialization code here */
            sbi     CAN_TXDIR_PORT,CanTxP
            cbi     CAN_RXDIR_PORT,CanRxP
            sbi     CAN_DIR_PORT,CanSleepP

            /* We keep the 82C250 always active, when we've tested
             * all the can code we will switch on the 82C250 only
             * before transmitting (power save mode)
             */
            cbi     CAN_OUT_PORT,CanSleepP
#if TX_QUEUE==1
            clr     EqueueIDptr
#endif

            /* Mark TxBuffer and RxBuffer available */
            /* These bits are used to avoid race conditions for
             * CAN Tx and Rx buffers.
             * Writing threads write the buffer only if LockBit is cleared,
             * after writing it raise LockBit.
             * Reading threads wait for LockBit is raised,
             * after reading it clear the LockBit.
             * CANBus thread is a "writing thread" for RxBuffer and
             * a "reading thread" for TxBuffer
             */
#if RX_DOUBLE_BUFFER==1
            cbr     UserBits,(1<<RxLockBit1)|(1<<RxLockBit2)|(1<<TxLockBit)
#else
            cbr     UserBits,(1<<RxLockBit1)|(1<<TxLockBit)
#endif
            /* Start in Bus Off state */
            rcall   CANBusStartUp

            /* Signal that we are now ready to accept request */
            ldi     Arg2,EDISPATCHER
            ldi     Arg1,(1<<CanReadySig)
            rcall   Signal
#ifdef  RC5DECODER
            ldi     Arg2,RC5DECODER
            ldi     Arg1,(1<<CanReadySig)
            rcall   Signal
#endif
#ifdef  THERMOMETER
            ldi     Arg2,THERMOMETER
            ldi     Arg1,(1<<CanReadySig)
            rcall   Signal
#endif

thr_loop:   /* wait for a transmission request or a start of frame */
            ldi     Arg1,(1<<CanReqSig)|(1<<CanSOFSig)
            rcall   Wait

            /* we need to check the SOF first to avoid any bus conflict */
test_sof:   sbrs    Res1,CanSOFSig
            rjmp    test_req

/*------------------------ Here we receive a new message ----------------------------------*/
recv_frame:
            /*========= Start of frame signal: start CAN msg receiving ========*/
            InitCRC
            sbr     CanBits,(1<<txBitstuffing)|(1<<lastTxbit)          /* enable and init bit stuffing */

            /* Start of frame */
            rcall   CANRxBit
            brts    CANRxError

            /* If we need to transmit a msg start arbitration of the bus */
            sbrc    UserBits,TxLockBit
            rjmp    sf_arb

recv_frame_sof:                /* we jump here if we got SOF while receiving last intermission bit */
            /* Arbitration frame */
            ldi     Arg4,11+1         /* len of ID + RTR bit */
            rcall   CANRxWord
            brcs    CANStuffErr1
LostArbFrame_entry:         /* we jump here if we lose arbitration during a msg transmission */
            /* Control field */
            rcall   CANRxBit            /* reserved bit (0) */
/*            brts    ExtFrame   */     /* Extended frame */
            rcall   CANRxBit            /* reserved bit (0) */
/*            brts    CANFormErr1  ** must accept either a 0 or 1 */
            rcall   CANRxBit            /* DLC3  */
            brcs    CANStuffErr1
            rcall   CANRxBit            /* DLC2 (now ID is shift to the right pos) */
            brcs    CANStuffErr1

#if RX_DOUBLE_BUFFER==1
            /* Use the first free RxBuffer (Check RxBuf1 first, then RxBuf2)
             * Note that if both RxBuffers are still locked it overwrite the
             * RxBuffer2 because CanBusThread need to store received msg somewhere!!
             */
            cbr     RxBufIdx,(1<<PRI_CANBUSDRV)
            sbrc    UserBits,RxLockBit1
            sbr     RxBufIdx,(1<<PRI_CANBUSDRV)

            ldi     Reg3,RxId_1         /* XL alias r26(Reg3) */
            sbrc    RxBufIdx,PRI_CANBUSDRV
            ldi     Reg3,RxId_2         /* XL alias r26(Reg3) */
#else
            ldi     Reg3,RxId_1
#endif
            st      X+,CanRxWordH
#ifdef CANDONGLE_DEBUG_SLOW
            out     UDR,CanRxWordH
#endif
            mov     Reg4,CanRxWordL
            andi    Reg4,0xf0


            rcall   CANRxBit            /* DLC1  */
            rcall   CANRxBit            /* DLC0 */
            brcs    CANStuffErr1
            mov     Reg1,CanRxWordL
            andi    Reg1,0x0f

            or      Reg4,Reg1
            st      X+,Reg4
#ifdef CANDONGLE_DEBUG_SLOW
            out     UDR,Reg4
#endif

            /** Data field **/
            sbrc    Reg4,4              /* test for Remote frame: no data byte */
            rjmp    rx_crcfield
            tst     Reg1                /* test for no data byte */
            breq    rx_crcfield
rx_dataloop:
            ldi     Arg4,8
            rcall   CANRxWord
            brcs    CANStuffError
            st      X+,CanRxWordL
#ifdef CANDONGLE_DEBUG_SLOW
            out     UDR,CanRxWordL
#endif
            dec     Reg1
            brne    rx_dataloop

            /* Note that we still calculate CRC, even on CRC field,
             * so if all is ok, the calculated CRC should be 0 */
rx_crcfield:
            /** CRC field **/
            ldi     Arg4,15
            rcall   CANRxWord
CANStuffErr1:
            brcs    CANStuffError
            ldi     Arg1,0x7f
            and     CrcRegH,Arg1

            cbr     CanBits,(1<<txBitstuffing)   /* disable bit stuffing */

            /** CRC check **/
            clt     /* ACK ok */

            /** If there was a CRC error the Error flag starts after ACK field **/
            clr     Arg1
            cp      CrcRegL,Arg1    /* calculated CRC now should be 0 if all bits received OK */
            cpc     CrcRegH,Arg1
            breq    rx_ackok
            set     /* no ACK */
rx_ackok:   bld     Reg2,0              /* Reg2:0 store CRC check result */

            /** CRC Delimiter **/
            rcall   CANRxBit
CANFormErr1:
            brtc    CANFormError

            /** Ack field **/
            bst     Reg2,0              /* ACK only if CRC ok */
            rcall   CANTxBit

            /** Ack Delimiter **/
            rcall   CANRxBit            ;CANTxBit??
            brtc    CANFormError

            sbrc    Reg2,0
            rjmp    CANCrcError    /* CANCrcError */

#ifdef  LedBusRxP
            cbi     LED_OUT_PORT,LedBusRxP    ;switch CAN-bus Rx led on
#endif
            /** End of frame **/
            EndOfFrame(Arg4)
            DecRxError(Arg4)                   /* Received OK: update error counter */

            /* Lock Rx Buffer */
#if RX_DOUBLE_BUFFER==1
            sbrs    RxBufIdx,PRI_CANBUSDRV
            sbr     UserBits,(1<<RxLockBit1)
            sbrc    RxBufIdx,PRI_CANBUSDRV
            sbr     UserBits,(1<<RxLockBit2)
#else
            sbr     UserBits,(1<<RxLockBit1)
#endif
            /* Signal the new received CAN msg to EDISPATCHER thread */
            ldi     Arg2,EDISPATCHER
            ldi     Arg1,(1<<CanDataSig)
            rcall   Signal

InterFrame: sbr     CanBits,(1<<HardSync)       /* perform Hard syncronization */

            /** Intermission (can be interrupted only by overload frame) **/
            /* First intermission bit */
            rcall   CANRxBit
            brtc    OverloadFrame
            /* Second intermission bit */
            rcall   CANRxBit
            brtc    OverloadFrame
            /* Third intermission bit: if we got a dominant bit here we
             * treat it as the SOF (CAN specification 1.2) */
            rcall   CANRxBit
            brts    if_ok
frame_sof1: rjmp    recv_frame_sof
if_ok:
            /* error passive node have to wait further 8 bits before
             * transmit another frame */
            sbrs    CanBits,ErrorPassive
            rjmp    test_req

			ldi		Arg4,8
pas8frame:
			rcall	CANRxBit
			brtc	frame_sof1
			dec		Arg4
			brne	pas8frame

            rjmp    test_req

/*----------
 * Bit Stuff Error
 *----------*/
CANStuffError:
#if TRACEBITERR==1
            sbi     TRACE_OUT_PORT,TraceStuffErrP
#endif
#ifdef CanErrCode
            ldi     Arg1,'S'
            mov     CanErrCode,Arg1
#endif
            rjmp    CANErrout

/*----------
 * Form Error
 *----------*/
CANFormError:
#if TRACEBITERR==1
            sbi     TRACE_OUT_PORT,TraceFormErrP
#endif
#ifdef CanErrCode
            ldi     Arg1,'F'
            mov     CanErrCode,Arg1
#endif
            rjmp    CANErrout

/*----------
 * CRC Error
 *----------*/
CANCrcError:
#if TRACEBITERR==1
            sbi     TRACE_OUT_PORT,TraceCrcErrP
#endif
#ifdef CanErrCode
            ldi     Arg1,'C'
            mov     CanErrCode,Arg1
#endif
            rjmp    CANErrout

/*----------
 * Bit error
 *----------*/
CANBitError:
#if TRACEBITERR==1
            sbi     TRACE_OUT_PORT,TraceBitErrP
#endif
#ifdef CanErrCode
            ldi     Arg1,'B'
            mov     CanErrCode,Arg1
CANErrout:
            /* Signal the offending CAN msg to EDISPATCHER thread*/
            ldi     Arg2,EDISPATCHER
            ldi     Arg1,(1<<CanErrSig)
            rcall   Signal
#else
CANErrout:
#endif
            /* Update Error counters */
            ldi     Arg1,1
            sbrs    CanBits,CanMaster
            rjmp    berr_loc1

            ldi     Arg1,8
            rcall   IncTxError
            brcc    CANErrNocount
            rjmp    InterFrame
berr_loc1:
            ldi     Arg1,1
            rcall   IncRxError
CANErrNocount:
            rcall   CANTxError

            /* Don't clear TxLockBit so we retry to transmit the msg */
            rjmp    InterFrame

            /* Overload frames are not supported (just ignored) */
OverloadFrame:
            rcall   CANBusResync

/*------------------------ Here we check for a message to transmit --------------------------*/
test_req:
            /*========= Check for a msg to be transmitted ===========*/
            sbrs    UserBits,TxLockBit
            rjmp    thr_loop

send_frame:
            /*========= CAN Transmission Request ===========*/

            cbi     CAN_OUT_PORT,CanSleepP      /* activate CAN driver */

            /* Enable and init bitstuffing, perform syncronization on SOF, trasmitter mode */
            sbr     CanBits,(1<<txBitstuffing)|(1<<lastTxbit)|(1<<HardSync)

            /* We wait for a bittime before to start the frame. This delay
             * permits recovery from Sleep mode of PCA82C250. However
             * if someone other start transmit now skip the SOF
             */
            InitCRC             ;we need to InitCRC because SOF update the CRC
            rcall   CANRxBit
            brtc    sf_arb

            InitCRC

            /* Start of frame */
            clt
            rcall   CANTxBit
CANFormError1:
            brcs    CANFormError
sf_arb:
            sbr     CanBits,(1<<CanMaster)  /* Master transmitter mode */

            ldi     XL,TxId         /* XL == Reg3 */
            ld      Reg1,X+         /* TxId in Reg1 */
            ld      Reg4,X+         /* TxLen in Reg4 */

            mov     Reg2,Reg4
            andi    Reg4,0x0F

            /* Reg1-Reg2 --> ID, Reg4 --> Buflen (1-8), Reg3 (XL) --> MsgBuffer */

            /***
             * Arbitration frame
             * Note! ID is a 16 bit constant, but the 4 LSBits are "don't care" (standart CAN ID are 11bits + 1 RTR bit)
             * Ex. ID 0x135 is :  0010 0110 101R XXXX (0x135 << 5)
             * R is RTR bit: 0 for DATA frame, 1 for REMOTE frame
             ***/
            ldi     Arg4,11+1         /* len of ID + RTR bit */
            rcall   CANTxWordArb

            /* CANTxWordArb returns error (Carry set) because
             * of a Bit error (tx 0, rx 1).
             * In the case of arbitration lost it jump to RxWord,
             * if RxWord is ok the carry is cleared, otherwise
             * there was another error (stuff error?) and RxWord
             * return Carry set.
             */
            brcs    CANStuffError   /* Need to check for "exception 2"?? */

            /* After the arbitration if we are still master it means that
             * we won, otherwise we have to jump in receiving mode as soon
             * as possible */
#if TRACELOSTARB==1
			sbrc	CanBits,CanMaster
			rjmp	WinArbJmp
            sbi     TRACE_OUT_PORT,TraceLostArbB
			rjmp	LostArbFrame_entry
WinArbJmp:
#else
            sbrs    CanBits,CanMaster
            rjmp    LostArbFrame_entry
#endif

            /* Control field */
            clt                     /* reserved bit */
            rcall   CANTxBit
CANBitError1:
            brcs    CANBitError
            clt                     /* reserved bit */
            rcall   CANTxBit
            brcs    CANBitError
            mov     Reg1,Reg4       /* N bytes to xmit */
            swap    Reg1
            ldi     Arg4,4          /* len of the field */
            rcall   CANTxWord
            brcs    CANBitError

            /* Remote Frame have no data bytes, even if DLC is different from 0.
             * To check RTR bit we look at CANRxWord that store always last 16 bits
             * transmitted.
             * Now CanRxWordL is: ID0 RTR IDE R0 DLC3 DLC2 DLC1 DLC0
             *                bit  7   6   5   4   3   2    1    0
             * so we have to test bit 6.
             */
            sbrc    CanRxWordL,6
            rjmp    crc_field

            /** Data field **/
            tst     Reg4
            breq    crc_field
data_loop:
            ld      Reg1,X+
            ldi     Arg4,8
            rcall   CANTxWord
            brcs    CANBitError
            dec     Reg4
            brne    data_loop
crc_field:
            /** CRC field **/
            mov     Reg2,CrcRegL
            mov     Reg1,CrcRegH
            lsl     Reg2
            rol     Reg1
            ldi     Arg4,15
            rcall   CANTxWord
            brcs    CANBitError1
            cbr     CanBits,(1<<txBitstuffing)          /* disable bit stuffing */

            /** CRC Delimiter **/
            set
            rcall   CANTxBit
            brcs    CANFormError1

            /** Ack field **/
            rcall   CANRxBit
            brts    CANnoAck

            /** Ack Delimiter **/
            set
            rcall   CANTxBit
            brcs    CANFormError1

            /** End of frame **/
            ldi     Arg4,7
sf_eofloop: rcall   CANRxBit
            brtc    CANFormError2
            dec     Arg4
            brne    sf_eofloop

            cbr     CanBits,(1<<CanMaster)          /* switch to slave mode */
            sbr     CanBits,(1<<HardSync)           /* wait for a SOF (hard sync) */

            /** Frame transmitted succesfully **/
            DecTxError
#if TX_QUEUE==1
            /** Check for an Event in the queue, if the queue is empty
              * clear TxLockBit, otherwise copy the Event from the queue
              * to the TxBuffer, clear the EventQueue and DON'T clear
              * the TxLockBit so we transmit the new Event after intermission
              **/
            tst     EqueueIDptr
            breq    sf_unlocktxbuf

            mov     Arg1,EqueueIDptr
            rcall   EEReadWord
            ldi     YL,TxId             ;YL = Arg4 = r28
            st      Y+,Arg1
            st      Y+,Arg2
            st      Y+,Equeue1
            st      Y+,Equeue2
            st      Y+,Equeue3
            clr     EqueueIDptr
            rjmp    InterFrame
#endif
sf_unlocktxbuf:
#ifdef  LedBusTxP
            sbi     LED_OUT_PORT,LedBusTxP    ;switch CAN-bus Rx led off
#endif
            /* We don't need to lock the buffer anymore */
            cbr     UserBits,(1<<TxLockBit)
            rjmp    InterFrame                      /* Send intermission */

CANFormError2:
            rjmp    CANFormError

/*----------
 * Bit Error while sending error flag
 *----------*/
DoubleError:
            /* Update Error counters */
            ldi     Arg1,8
            sbrs    CanBits,CanMaster
            rjmp    dbe_rx

            rcall   IncTxError
            brcc    dbe_err
            rjmp    InterFrame
dbe_rx:
            rcall   IncRxError

dbe_err:    rcall   CANTxError
            brcs    DoubleError

            /* Don't clear TxLockBit so we retry to transmit the msg */
            rjmp    InterFrame


/*----------
 * No Acknowledge Error
 *----------*/
CANnoAck:
/*            sbr     CanError,(1<<NoAckError)  */

            /* Update Error counters:
             * Error passive transmitter: TEC no change (only if there was no dominant bit during error flag)
             * Error active transmitter: TEC inc by 8
             * Any receiver: REC inc by 1
             */
            ldi     Arg1,1
            sbrs    CanBits,CanMaster
            rjmp    loc_inc1            /* Any receiver */
            sbrc    CanBits,ErrorPassive
            rjmp    loc_txerr           /* Error passive transmitter */

            ldi     Arg1,8
            rcall   IncTxError
            brcc    loc_txerr
            cbr     UserBits,(1<<TxLockBit)     ;After bus off don't tx error frame, and don't try to retransmit the msg
            rjmp    InterFrame
loc_inc1:
            rcall   IncRxError
loc_txerr:
            /* Transmit Error flag (depends on passive or active state) */
            rcall   CANTxError
            brcs    DoubleError

            /* Don't clear TxLockBit so we retry to transmit the msg */
#if NOACK_RETRY==1
            cbr     UserBits,(1<<TxLockBit)
#endif
            rjmp    InterFrame

/*---------
 * Transmit the error frame (it depends on the current state:
 * "error active" or "error passive") and then try to
 * transmit again the interrupted frame
 *
 * used regs: r28 (Arg4)
 *   by CANTxBit: r29 (Arg3), r30 (Arg2), r31 (Arg1)
 *---------*/
CANTxError:
            cbr     CanBits,(1<<txBitstuffing)      /* disable bit stuffing */

            sbrs    CanBits,ErrorPassive
            rjmp    test_act

test_pass:  /* Error Passive */
            ldi     Arg4,6      /* Passive Error flag */
            set
txe_loop:   rcall   CANTxBit
            dec     Arg4
            brne    txe_loop

            /* Resync to the bus (wait for 11 consecutive recessive bits) */
            rcall   CANBusResync
            rjmp    TxErrFlag_ok

test_act:   /* Error Active */
            ldi     Arg4,6        /* Active Error flag */
            CANTxDominant(Arg4)

            /* wait for any echo error flag */
            ldi     Arg4,(6+1)         /* 6 bits max */
            set
echo_loop:
            rcall   CANTxBit
            brcc    TxErrFlag
            dec     Arg4
            brne    echo_loop

            /* Error!! More than 12 dominant bits in error frame!! */
            rjmp    BadErrflag

            /* Error delimiter */
TxErrFlag:
            ldi     Arg4,7
txe_loop2:
            rcall   CANRxBit
            brtc    BadErrflag
            dec     Arg4
            brne    txe_loop2

            sbr     CanBits,(1<<HardSync)       /* perform Hard syncronization */
TxErrFlag_ok:
            clc
            ret
BadErrflag:
            sbr     CanBits,(1<<HardSync)       /* perform Hard syncronization */
            sec
            ret

/*-------
 * Increment TxErrorCounter by the value in Arg1
 *
 * Return Carry = 1 if we come back from BusOff state
 *-------*/
IncTxError:
            add     Arg1,TxErrcounter
            brcs    CANBusOff                    /* If TxErrcounter > 255 go Bus off */
            cpi     Arg1,128
            brlo    ite_ok
            sbr     CanBits,(1<<ErrorPassive)   /* switch to ErrorPassive state */
ite_ok:
            mov     TxErrcounter,Arg1
            clc
            ret

/*-------
 * Increment RxErrorCounter by the value in Arg1
 *-------*/
IncRxError:
            add     RxErrcounter,Arg1
            cpi     RxErrcounter,128
            brlo    ire_ok
            ldi     RxErrcounter,128
            sbr     CanBits,(1<<ErrorPassive)   /* switch to ErrorPassive state */
ire_ok:
            ret

/*--------
 * used regs: r28 (Arg4)
 *   by CANRxBit: r29 (Arg3), r30 (Arg2), r31 (Arg1)
 *--------*/
CANBusResync:
            cbr     CanBits,(1<<txBitstuffing)   /* disable bit stuffing */
            WaitRecessiveBits(Arg4,11)

            ldi     ZL,CANBUSDRV
            FlushSignal((1<<CanSOFSig),Z,Arg1)

            sbr     CanBits,(1<<HardSync)       /* perform Hard syncronization */
            ret

/*--------
 * Bus off state: wait for a watchdog reset (to go idle for 128*11 recessive bits and restart)
 *
 * used regs: r31 (Arg1), r30 (Arg2), Arg3
 *--------*/
CANBusOff:
            /* TX pin high during idle state */
            SetCanTx
#if TRACEBUSOFF==1
            sbi     TRACE_OUT_PORT,TraceBusOffP
#endif
            cli     ;disable interrupts
            clr     Arg3
#ifdef  LedDrvP
            ldi     Arg2,(1<<LedDrvP)
#else
            ldi     Arg2,(1<<LedBusTxP)
#endif
busoff_loop:
            dec     Arg3
            brne    busoff_loop

            in      Arg1,LED_OUT_PORT
            eor     Arg1,Arg2
            out     LED_OUT_PORT,Arg1
            
            rjmp    busoff_loop     ;wait for Watchdog reset
            ret


/*----------
 * CANRxBit(), the bit received is stored in T
 * when exit, Carry is 1 if error, 0 if ok
 *
 * used regs: r31 (Arg1)
 *   by Wait: r29, r30 (ZL)
 *   by UpdateCRC: r31
 *----------*/
CANRxBit:
            /* If we transmitted a dominant bit we need to raise CanTx line during the Sync segment */
            sbic    CAN_TXOUT_PORT,CanTxP
            rjmp    rxbit_start
rxbit_wait:
            in      Arg1,TCNT0
            cpi     Arg1,MIDDLE_SWITCH_POINT
            brlo    rxbit_wait          /* wait for the middle of SYNC_SEG to
                                         * switch the line */
            SetCanTx                    /* set the line "recessive" */
rxbit_start:
            cbr     CanBits,(1<<StuffError)

            /* wait for the timer interrupt (sampling point) */
            ldi     Arg1,(1<<Timer0Sig)
            rcall   Wait
LostArbRxBit:
            bst     CanBits,Rxbit        /* copy the received bit to T */
#ifdef CANDONGLE_DEBUG_BITS
            ;in      Arg1,TCNT1L
            clr     Arg1
            bld     Arg1,0
            out     UDR,Arg1
            /* output doesn't contain stuff bits! */ 
#endif
#if TRACERX_BIT==1
            sbi     TRACE_OUT_PORT,TraceRxBitP
            cbi     TRACE_OUT_PORT,TraceRxBitP
            sbrc    CanBits,Rxbit
            sbi     TRACE_OUT_PORT,TraceRxBitP
#endif
            lsl     CanRxWordL
            rol     CanRxWordH
            bld     CanRxWordL,0

            rcall   UpdateCRC

            /* check for bitstuffing enabled on current field */
            sbrs    CanBits,txBitstuffing
            rjmp    RXBIT_OK

            /**
             * Notice that lastTxbit must be the same # bit of T within SREG (6)
             **/
            in      r31,SREG
            eor     r31,CanBits
            andi    r31,(1 << lastTxbit)
            brne    RXBIT_L1

            /* if (T == lastTxbit) */
            inc     TxBitcounter
            cpi     TxBitcounter,4
            brlo    RXBIT_OK

#ifdef CANDONGLE_DEBUG_STUFF
            in      Arg1,TCNT1L
            out     UDR,Arg1
#endif
            /* receive the next bit and check for the bitstuffing */
            /* wait for the timer interrupt (sampling point) */
            ldi     Arg1,(1<<Timer0Sig)
            rcall   Wait
#if TRACERX_BIT==1
            sbi     TRACE_OUT_PORT,TraceRxBitP
            cbi     TRACE_OUT_PORT,TraceRxBitP
            sbrc    CanBits,Rxbit
            sbi     TRACE_OUT_PORT,TraceRxBitP
#endif

#ifdef CANDONGLE_DEBUG_STUFF
            in      Arg1,TCNT1L
            out     UDR,Arg1
#endif
            /* compare sampled bit (Rxbit) with T (last bit received) */
            cpneRegBit(CanBits,Rxbit,BitStuffErr)
            /* Bit stuffing OK */
            clr     TxBitcounter
            /* lastTxbit = rxbit */
            cbr     CanBits,(1<<lastTxbit)
            sbrc    CanBits,Rxbit
            sbr     CanBits,(1<<lastTxbit)
            rjmp    RXBIT_OK

RXBIT_L1:   /* if (T != lastTxbit) */
            bld     CanBits,lastTxbit    /* lastTxbit = T */
            clr     TxBitcounter
RXBIT_OK:
            clc
            ret
BitStuffErr:
            /* Stuff error */
            sbr     CanBits,(1<<StuffError)
            sec
            ret

/*----------------
 * CANTxBit(), the bit to be transmitted is stored in T
 * when exit, Carry is 1 if error, 0 if ok
 *
 * used regs: r31 (Arg1)
 *   by Wait: r29, r30, r31
 *   by UpdateCRC: r31
 *----------------*/
CANTxBit:
Can_SendBit:
            in      r31,TCNT0
            cpi     r31,SWITCH_POINT
            brlo    Can_SendBit      /* wait for the SYNC_SEG to
                                      * switch the line */

            brts    Can_SetTx
Can_ClrTx:
            /**
             * NB the falling edge of CanTx rise the interrupt EXT0 because
             * CanRx resemble CanTx (with a small delay). This is an unwanted
             * interrupt, however it shouldn't disturb
             **/
            ClrCanTx
            rjmp    Can_TxWait
Can_SetTx:
            SetCanTx
Can_TxWait:
            /* Reset Timer0Sig signal */
            ldi     ZL,CANBUSDRV
            FlushSignal(((1<<Timer0Sig)|(1<<CanSOFSig)),Z,r31)

            /* wait for the timer interrupt (sampling point) */
            ldi     Arg1,(1<<Timer0Sig)
            rcall   Wait
#if SIMUL_DEBUG==1
            bld     CanBits,Rxbit
#endif
            /* compare sampled bit (Rxbit) with T (bit transmitted) */
            cpneRegBit(CanBits,Rxbit,CANSendOk)
            cbr     CanBits,(1<<StuffError)
            sec
            rjmp    TXBIT_END /* the bus differ from tx bit:
                               * arbitration lost? */
CANSendOk:
            lsl     CanRxWordL
            rol     CanRxWordH
            bld     CanRxWordL,0

            rcall   UpdateCRC

            sbrs    CanBits,txBitstuffing
            rjmp    TXBIT_OK

            /**
             * Notice that lastTxbit must be the same # bit of T within SREG (6)
             **/
            in      r31,SREG
            eor     r31,CanBits
            andi    r31,(1 << lastTxbit)
            brne    TXBIT_L1

            /* if (T == lastTxbit) */
            inc     TxBitcounter
            cpi     TxBitcounter,4
            brlo    TXBIT_OK

            /* T = ~T */        /* insert bit stuffing in the output stream */
            bld     r31,0
            com     r31
            bst     r31,0

            /* NOTE!! As far as I know there is no way to lose arbitration during
             * the stuff bit. If we TX a Recessive stuff bit and receive a
             * dominant stuff bit we signal a stuff error */
/***** send stuffing bit ********/
Stuff_SendBit:
            in      r31,TCNT0
            cpi     r31,MIDDLE_SWITCH_POINT
            brlo    Stuff_SendBit      /* wait for the middle of SYNC_SEG
                                        * to switch the line */
            brts    Stuff_SetTx
Stuff_ClrTx:
            ClrCanTx
            rjmp    Stuff_TxWait
Stuff_SetTx:
            SetCanTx
Stuff_TxWait:
            bld     CanBits,lastTxbit    /* lastTxbit = T */
            clr     TxBitcounter        /* Was missing!!! */

            /* wait for the timer interrupt (sampling point) */
            ldi     Arg1,(1<<Timer0Sig)
            rcall   Wait

#if SIMUL_DEBUG==1
            bld     CanBits,Rxbit
#endif
            /* compare sampled bit (Rxbit) with T (transmitted bit) */
            cpneRegBit(CanBits,Rxbit,TXBIT_OK)
            /* stuff error */
            sbr     CanBits,(1<<StuffError)
            sec
            clt
            rjmp    TXBIT_END
/******************************************/

TXBIT_L1:    /* caso in cui (T != lastTxbit) */
            bld     CanBits,lastTxbit    /* lastTxbit = T */
            clr     TxBitcounter
TXBIT_OK:
            clc
TXBIT_END:
            ret

/*-------------------
 * CANRxWord(), CanRxWordH+CanRxWordL --> data, Arg4 --> nbits
 * Carry set if error
 *
 * used regs: r28 (Arg4)
 *   by CANRxBit: r29 (Arg3), r30 (Arg2), r31 (Arg1)
 *-------------------*/
CANRxWord:
            clr     CanRxWordH
            clr     CanRxWordL
CANRx_loop:
            rcall   CANRxBit
CANRx_LostArb_entry:
            brcs    CANRxWord_End
            dec     Arg4
            brne    CANRx_loop
            clc
CANRxWord_End:
            ret

/*-------------------
 * CANTxWordArb(), Reg1+Reg2 --> data, Arg4 --> nbits
 * Carry set if error
 *
 * used regs: Reg1 (r25), Reg2 (r0), Arg4 (r28)
 *   by CANTxBit: Arg1 (r31), Arg2 (r30 - ZL), Arg3 (r29)
 *-------------------*/
CANTxWordArb:
            /* Note that we store received bits into CanRxWord. In case we lose
             * arbitration we can switch to receive word */
            clr     CanRxWordH
            clr     CanRxWordL
CANTxWordArb_loop:
            bst     Reg1,7
            rcall   CANTxBit
            brcs    CANTxWordArb_Err
            lsl     Reg2
            rol     Reg1
            dec     Arg4
            brne    CANTxWordArb_loop
            clc
CANTxWordArb_End:
            ret

CANTxWordArb_Err:
            /* If we transmit a 0 and read 1 there is a Bit error,
             * viceversa is arbitration lost
             */
            sbrs    CanBits,StuffError
            sbrs    Reg1,7                      /* error occurred while tx a 1 */
            rjmp    CANTxWordArb_End

            /* Arbitration lost: jump to RxWord */
            cbr     CanBits,(1<<CanMaster)      /* switch to slave mode */
            rcall   LostArbRxBit                /* finish to receive current bit */
            rjmp    CANRx_LostArb_entry


/*-------------------
 * CANTxWord(), Reg1+Reg2 --> data, Arg4 --> nbits
 * Carry set if error
 *
 * used regs: Reg1 (r25), Reg2 (r0), Arg4 (r28)
 *   by CANTxBit: Arg1 (r31), Arg2 (r30 - ZL), Arg3 (r29)
 *-------------------*/
CANTxWord:
CANTxWord_loop:
            bst     Reg1,7
            rcall   CANTxBit
            brcs    CANTxWord_End
            lsl     Reg2
            rol     Reg1
            dec     Arg4
            brne    CANTxWord_loop
            clc
CANTxWord_End:
            ret

/*-----------------
 * UpdateCRC(), the next bit is in T
 *
 * used regs: r31
 *-----------------*/
UpdateCRC:
            bld     r31,6           /* CRCNEXT is r31:6 (loaded with T) */
            eor     r31,CrcRegH     /* CRCNEXT = NEXTBIT EXOR CRCREG(14) */
            lsl     CrcRegL
            rol     CrcRegH         /* LSL(CRCREG) */
            sbrs    r31,6           /* IF CRCNEXT != 0 */
            rjmp    _crcOk
            ldi     r31,0x45        /* THEN CRCREG = CRCREG EXOR 4599h */
            eor     CrcRegH,r31
            ldi     r31,0x99
            eor     CrcRegL,r31
_crcOk:
            ret


/*--------
 * StartUp Bus in off state: we wait for 128 * (EOF + Intermission) before to go to Active state
 *
 * used regs: r28 (Arg4)
 *   by CANRxBit: r29 (Arg3), r30 (Arg2), r31 (Arg1)
 *--------*/
CANBusStartUp:
            /* TX pin high during idle state */
            SetCanTx
            clr     CanBits

            push    Reg1
            ldi     Reg1,128
bustart_loop:
            rcall   CANBusResync
            dec     Reg1
            brne    bustart_loop

            clr     TxErrcounter
            clr     RxErrcounter
            pop     Reg1
            sec
            ret
