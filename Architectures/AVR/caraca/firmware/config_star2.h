/** CONFIGURATION **/
#ifndef _CONFIG_H
#define _CONFIG_H

#arch AT90S4433 

/****************************************
 * Version and Revision numbers
 ****************************************/
#include "version.h"

/****************************************
 * DEBUG AND/OR CODE SWITCH FLAGS
 ****************************************/

#define RESET_BLINK 1       /* blink the LED 3 times on startup */
#define CANTX_TEST  0       /* CAN Tx test: loop every second and xmit a different msg ID */
#define HARDWARE_TEST   0   /* 1 to perform functional test on startup */

#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check of the transmitted bit) */
#define NOACK_RETRY 0       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0
#define TRACERXERR  0
#define TRACEBUSOFF 0
#define TRACELOSTARB 0
#define TRACERESYNC 0       /* This should exclude the TRACEBITERR */
#define SCHEDULER_LED 0     /* 0 --> no led; 1 --> Load led; 2 --> Thread Led */

#define SOFT_RESYNC 1       /* if 1 enable soft resyncronization on every falling edge: NEED TEST */
#define RX_DOUBLE_BUFFER 1  /* Use 2 Rx-Buffers alternativly */
#define TX_QUEUE    1       /* Use 4 Registers as Tx-Queue */

#define ENABLE_ASSTABLE 1

/****************************************
 * KERNEL
 ****************************************/

/** Set to 1 if you need priority scheduling (Thread 0 higher priority)
  * Set to 0 for round-robin scheduling **/
#define PRIORITY_SCHEDULE 1

/** If 1: Timer0 interrupt routine call Schedule() (short responses)
  * if 0: Timer0 interrupt act like other ISR (end with reti) **/
#define INTERRUPT_SCHEDULE  0

/** Number of active THREADS **/
#define NO_OF_THREAD  3

/** Stack RAM allocation for every thread **/
#define THR1_STACK  28
#define THR2_STACK  24
#define THR3_STACK  24

/* Priority of Threads, MUST be a different number for every thread
 * from 1 to NO_OF_THREAD */
#define PRI_CANBUSDRV   1
#define PRI_EDISPATCHER 2
#define PRI_RTCSERVICE  3

#define THR_ID(x) (ThrVect + (x) - 1)

#define CANBUSDRV       THR_ID(PRI_CANBUSDRV)
#define EDISPATCHER     THR_ID(PRI_EDISPATCHER)
#define RTCSERVICE      THR_ID(PRI_RTCSERVICE)


#define XTAL_FREQUENCY  8000000
#define TIMER0_PRESCALER    8

#define TIMER1_PRESCALER    1024
#define TIMER1_FREQUENCY    5
#define TIMER1_RELOAD       (XTAL_FREQUENCY / TIMER1_PRESCALER / TIMER1_FREQUENCY)

/*****************************************
 * SIGNALS
 *****************************************/

/** Thr1 related **/
#define CanReqSig   0       /* CAN request signal */
#define CanSOFSig   1       /* CAN StartOfFrame signal */

/** Thr2 related **/
#define CanDataSig  0       /* Valid CAN message received */
#define KeyDataSig  1       /* Valid keystroke */

/** Thr3 related **/
#define RtcLockSig  0       /* Ir wait for data to be processed */

/** Common **/
#define CanReadySig 3       /* CanBus ready after power-up */
#define Timer0Sig   4       /* Timer0 overflow signal */
#define Timer1Sig   5       /* Timer1 output compare signal (200msec) */
#define Int0Sig     6       /* Interrupt0 signal */
#define Int1Sig     7       /* Interrupt1 signal */


/***************************************************************************
 * REGISTERS <-> VARIABLES ASSIGNMENTS
 ***************************************************************************/

/* kernel vars */
/* R0: scratch register (used by LPM instruction) */

/* 4 bit ReadyThreadCounter - 4 bit RunningThread index */
#define KernelStatus    r1      /* current running thread */

#define KernelBits      r17     /* to save space this reg is used to store even UserBits */
#define SleepStatus     7       /* 1 when sleep withink schedule, 0 otherwise */

#define UserBits        r17     /* to save space this reg is used to store even KernelBits */
#define TxLockBit       0       /* CAN msg Tx Buffer lock */
#define RxLockBit1      1       /* CAN msg Rx Buffer 1 lock */
#define RxLockBit2      2       /* CAN msg Rx Buffer 2 lock */

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

#define CanBits         r20
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

#if TX_QUEUE==1
/* Event queue */
#define EqueueIDptr     r21     /* if 0 the queue is empty, otherwise point to eeprom location for MsgID */
#define Equeue1         r13
#define Equeue2         r14
#define Equeue3         r15
#endif

#if RX_DOUBLE_BUFFER==1
#define RxBufIdx        r22     /* a bit for every thread tell what RxBuffer they are using */
#endif

#define KeyMask1    r6          /* temp regs used by Lookup Association Table routine */
#define KeyMask2    r7

#define AlarmCountL r8
#define AlarmCountH r9

/*** available registers
#define r23
#define r24
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

#define MAX_CANMSG_SIZE    4

/***************************************************************************
 * Port Pin Assignements
 ***************************************************************************/

/* RS232 UART Pins */
#define     SerRxP          0      /* In */
#define     SerTxP          1      /* Out */

#define     UART_DIR_PORT   DDRD
#define     UART_OUT_PORT   PORTD
#define     UART_IN_PORT    PIND


/* CAN Pins */
#define		CANRxInterrupt	1

#define     CanRxP          3      /* In  (INT1) */
#define		CAN_RXDIR_PORT	DDRD
#define     CAN_RXIN_PORT   PIND

#define     CanSleepP       2      /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD

#define     CanTxP          5      /* Out */
#define     CAN_TXDIR_PORT  DDRC
#define     CAN_TXOUT_PORT  PORTC

/* Relay Pins */
#define     ReleA1P         2
#define     ReleA2P         1
#define     ReleA3P         0

#define     RELEA_DIR_PORT  DDRB
#define     RELEA_OUT_PORT  PORTB

#define     ReleB1P         7
#define     ReleB2P         6
#define     ReleB3P         5
#define     ReleB4P         4

#define     RELEB_DIR_PORT  DDRD
#define     RELEB_OUT_PORT  PORTD


/* Led Pins */
#if RESET_BLINK==1
#define     LedDrvP     2   /* Reset blink led */
#endif
#define     LedBusRxP   2   /* CANBus Rx Led */
#define     LedBusTxP   2   /* CANBus Tx Led */
/*#define     LedDebugP   2*/   /* Debug led */
#if SCHEDULER_LED==1
#define     LedLoadP    2   /* Load vs. Idle cpu led */
#endif

#define     LED_DIR_PORT    DDRC
#define     LED_OUT_PORT    PORTC
#define     LED_IN_PORT     PINC


/* SPI Pins */
#define     SckP            5
#define     MisoP           4
#define     MosiP           3

#define     SPI_DIR_PORT    DDRB
#define     SPI_OUT_PORT    PORTB
#define     SPI_IN_PORT     PINB


/* I2C Bus pins */
#define     SCLP            3       /* SCL Pin number */
#define     SDAP            4       /* SDA Pin number */

#define     I2C_DIR_PORT    DDRC
#define     I2C_OUT_PORT    PORTC
#define     I2C_IN_PORT     PINC


/* Trace pins (only for debug) */
#define     TRACE_DIR_PORT  DDRB
#define     TRACE_OUT_PORT  PORTB

#define     TraceRxBitP     3
#define     TraceInt0P      2
#define     TraceBitErrP    1
#define     TraceStuffErrP  1
#define     TraceFormErrP   0


/**
 * Relays INIT MASK
 **/
#ifdef ReleB2P
#ifdef ReleB3P
#ifdef ReleB4P
#ifdef ReleB5P
#define REL_INIT_MASKB   (1<<ReleB1P)|(1<<ReleB2P)|(1<<ReleB3P)|(1<<ReleB4P)|(1<<ReleB5P)
#else
#define REL_INIT_MASKB   (1<<ReleB1P)|(1<<ReleB2P)|(1<<ReleB3P)|(1<<ReleB4P)
#endif
#else
#define REL_INIT_MASKB   (1<<ReleB1P)|(1<<ReleB2P)|(1<<ReleB3P)
#endif
#else
#define REL_INIT_MASKB   (1<<ReleB1P)|(1<<ReleB2P)
#endif
#else
#define REL_INIT_MASKB   (1<<ReleB1P)
#endif

#ifdef ReleA2P
#ifdef ReleA3P
#ifdef ReleA4P
#ifdef ReleA5P
#define REL_INIT_MASKA   (1<<ReleA1P)|(1<<ReleA2P)|(1<<ReleA3P)|(1<<ReleA4P)|(1<<ReleA5P)
#else
#define REL_INIT_MASKA   (1<<ReleA1P)|(1<<ReleA2P)|(1<<ReleA3P)|(1<<ReleA4P)
#endif
#else
#define REL_INIT_MASKA   (1<<ReleA1P)|(1<<ReleA2P)|(1<<ReleA3P)
#endif
#else
#define REL_INIT_MASKA   (1<<ReleA1P)|(1<<ReleA2P)
#endif
#else
#define REL_INIT_MASKA   (1<<ReleA1P)
#endif

#endif
