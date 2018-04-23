/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2002 by Claudio Lanconelli
 * e-mail: lancos at users.sourceforge.net
 * WWW: http://caraca.sourceforge.net
 *
 *--- RC5 decoding routines and key sampling ---*

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
#include "eeprom.e"
#include "canbus.e"

            extern TableLookup:

/****************
 * Constants
 ****************/

#define     RC5_WORDLEN     14      /* 2bit Start + 1bit Toggle + 5bit address + 6bit command */

/*the following constant is crucial for RC5 decoding. The standard bit
 * time is 1.778 msec. RC5 thread can sample the IrRx line at any frequency
 * from 1/1.778E-3 * 4 = 2250Hz to 1/1.778E-3 * 169 = 95KHz
 * In this case we have the sampling frequency equal to 4800, so RC5_BITTIME
 * is BITRATE / (1/1.778E-3)
 * Example:
 * 15625 / (1/1.788E-3) = 27.78, rounded to 28
 *
 * This routine samples countinously the Ir line at the sample frequency (BITRRATE)
 * to look for a change. When a change occurs it measures how many time elapsed.
 * In case of too short time (T < 1/4 BITTIME) or too long time (T > 11/8 BITTIME)
 * the code is invalid and discarded.
 * In case of short time (1/4 BITTIME < T < 3/4 BITTIME) the bit value is the same
 * of the previous value.
 * In case of long time (3/4 BITTIME < T < 11/8 BITTIME) the bit value is the previous
 * value inverted. When the line is idle is always high so the start bit is always
 * "1" (falling edge).
 * The biphase coding used by the RC5 (also called manchester) is showed in the
 * following diagram:
 *
 *       bit time
 *     <--------->
 * ----+    +----+         +----+    +----+----+         +----
 *     |    |    |         |    |    |         |         |
 *     +----+    +----+----+    +----+         +----+----+
 *     ^    *    ^         ^    *    ^         ^         ^
 *     1         1         0         0         1         0
 *
 * Falling edge is interpreted as a logic "1"
 * Rising edge is interpreted as a logic "0"
 *
 * The T flash is used to skip the "extra" changes when the "short" state is received.
 * Those extra changes are marked with a * in the diagram above.
 */

#define		RC5_BITTIME		( ((BITRATE * 100) / 5624 + 5) / 10 )


/* A note on register usage:
 * Thread3 should send the CANBus msg directly instead of let EDISPATCHER
 * to send the msg */

#define IrCount Reg1        /* use Reg1 to save global register */

        seg    flash.code
/*=================================
 * THREAD 3  IrReceiver thread (decode RC-5)
 * RC5DEflash.codeR
 *=================================*/
public Thr3_entry:
            /* put initialization code here */

/** configured as input by default after reset
            cbi     IR_DIR_PORT,IrRxP       ;input

            cbi     KEY_DIR_PORT,Key1P      ;input
            cbi     KEY_DIR_PORT,Key2P
            cbi     KEY_DIR_PORT,Key3P
**/
            sbi     IR_OUT_PORT,IrRxP       ;pullup

            in      Arg1,KEY_OUT_PORT       ;pullup
            sbr     Arg1,KEY_INIT_MASK
            out     KEY_OUT_PORT,Arg1

#ifdef  LedIrP
            sbi     LED_DIR_PORT,LedIrP
            sbi     LED_OUT_PORT,LedIrP    ;switch LED OFF on power-up
#endif
            clr     Reg2

            clr     OldKeyStatus            /* Only registers r16-r31 for 'ser' */
            dec     OldKeyStatus
            clr     KeyChange

            /* Wait for CanBus thread */
            ldi     Arg1,(1<<CanReadySig)
            rcall   Wait

            /* Clear any previous timer signals */
            ldi     Arg1,(1<<Timer0Sig)|(1<<Timer1Sig)
            rcall   Wait

thr_loop:   ldi     Arg1,(1<<Timer0Sig)|(1<<Timer1Sig)
            rcall   Wait

            push    Res1
            sbrs    Res1,Timer0Sig
            rjmp    test_keys
rc5_start:
            /* compare last sampled status with current one */
            cpeqPortBit(IR_IN_PORT, IrRxP, RC5_NOTEQ)

            tst     IrIdx           /* IrIdx zero mean "wait start" */
            breq    RC5_NOTIMEOUT   /*    breq    rc5_end */

            inc     IrCount
            cpi     IrCount,(RC5_BITTIME*110 + 5) / 80  ;11/4 bittime rounded to integer
            brlo    RC5_NOTIMEOUT

            /* test if timeout was caused by stop bit (last bit was 1) */
            cpi     IrIdx,RC5_WORDLEN
            breq    RC5_LONG
RC5_TIMEOUT:
            clr     IrIdx        /* discard data and go to "wait start" state */
            set
RC5_NOTIMEOUT:
            rjmp    rc5_end

RC5_NOTEQ:
            tst     IrIdx
            brne    RC5_NOWAIT
            sbr     UserBits, (1 << lastIrBit)
            ldi     IrCount,RC5_BITTIME
            clr     Reg4                        ;IrDataL
RC5_NOWAIT:
            ;check for short impulses (spikes or wrong coding)
            cpi     IrCount,(RC5_BITTIME*20) / 80       ;1/4 bittime truncated to integer
            brlo    RC5_TIMEOUT                 ;discard data

            ;test for short or long status
            cpi     IrCount,(RC5_BITTIME*60 + 5) / 80   ;3/4 bittime rounded to integer
            brsh    RC5_LONG

            cpeqRegBit(UserBits,lastIrBit,RC5_L1)
            lsl     Reg4                        ;IrDataL
            rol     Reg3                        ;IrDataH
            sbrc    UserBits, lastIrBit
            sbr     Reg4,1                      ;IrDataL
            inc     IrIdx
RC5_L1:
            rjmp    RC5_L2
RC5_LONG:                                       ;long status
            lsl     Reg4                        ;IrDataL
            rol     Reg3                        ;IrDataH
            sbrc    UserBits, lastIrBit
            sbr     Reg4,1                      ;IrDataL
            inc     IrIdx

            ldi     Arg1,(1 << lastIrBit)       ;one's complement of lastIrBit
            eor     UserBits,Arg1
RC5_L2:
            brts    RC5_ClrT                    ;one's complement of T
            set
            rjmp    RC5_L3
RC5_ClrT:
            clt
RC5_L3:
            clr     IrCount
            cpi     IrIdx,RC5_WORDLEN+1
            brlo    rc5_end
RC5_RECVOK:
            clr     IrIdx

            mov     Arg1,Reg3
            andi    Arg1,0xF0
            cpi     Arg1,0x40
            brne    rc5_end
#if IR_DUMP==0
            mov     Reg2,Reg3
            andi    Reg3,0x07       ;Clear "no repeat toggle flag"

/* Look for the correct entry in the Table */
            ldi     Arg1,e2_RC5Table   /* start of table in EEprom */
            ldi     Arg2,((e2_EndOfRC5Table - e2_RC5Table) / 2)   /* table size (in word) */
            rcall   TableLookup
            brcs    rc5_end
            mov     Arg4,Arg1       ;Arg4 = key index

            ;repeat test
            clr     Arg3            ;Arg3 = 1 if repeated key
            mov     Arg1,Reg2
            andi    Arg1,0x08       ;toggle bit
            brne    rc5_reptest
            sbrs    UserBits,lastIrToggle
            inc     Arg3
            cbr     UserBits,BV(lastIrToggle)
            rjmp    rc5_norep
rc5_reptest:
            sbrc    UserBits,lastIrToggle
            inc     Arg3
            sbr     UserBits,BV(lastIrToggle)
rc5_norep:
#else
            /* Promiscuous mode - every IrCode gets transfered */
             
            mov     Arg3, Reg4              ;IrDataL
            mov     Arg4, Reg3              ;IrDataH
#endif
rc5_tx:     ;transmit IrData...

            ldi     Arg1,e2_RC5code
            rcall   CanTxFrame3Queue

#ifdef LedIrP
            cbi     LED_OUT_PORT,LedIrP    ;switch Led ON
            clr     Reg2            ;delayed switch LED off
                                    ;When Ir code is received the led lamp
                                    ;The led is switched ON now and switched off
                                    ;again after 250 loops (timer signals)
                                    ;The counter is stored in Reg2
#endif

rc5_end:
#ifdef LedIrP
            dec     Reg2
            skipne
            sbi     LED_OUT_PORT,LedIrP
#endif


test_keys:
            pop     Res1
            sbrs    Res1,Timer1Sig
            rjmp    thr_loop
sample_key:
            in      Arg1,KEY_IN_PORT

            ;loop to wait for lines stabilize
            rcall   Yield
            in      Arg2,KEY_IN_PORT
            cp      Arg1,Arg2
            brne    sample_key

            eor     OldKeyStatus,Arg1

            ldi     Arg1,e2_InputMask_a
            rcall   EEReadByte
            and     Arg1,OldKeyStatus
            breq    key_ok
            mov     KeyChange,Arg1
            
            ldi     Arg2,EDISPATCHER
            ldi     Arg1,(1<<KeyDataSig)
            rcall   Signal

key_ok:     in      OldKeyStatus,KEY_IN_PORT
            rjmp    thr_loop
