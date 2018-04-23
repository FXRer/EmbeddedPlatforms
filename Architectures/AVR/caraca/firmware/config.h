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

#define RESET_BLINK 1       /* blink the LED N times on startup */
#define CANTX_TEST  0       /* CAN Tx test: loop every second and xmit a different msg ID */
#define HARDWARE_TEST 0     /* 1 to perform functional test on startup */

#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check of the transmitted bit) */
#define NOACK_RETRY 0       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0
#define TRACERXERR  0
#define TRACEBUSOFF 0
#define TRACELOSTARB 0
#define TRACERESYNC 0       /* This should exclude the TRACEBITERR */
#define SCHEDULER_LED 0     /* 0 --> no led; 1 --> Load led; 2 --> Thread Leds */

#define SOFT_RESYNC 1       /* if 1 enable soft resyncronization on every falling edge: NEED MORE TESTS */
#define RX_DOUBLE_BUFFER 1  /* Use 2 Rx-Buffers alternativly */
#define TX_QUEUE    1       /* Use 4 Registers as Tx-Queue */

#define IR_DUMP     0       /* Enable promiscuous mode (raw RC5 code) */
#define KEY_DUMP    0       /* Enable promiscuous mode (raw key code) */
#define ENABLE_THERMOSTAT 0 /* Enable thermostat operations (thermometer always enabled) */

/****************************************
 * KERNEL
 ****************************************/

/** Set to 1 if you need priority scheduling (Thread 0 higher priority)
  * Set to 0 for round-robin scheduling **/
#define PRIORITY_SCHEDULE 1

/** If 1: Timer0 interrupt routine call Schedule() (NOT TESTED!!)
  * if 0: Timer0 interrupt act like other ISR (end with reti) **/
#define INTERRUPT_SCHEDULE  0

/** Number of active THREADS **/
#define NO_OF_THREAD  4

/** Stack RAM allocation for every thread **/
#define THR1_STACK  28
#define THR2_STACK  26
#define THR3_STACK  22
#define THR4_STACK  22

/* Priority of Threads, MUST be a different number for every thread
 * from 1 to NO_OF_THREAD */
#define PRI_CANBUSDRV   1
#define PRI_EDISPATCHER 2
#define PRI_RC5DECODER  3
#define PRI_THERMOMETER 4

#define THR_ID(x) (ThrVect + (x) - 1)

#define CANBUSDRV       THR_ID(PRI_CANBUSDRV)
#define EDISPATCHER     THR_ID(PRI_EDISPATCHER)
#define RC5DECODER      THR_ID(PRI_RC5DECODER)
#define THERMOMETER     THR_ID(PRI_THERMOMETER)


#define XTAL_FREQUENCY  8000000
#define TIMER0_PRESCALER    8

#define TIMER1_PRESCALER    1024
#define TIMER1_FREQUENCY    35
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
/*#define IrLockSig   0       * Ir wait for data to be processed */

/** Thr4 related **/
#define ThermoReqSig  0       /* Thermometer Request signal */

/** Common **/
#define CanReadySig 3       /* CanBus ready after power-up */
#define Timer0Sig   4       /* Timer0 overflow signal */
#define Timer1Sig   5       /* Timer1 Tick signal */
#define Int0Sig     6       /* Interrupt0 signal */
#define Int1Sig     7       /* Interrupt1 signal */


/***************************************************************************
 * REGISTERS <-> VARIABLES ASSIGNMENTS
 ***************************************************************************/

/* kernel vars */
/* R0: scratch register (used for LPM instruction) */

/* 4 bit ReadyThreadCounter - 4 bit RunningThread index */
#define KernelStatus    r1      /* current running thread */

#define KernelBits      r20     /* to save space this reg is used to store thread bits */
#define SleepStatus     7       /* 1 when sleep within schedule, 0 otherwise */

#define UserBits        r20     /* to save space share the same register with KernelBits */
#define lastIrBit       0
#define lastIrToggle    1
#define ThermoLockBit   2       /* thermometer acccess register lock */
#define RxLockBit1      3       /* CAN msg Rx Buffer 1 lock */
#define RxLockBit2      4       /* CAN msg Rx Buffer 2 lock */
#define TxLockBit       5       /* CAN msg Tx Buffer lock */

#define SaveStatus      r2      /* save SREG within Interrupt */
#define IntTmp          r16     /* interrupt scratch register */
#define IntTmp1         IntTmp  /* just an alias */

#define IntTmp2         r6      /* It's not strictly necessary, but save a push within Interrupt routines (fast response and less stack used) */

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

/* Ir RC5 vars */
#define IrIdx           r23

/* temperature vars */
#define ThermoSelect    r22
#if ENABLE_THERMOSTAT==1
#define ThermoTL        r5
#define ThermoTH        r9
#endif

/* key vars */
#define OldKeyStatus    r7
#define KeyChange       r8

#if TX_QUEUE==1
/* Event queue */
#define EqueueIDptr     r17     /* if 0 the queue is empty, otherwise point to eeprom location for MsgID */
#define Equeue1         r13
#define Equeue2         r14
#define Equeue3         r15
#endif

#if RX_DOUBLE_BUFFER==1
#define RxBufIdx        r24     /* a bit for every thread tell what RxBuffer they are using */
#endif

/*** available registers
 none
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
#define     SerRxP          0       /* In */
#define     SerTxP          1       /* Out */

#define     UART_DIR_PORT   DDRD
#define     UART_OUT_PORT   PORTD
#define     UART_IN_PORT    PIND


/* CAN Pins */
#define		CANRxInterrupt	1

#define     CanRxP          3       /* In  (INT1) */
#define		CAN_RXDIR_PORT	DDRD
#define     CAN_RXIN_PORT   PIND

#define     CanTxP          2       /* Out */
#define     CAN_TXDIR_PORT  DDRD
#define     CAN_TXOUT_PORT  PORTD

#define     CanSleepP       4       /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD


/* Relay Pins */
#define     ReleA1P         1       /* Out */
#define     ReleA2P         0       /* Out */

#define     RELEA_DIR_PORT  DDRD
#define     RELEA_OUT_PORT  PORTD

#define     ReleB1P         5       /* Out */
#define     ReleB2P         4       /* Out */
#define     ReleB3P         3       /* Out */
#define		ReleB4P			2		/* Out */
#define		ReleB5P			1		/* Out */

#define     RELEB_DIR_PORT   DDRC
#define     RELEB_OUT_PORT   PORTC
#define     RELEB_IN_PORT    PINC


/* Led Pins */
#if RESET_BLINK==1
#define     LedDrvP     5   /* Reset blink led */
#endif
/*#define     LedIrP      5*/   /* Ir activity led */
/*#define     LedBusRxP   5*/   /* CANBus Rx Led */
#define     LedBusTxP   5   /* CANBus Tx Led */
/*#define     LedDebugP   5*/   /* Debug led */
#if SCHEDULER_LED==1
#define     LedLoadP    5   /* Load vs. Idle cpu led */
#endif

#define     LED_DIR_PORT    DDRD
#define     LED_OUT_PORT    PORTD
#define     LED_IN_PORT     PIND


/* Key Pins */
#define     Key1P           5       /* Input + pullup */
#define     Key2P           4
#define     Key3P           3
#define     Key4P           0

#define     KEY_DIR_PORT    DDRB
#define     KEY_OUT_PORT    PORTB
#define     KEY_IN_PORT     PINB


/* InfraRed Receiver Pin */
#define     IrRxP           2       /* Input + pullup */
#define     IR_DIR_PORT     DDRB
#define     IR_OUT_PORT     PORTB
#define     IR_IN_PORT      PINB


/* I2C Bus pins */
#define     SCLP            7       /* SCL Pin number */
#define     SDAP            6       /* SDA Pin number */

#define     I2C_DIR_PORT    DDRD
#define     I2C_OUT_PORT    PORTD
#define     I2C_IN_PORT     PIND


/* DS1621 pins */
#if ENABLE_THERMOSTAT==1
#define     TOUTP           1       /* Thermostat output Pin number (Input) */

#define     TOUT_DIR_PORT   DDRB
#define     TOUT_OUT_PORT   PORTB
#define     TOUT_IN_PORT    PINB
#endif


/* Trace pins (only for debug) */
#define     TRACE_DIR_PORT  DDRC
#define     TRACE_OUT_PORT  PORTC

#define     TraceRxBitP     5
#define     TraceInt0P      4
#define     TraceBitErrP    3
#define     TraceStuffErrP  2
#define     TraceFormErrP   1

#define     TraceKeyChange  3
#define     TraceResyncNeg  2
#define     TraceResyncPos  1

/**
 * Key INIT MASK
 **/

#ifdef Key2P
#ifdef Key3P
#ifdef Key4P
#define KEY_INIT_MASK   (1<<Key1P)|(1<<Key2P)|(1<<Key3P)|(1<<Key4P)
#else
#define KEY_INIT_MASK   (1<<Key1P)|(1<<Key2P)|(1<<Key3P)
#endif
#else
#define KEY_INIT_MASK   (1<<Key1P)|(1<<Key2P)
#endif
#else
#define KEY_INIT_MASK   (1<<Key1P)
#endif

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
