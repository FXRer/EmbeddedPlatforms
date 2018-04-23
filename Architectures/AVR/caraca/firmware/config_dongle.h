/** CONFIGURATION **/
#ifndef _CONFIG_H
#define _CONFIG_H

#arch AT90S2313

/****************************************
 * Version and Revision numbers
 ****************************************/
#include "version.h"

/****************************************
 * DEBUG AND/OR CODE SWITCH FLAGS
 ****************************************/

#define RESET_BLINK 0       /* blink the LED 3 times on startup */
#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check for transmitted bit) */
#define NOACK_RETRY 1       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0
#define TRACERXERR  0
#define TRACEBUSOFF 0
#define TRACELOSTARB 0
#define TRACERESYNC 0       /* This should exclude the TRACEBITERR */
#define SOFT_RESYNC 0       /* if 1 enable soft resyncronization on every falling edge: NEED TEST */
#define RX_DOUBLE_BUFFER 0  /* Use 2 Rx-Buffers alternativly */
#define TX_QUEUE    0       /* Use 4 Registers as Tx-Queue */


#define CANDONGLE
/*#define BOOT_MSG*/
/*#define DEBUG_ECHO*/
/*#define CANDONGLE_DEBUG_TIMING*/

/****************************************
 * KERNEL
 ****************************************/

/** Set to 1 if you need priority scheduling (Thread 0 higher priority)
  * Set to 0 for round-robin scheduling **/
#define PRIORITY_SCHEDULE 0

/** If 1: Timer0 interrupt routine call Schedule() (short responses)
  * if 0: Timer0 interrupt act like other ISR (end with reti) **/
#define INTERRUPT_SCHEDULE  0

/** Number of active THREADS **/
#define NO_OF_THREAD  2

/** Stack RAM allocation for every thread **/
#define THR1_STACK  28
#define THR2_STACK  30

/* Priority of Threads, MUST be a different number for every thread
 * from 1 to NO_OF_THREAD */
#define PRI_CANBUSDRV   1
#define PRI_EDISPATCHER 2

#define THR_ID(x) (ThrVect + (x) - 1)

#define CANBUSDRV       THR_ID(PRI_CANBUSDRV)
#define EDISPATCHER     THR_ID(PRI_EDISPATCHER)


#define XTAL_FREQUENCY      8000000     /* Use the same XTAL of NODE and STAR to match exactly the BitRate */
#define TIMER0_PRESCALER    8

#define BAUDRATE            38400


/*****************************************
 * SIGNALS
 *****************************************/

/** Thr1 related **/
#define CanReqSig   0       /* CAN request signal */
#define CanSOFSig   1       /* CAN StartOfFrame signal */

/** Thr2 related **/
#define RSDataSig   0       /* Valid message from RS232 received */
#define CanDataSig  1       /* Valid CAN message received */
#define CanErrSig   2    /* CAN error occured, send to RS232 */

/** Common **/
#define CanReadySig 3       /* CanBus ready after power-up */
#define Timer0Sig   4       /* Timer0 overflow signal */
#define Int0Sig     5       /* Interrupt0 signal */
#define Int1Sig     6       /* Interrupt1 signal */


/***************************************************************************
 * REGISTERS <-> VARIABLES ASSIGNMENTS
 ***************************************************************************/

/* kernel vars */
/* R0: scratch register (used for LPM instruction) */

/* 4 bit ReadyThreadCounter - 4 bit RunningThread index */
#define KernelStatus    r1      /* current running thread */

#define KernelBits      r20     /* to save space this reg is used to store thread bits */
#define SleepStatus     7       /* 1 when sleep withink schedule, 0 otherwise */

#define UserBits        r20     /* to save space share the same register with KernelBits */
#define RxLockBit       0       /* CAN msg Rx Buffer 1 lock */
#define RxLockBit1      0       /* CAN msg Rx Buffer 1 lock */
#define RxLockBit2      1       /* CAN msg Rx Buffer 2 lock */
#define TxLockBit       2       /* CAN msg Tx Buffer lock */

#define SaveStatus      r2      /* save SREG within Interrupt */
#define IntTmp          r16     /* interrupt scratch register */
#define IntTmp1         IntTmp  /* just an alias */

#define IntTmp2         r5      /* It's not strictly necessary, but save a push within Interrupt routines (fast responese and less stack) */

/** CRC vars **/
#define CrcRegL         r3
#define CrcRegH         r4

/** CAN vars **/
#define TxErrcounter    r12
#define RxErrcounter    r18

#define TxBitcounter    r19

#define CanBits         r21
#define CanMaster       7       /* tell if we are master or slave in current frame */
#define lastTxbit       6
#define txBitstuffing   5       /* bit stuffing enabled/disabled */
#define Rxbit           4       /* CAN Rx sampled in timer intr */
#define HardSync        3       /* if 1 HardSync, else SoftSync */
                                /* note that hard sync is performed only at SOF,
                                 * so this bit tell if we are within a frame or not */
#define ErrorPassive    2       /* 1 - Error Passive, 0 - Error Active */
#define StuffError      1       /* 1 - Stuff error, 0 - bit error */
                                /* bit 0 available */

#define StuffArbRtr     5       /* Stuff error during arbitration before RTR bit (exception 2) */
#define PassNoAck       6       /* Error passive get NoAck (exception 1) */

/* Store the received word here, even when we're transmitting, so we can switch between
 * master transmitter and slave receiver without lose current word */
#define CanRxWordH      r10
#define CanRxWordL      r11

/* RS232 vars for candongle - same as Ir RC5 (can't be used together) */
#define sRxPos          r22
#define sRxMsgLen       r24
#define sTxPos          r15    /* Pos in Tx Buffer */
#define sTxMsgLen       r17     /* Bytes to transmit */
#define sRxMode         r23     /* Command mode: C-CanMsg, R,W - eeprom */

#if TX_QUEUE==1
/* Event queue */
#define EqueueIDptr     r9      /* if 0 the queue is empty, otherwise point to eeprom location for MsgID */
#define Equeue1         r13
#define Equeue2         r14
#define Equeue3         r6
#endif

#if RX_DOUBLE_BUFFER==1
#define RxBufIdx        r24      /* a bit for every thread tell what RxBuffer they are using */
#endif

#define CanErrCode      r7

/*** available registers
 r8
***/

/* Scratch Regs: r28, r29, r30, r31 */
/* Argument Passing conventions */
#define Arg1  r31
#define Arg2  r30
#define Arg3  r29
#define Arg4  r28

#define Res1  r31
#define Res2  r30

/* preservable regs */
#define Reg1    r25
#define Reg2    r0
#define Reg3    r26
#define Reg4    r27


/*******************************************
 * CANBus Constants
 *******************************************/
#include "can_config.h"

#define MAX_CANMSG_SIZE    8

/***************************************************************************
 * Port Pin Assignements
 ***************************************************************************/

/* RS232 UART Pins */
#define     SerRxP          0      /* In */
#define     SerTxP          1      /* Out */
#define     SerCTS          5      /* Out */

#define     UART_DIR_PORT   DDRD
#define     UART_OUT_PORT   PORTD
#define     UART_IN_PORT    PIND


/* CAN Pins */
#define		CANRxInterrupt	0

#define     CanRxP          2      /* In  (INT0) */
#define		CAN_RXDIR_PORT	DDRD
#define     CAN_RXIN_PORT   PIND

#define     CanTxP          4      /* Out */
#define		CAN_TXDIR_PORT	DDRD
#define		CAN_TXOUT_PORT	PORTD

#define     CanSleepP       6      /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD


/* Led Pins */
#define     LedBusRxP       2       /* LedRedP         2 */
#define     LedYellowP      3
#define     LedLoadP        3     /* To see Processor load/work */
#define     LedBusTxP       4       /* LedGreenP       4 */

#define     LED_DIR_PORT    DDRB
#define     LED_OUT_PORT    PORTB
#define     LED_IN_PORT     PINB


/* Trace pins (only for debug) */
#define     TRACE_DIR_PORT  DDRB
#define     TRACE_OUT_PORT  PORTB

#define     TraceRxBitP     0
#define     TraceInt0P      1
#define     TraceBitErrP    5
#define     TraceStuffErrP  6
#define     TraceFormErrP   7

#define     TraceResyncNeg  6
#define     TraceResyncPos  7

#endif
