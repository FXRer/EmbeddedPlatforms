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

#define ICALL_TABLE 1       /* call table vs. if then else */
#define RESET_BLINK 1       /* blink the LED 3 times on startup */
#define CANTX_TEST  0       /* CAN Tx test: loop every second and xmit a different msg ID */

#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check of the transmitted bit) */
#define NOACK_RETRY 0       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0

#define SOFT_RESYNC 0       /* if 1 enable soft resyncronization on every falling edge: NEED TEST */

/****************************************
 * KERNEL
 ****************************************/

/** Set to 1 if you need priority scheduling (Thread 0 higher priority)
  * Set to 0 for round-robin scheduling **/
#define PRIORITY_SCHEDULE   0

/** If 1: Timer0 interrupt routine call Schedule() (short responses)
  * if 0: Timer0 interrupt act like other ISR (end with reti) **/
#define INTERRUPT_SCHEDULE  0

/** Number of active THREADS **/
#define NO_OF_THREAD  3

/** Stack RAM allocation for every thread **/
#define THR1_STACK  28
#define THR2_STACK  24
#define THR3_STACK  20
#define THR4_STACK  22

#define THR_ID(x) (ThrVect + (x) - 1)

#define CANBUSDRV       THR_ID(1)
#define EDISPATCHER     THR_ID(2)
#define RC5DECODER      THR_ID(3)
#define THERMOMETER     THR_ID(4)


#define XTAL_FREQUENCY      10000000
#define TIMER0_PRESCALER    8


/*****************************************
 * SIGNALS
 *****************************************/

/** Thr1 related **/
#define CanReqSig   0       /* CAN request signal */
#define CanSOFSig   1       /* CAN StartOfFrame signal */

/** Thr2 related **/
#define IrDataSig   0       /* Valid RC-5 code received */
#define CanDataSig  1       /* Valid CAN message received */
#define KeyDataSig  2       /* Valid keystroke */

/** Thr3 related **/
#define IrLockSig   0       /* Ir wait for data to be processed */

/** Thr4 related **/
#define TempReqSig  0       /* Temperature Request signal */
#define SetThrSig   1       /* Write thermostat threshold */
#define GetThrSig   2       /* Read thermostat threshold */

/** Common **/
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
#define SleepStatus     7       /* 1 when sleep withink schedule, 0 otherwise */

#define UserBits        r20     /* to save space share the same register with KernelBits */
#define lastIrBit       0
#define TempLockBit     1       /* temperature acccess register lock (Write config, Read config) */
#define RxLockBit       2       /* CAN msg Rx Buffer lock */
#define TxLockBit       3       /* CAN msg Tx Buffer lock */

#define SaveStatus      r2      /* save SREG within Interrupt */
#define IntTmp          r16     /* interrupt scratch register */
#define IntTmp1         IntTmp  /* just an alias */

#define IntTmp2         r6      /* It's not strictly necessary, but save a push within Interrupt routines (fast responese and less stack) */

/** CRC vars **/
#define CrcRegL         r3
#define CrcRegH         r4

/** CAN vars **/
#define TxErrcounter    r12
#define RxErrcounter    r18

#define TxBitcounter    r19

#define CanBits         r21
#define CanMaster       7       /* tell if we are master or slave in current frame */
#define lastTxbit       6        /* don't change it!!! */
#define txBitstuffing   5       /* bit stuffing enabled/disabled */
#define Rxbit           4       /* CAN Rx sampled in timer intr */
#define HardSync        3       /* if 1 HardSync, else SoftSync */
                                /* note that hard sync is performed only at SOF,
                                 * so this bit tell if we are within a frame or not */
#define ErrorPassive    2       /* 1 - Error Passive, 0 - Error Active */
#define StuffError      1       /* 1 - Stuff error, 0 - bit error */
                                /* bit 0 available */
/**
#define CanError        r9

#define BitErrCode      0x01
#define StuffErrCode    0x02
#define FrameErrCode    0x03
#define NoAckErrCode    0x04
#define CrcErrCode      0x05
**/

#define StuffArbRtr     5       /* Stuff error during arbitration before RTR bit (exception 2) */
#define PassNoAck       6       /* Error passive get NoAck (exception 1) */

/* Store the received word here, even when we're transmitting, so we can switch between
 * master transmitter and slave receiver without lose current word */
#define CanRxWordH      r10
#define CanRxWordL      r11

/* Ir RC5 vars */
#define IrCount         r22
#define IrIdx           r23
#define IrDataH         r5
#define IrDataL         r24

/* key vars */
#define OldKeyStatus    r7
#define KeyChange       r8

/* temperature vars */
#define TempRegH        r13
#define TempRegL        r14

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

#define     CanTxP          1      /* Out */
#define		CAN_TXDIR_PORT	DDRD
#define		CAN_TXOUT_PORT	PORTD

#define     CanSleepP       0      /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD


/* Relay Pins */
#define     ReleA1P         0      /* Out */
#define     ReleA2P         1      /* Out */
#define		ReleB1P			0
#define		ReleB2P			1
#define     ReleB3P         2      /* Out */
#define     ReleB4P         3      /* Out */
#define		ReleB5P			3

#define     RELEA_DIR_PORT   DDRB
#define     RELEA_OUT_PORT   PORTB
#define     RELEA_IN_PORT    PINB

#define     RELEB_DIR_PORT   DDRB
#define     RELEB_OUT_PORT   PORTB
#define     RELEB_IN_PORT    PINB


/* Led Pins */
#define     LedDrvP         6
#define     LED_DIR_PORT    DDRD
#define     LED_OUT_PORT    PORTD
#define     LED_IN_PORT     PIND


/* Key Pins */
#define     Key1P           7       /* Input + pullup */
#define     Key2P           6
#define     Key3P           5
#define     Key4P           4

#define     KEY_DIR_PORT    DDRB
#define     KEY_OUT_PORT    PORTB
#define     KEY_IN_PORT     PINB


/* InfraRed Receiver Pin */
#define     IrRxP           4       /* Input + pullup */
#define     IR_DIR_PORT     DDRB
#define     IR_OUT_PORT     PORTB
#define     IR_IN_PORT      PINB


/* I2C Bus pins */
#define     SCLP            5       /* SCL Pin number */
#define     SDAP            4       /* SDA Pin number */

#define     I2C_DIR_PORT    DDRD
#define     I2C_OUT_PORT    PORTD
#define     I2C_IN_PORT     PIND


/* DS1621 pins */
#define     TOUTP           3       /* Thermostat output Pin number (Input) */

#define     TOUT_DIR_PORT   DDRD
#define     TOUT_OUT_PORT   PORTD
#define     TOUT_IN_PORT    PIND

/* Trace pins (only for debug) */
#define     TRACE_DIR_PORT  DDRB
#define     TRACE_OUT_PORT  PORTB

#define     TraceRxBitP     3
#define     TraceInt0P      2
#define     TraceBitErrP    1
#define     TraceStuffErrP  1
#define     TraceFormErrP   0

#endif

