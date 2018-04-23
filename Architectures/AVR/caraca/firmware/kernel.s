/***************************************************************************
 * CARACA
 * CAn Remote Automation and Control with the Avr
 *
 * Copyright (c) 1998-2000 by Claudio Lanconelli
 * e-mail: lanconel@cs.unibo.it
 * WWW: http://caraca.sourceforge.net
 *
 *--- A nanokernel for any AVR with 128Byte SRAM (AT90S2313/2333/4433/...) ---*

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
#include "kernel.h"
#include "serial.h"  

        extern  Thr1_entry:
        extern  Thr2_entry:
        extern  Thr3_entry:
        extern  Thr4_entry:
        extern  Thr5_entry:

        extern  Reset_drv:  /* Reset handle */
        extern  Int0_drv:   /* external Interrupt0 Vector */
        extern  Int1_drv:   /* external Interrupt1 Vector */
        extern  Icp1_drv:   /* Input Capture1 Interrupt Vector */
        extern  Oc1_drv:    /* Output Compare1 Interrupt Vector */
        extern  Ovf1_drv:   /* Overflow1 Interrupt Vector */
        extern  Ovf0_drv:   /* Overflow0 Interrupt Vector */

        extern  Urxc_drv:   /* UART Receive Complete Interrupt Vector */
        extern  Udre_drv:   /* UART Data Register Empty Interrupt Vector */
        extern  Utxc_drv:   /* UART Transmit Complete Interrupt Vector */
        extern  Aci_drv:    /* Analog Comparator Interrupt Vector */

#ifdef  AT90S4433           /* AVR 4433 add some Interrupt respect to 2313 */
        extern  Spi_drv:    /* SPI Interrupt Vector */
        extern  Adc_drv:    /* ADC Interrupt Vector */
        extern  EErdy_drv:  /* EEProm ready Interrupt Vector */
#endif

/***************************************************************************
 * KERNEL STACKS
 ***************************************************************************/

/* --- A note on stack allocation ---
 * When the system show strange behaviurs, it may depend on
 * bad stack allocation (may be it resets randomly)
 *
 * To avoid these annoying problems consider the following minimal stack
 * space allocation.
 *
 * Stack Image after a Wait system call, system Idle and Interrupt
 ** Every thread need at least the following bytes of stack
 *        RET PC (from Wait)          (2 BYTES)
 *        RET PC (from ContextSwitch) (2 BYTES)
 *        R0     (1
 *        SREG   (1
 *        R31    (1
 *        R30    (1
 *        R29    (1
 *        R28    (1
 *        R27    (1
 *        R26    (1
 * SP --> R25    (1
 *        RET PC (interrupt)          (2 BYTES)
 * Interrupt save/restore registers (usually 2 BYTES but depend on the application)
 *
 * Total = 17 bytes
 * Add all stack used by the application thread (nested calls and saved registers)
 **/

/** Thread Status Structure
 * - WaitSignals     (0 mean a ready thread)
 * - ReceivedSignals
 * - StackPtr
 **/

/*-------------------------*/
          seg  eram.data

Bos:
#ifdef  THR5_STACK
#if (NO_OF_THREAD > 4) & (THR5_STACK > 2)
            ds.b    THR5_STACK-2
Thr5Stack:  ds.b    2
#endif
#endif

#ifdef  THR4_STACK
#if (NO_OF_THREAD > 3) & (THR4_STACK > 2)
            ds.b    THR4_STACK-2
Thr4Stack:  ds.b    2
#endif
#endif

#ifdef  THR3_STACK
#if (NO_OF_THREAD > 2) & (THR3_STACK > 2)
            ds.b    THR3_STACK-2
Thr3Stack:  ds.b    2
#endif
#endif

#ifdef  THR2_STACK
#if (NO_OF_THREAD > 1) & (THR2_STACK > 2)
            ds.b    THR2_STACK-2
Thr2Stack:  ds.b    2
#endif
#endif

#if THR1_STACK > 2
            ds.b    THR1_STACK-2
Thr1Stack:
#endif
Tos:        ds.b    2


/***************************************************************************
 * KERNEL DATA
 ***************************************************************************/
          seg eram.data 

public ThrVect:
WaitStatusVect:  ds.b  NO_OF_THREAD  /* WaitStatus Vector:
                                      * tell if a thread is waiting for
                                      * a signal (and which signal) */
RecvSignalVect:  ds.b  NO_OF_THREAD  /* Received Signal Vector:
                                      * tell which signals a thread
                                      * have received */
StackPtrVect:    ds.b  NO_OF_THREAD  /* StackPointer Vector:
                                      * tell the stack ptr of every thread */


/***************************************************************************
 * VECTORS
 ***************************************************************************/
          seg abs=0 flash.code

          rjmp  Reset_drv   /* Reset handle */
          rjmp  Int0_drv    /* external Interrupt0 Vector */
          rjmp  Int1_drv    /* external Interrupt1 Vector */
          rjmp  Icp1_drv    /* Input Capture1 Interrupt Vector */
          rjmp  Oc1_drv     /* Output Compare1 Interrupt Vector */
          rjmp  Ovf1_drv    /* Overflow1 Interrupt Vector */
          rjmp  Ovf0_drv    /* Overflow0 Interrupt Vector */
#ifdef  AT90S4433 
          rjmp  Spi_drv     /* SPI Interrupt Vector */
#endif
          rjmp  Urxc_drv    /* UART Receive Complete Interrupt Vector */
          rjmp  Udre_drv    /* UART Data Register Empty Interrupt Vector */
          rjmp  Utxc_drv    /* UART Transmit Complete Interrupt Vector */
#ifdef  AT90S4433
          rjmp  Adc_drv     /* ADC Interrupt Vector */
          rjmp  EErdy_drv   /* EEProm ready Interrupt Vector */
#endif
          rjmp  Aci_drv     /* Analog Comparator Interrupt Vector */

/*-------------------------------------*/
public SerialNumber:    /* moved from eeprom to flash segment */
            dc.W    SERIAL
			
			/* serial number, unique for every node.
			   incremented via make or
			   The programmer (PonyProg) will set it
			   before programming big endian format */

/***************************************************************************
 * KERNEL ROUTINES
 ***************************************************************************/
          seg  flash.code.Kernel

/*----------------
 * KernelInit
 *----------------*/
public KernelInit:
        /* Init system stack */
          ldi   r31,Tos
          out   SPL,r31

          clr   KernelBits

          ldi   r31,(NO_OF_THREAD << 4)
          mov   KernelStatus,r31        /* ReadyThreadCounter = NO_OF_THREAD and current running thread = 0 */

          /* init RecvSignalVect and WaitStatusVect with 0s */          
          ldi   r30,WaitStatusVect
          clr   XL
          ldi   XH,(NO_OF_THREAD * 2)
clr_loop: st    Z+,XL
          dec   XH
          brne  clr_loop

          /* init StackPtrVector */
          ldi   XL,Thr1Stack
       /* ldi   YL,low(Thr1_entry/2)
          ldi   YH,high(Thr1_entry/2)
          rcall InitStack   ** the first thread call directly context_switch, so it doesn't need custom stack initialization */
          st    Z+,XL
#if NO_OF_THREAD  > 1
          ldi   YL,low(Thr2_entry/2)
          ldi   YH,high(Thr2_entry/2)
          ldi   XL,Thr2Stack
          rcall InitStack
          st    Z+,XL
#if NO_OF_THREAD  > 2
          ldi   YL,low(Thr3_entry/2)
          ldi   YH,high(Thr3_entry/2)
          ldi   XL,Thr3Stack
          rcall InitStack
          st    Z+,XL
#if NO_OF_THREAD  > 3
          ldi   YL,low(Thr4_entry/2)
          ldi   YH,high(Thr4_entry/2)
          ldi   XL,Thr4Stack
          rcall InitStack
          st    Z+,XL
#if NO_OF_THREAD  > 4
          ldi   YL,low(Thr5_entry/2)
          ldi   YH,high(Thr5_entry/2)
          ldi   XL,Thr5Stack
          rcall InitStack
          st    Z+,XL
#endif
#endif
#endif
#endif
          sei                       /* enable interrupts */
          rcall ContextSwitch       /* first context switch */
          rjmp  Thr1_entry

/*----------------
 * InitStack( top_ofthe_stack:X, code_entry:Y )
 *----------------*/
public InitStack:
          st    X,YL    /* LSB of address first */
          st    -X,YH    /* MSB of address */
          clr   r28
          st    -X,r28    /* R0 */
          sbr   r28,0x80
          st    -X,r28    /* SREG with interrupt enabled */

          /* Add Other registers ininitalization here */
          subi   XL,1+7    /* alloc 7 register space on the stack */
          ret


/*----------------
 * Signal( Signal_mask, thr_id )
 *              r31       r30(ZL)
 *
 * scratch registers:
 *    r31, r29
 *
 * thr_id : is the pointer to WaitStatusVect[THR_IDX],
 *          note that WaitStatusVect + THR_IDX + NO_OF_THREAD point to RecvSignalVect[THR_IDX]
 *
 * return r31 cleared (0)
 *
 * NOTE!!! Don't call Signal withing ISR, use IsrSignal() macro instead
 *----------------*/
public Signal:
            cli
            IsrSignal(r31,Z,r29)
            sei
        /*  rjmp  ContextSwitch  ** switch immediately to the new wake up thread */
            ret


/*----------------
 *  recv_signal_mask = Wait( byte wait_mask )
 *          Res1                  Arg1
 *
 * scratch registers:
 *     r30, r29
 *----------------*/
public Wait:
          cli
          
          /* at first check if we have already received the signals */
       /*   clr   ZH ** 2313 */
          mov   ZL,KernelStatus
          andi  ZL,0x0F
          addi(ZL,WaitStatusVect)
          
          ldd   r29,Z+NO_OF_THREAD   /* get the received signal mask from RecvSignalVect(RunningThread) */
          and   r29,r31
          brne  wait_nosleep

          st    Z,r31               /* set the new waitmask */
          tst   r31
          breq  wait_nosleep
          swap  KernelStatus
          dec   KernelStatus         /* decrement the number of ready threads */
          swap  KernelStatus
wait_nosleep:
          sei
          rcall ContextSwitch
          ldd   r29,Z+NO_OF_THREAD   /* get the received signal mask from RecvSignalVect(RunningThread) */
          and   r31,r29             /* returns received signal we are waiting for */
          com   r31
          and   r29,r31              /* clear received signal we are waiting for */
          std   Z+NO_OF_THREAD,r29   /* store the new received signal mask */
          com   r31
          ret

/*-------------------
 * perform a thread context switch
 *-------------------*/
public Yield:
public ContextSwitch:
          push  r0        /* save R0 */
          in    r0,SREG
          cli             /* disable further interrupts */
public _IsrContextSwitch:
          push  r0        /* save SREG */
          push  r31       /* save R31 */
          push  r30
          push  r29
          push  r28
          push  r27
          push  r26
          push  r25

          in    r0,SPL
          mov   ZL,KernelStatus
          mov   r31,KernelStatus
          cbr   r31,0x0F
          mov   KernelStatus,r31

#if  SCHEDULER_LED == 2
          ;Switch all scheduler LEDs off
          sbi     LED_OUT_PORT,Led1P
          sbi     LED_OUT_PORT,Led2P
          sbi     LED_OUT_PORT,Led3P
          sbi     LED4_OUT_PORT,Led4P
#endif

      /*    clr   ZH ** 2313 */
          andi  ZL,0x0F
          addi(ZL,low(WaitStatusVect))
      /*    addiz(WaitStatusVect) ** 2313 Address is 8 bit */
          std   Z+(NO_OF_THREAD*2),r0       /* save the stack pointer */

scheduler:
          tst   KernelStatus    /* test for ready thread counter */
          brne  sched_0
#ifdef LedLoadP 
          sbi     LED_OUT_PORT,LedLoadP
#endif
          sbr   KernelBits,(1<<SleepStatus)
          sei
          sleep
#if PRIORITY_SCHEDULE==0
          ldi   ZL,low(WaitStatusVect-1)    /* After Sleep always restart with thread 0 */
#endif
		  wdr
          cli
          cbr   KernelBits,(1<<SleepStatus)
#ifdef LedLoadP
          cbi     LED_OUT_PORT,LedLoadP
#endif       
          rjmp  scheduler

sched_0:
          /* if you need a round-robin (without priority) comment the following line */
#if PRIORITY_SCHEDULE==1
          ldi   ZL,low(WaitStatusVect-1)  /* thread 0 --> max priority
                                           * thread 1
                                           * ...
                                           * thread N --> min priority
                                           */
#endif

sched_loop:
          inc   ZL              /* ZL --> current thread index */
          cpi   ZL,(WaitStatusVect+NO_OF_THREAD)
          brlo  sched_1
          ldi   ZL,low(WaitStatusVect)
      /*  ldi   ZH,high(WaitStatusVect) ** 2313 address is 8 bit */
sched_1:
          /* test if the new thread is ready */
          ld    r0,Z
          tst   r0                   /* if zero the new thread is ready */
          brne  sched_loop

dispatch:
          ldd   r0,Z+(NO_OF_THREAD*2)     /* restore the stack pointer in R0 */

          subi  ZL,low(WaitStatusVect)
       /* sbci  ZH,high(WaitStatusVect) ** 2313 address is 8 bit */
          or    KernelStatus,ZL      /* set new current thread index */

#if  SCHEDULER_LED == 2
          cpi   ZL,0    ;Thread 0
          skipne
          cbi     LED_OUT_PORT,Led1P
          ;switch thread0 led
          cpi   ZL,1    ;Thread 1
          skipne
          cbi     LED_OUT_PORT,Led2P
          cpi   ZL,2    ;Thread 2
          skipne
          cbi     LED_OUT_PORT,Led3P
          cpi   ZL,3    ;Thread 3
          skipne
          cbi     LED4_OUT_PORT,Led4P
#endif
          out   SPL,r0                   /* restore stack pointer */

          pop   r25
          pop   r26
          pop   r27
          pop   r28
          pop   r29
          pop   r30
          pop   r31
          pop   r0          /* restore SREG */
          out   SREG,r0
          pop   r0          /* restore R0 */
          ret
