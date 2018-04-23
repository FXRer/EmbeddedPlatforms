/** CONFIGURATION **/
#ifndef _CONFIG_H
#define _CONFIG_H

#define NODE_SKR


/****************************************
 * DEBUG AND/OR CODE SWITCH FLAGS
 ****************************************/

#define RESET_BLINK 0       /* blink the LED 3 times on startup */
#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check for transmitted bit) */
#define NOACK_RETRY 0       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0
#define SOFT_RESYNC 0       /* if 1 enable soft resyncronization on every falling edge: NEED TEST */

#arch AT90S2313 

extern TxSend
extern e2_Node_ID
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
#define THR2_STACK  24
;#define THR3_STACK  20
;#define THR4_STACK  22

#define THR_ID(x) (ThrVect + (x) - 1)

#define CANBUSDRV       THR_ID(1)
#define EDISPATCHER     THR_ID(2)
;#define RC5DECODER      THR_ID(3)
;#define THERMOMETER     THR_ID(4)


#define XTAL_FREQUENCY  10000000
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
#define NewTempSig  3       /* New Temperature is ready */

/** Thr3 related **/
#define IrLockSig   0       /* Ir wait for data to be processed */

/** Thr4 related **/
#define	TempReqSig  0       /* Read temperature request */
#define SetThrSig   1       /* Write thermostat threshold */
#define GetThrSig   2       /* Read thermostat threshold */

/** Common **/
#define Timer0Sig   4       /* Timer0 overflow signal */
#define Timer1Sig   5       /* 1s Tick signal - (after 100 Zero crossings)*/

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
#define lastTxbit       6
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

/** Triac vars **/
#define TriZC           r14    /* During zero crossing this holds the timer count */
#define TriZCnew        r15    
#define Timer           r13    /* 8Bit Timebase ZeroCrossing int 
				  wraps every 5,12sek (mains freq=50HZ)*/

#define TriacBits       r17    
#define TOV1Cnt         0     /* count Timer1 Overflows during ZCD ints 
				 normally 4 (so we need 3 Bits) */ 
#define TOn             3      
#define TSwitchOn       4
#define TSynced         5
#define TBrightBit      6     /* Brightness (see Overflow-Int for explanation */
#define TBrightChange   7     /* to signal the Overflow-Int change of Brightness */
#define TempSelect      r9

/* Ir RC5 vars */
#define IrCount         r22
#define IrIdx           r23
#define IrDataH         r5
#define IrDataL         r24

#define UserBits        r20	/* Bits 2 to 3 used for Triacs! */
#define lastIrBit       0
#define TempLockBit     1       /* temperature acccess register lock (Write config, Read config) */
#define TxLockBit       5       /* CAN msg Tx Buffer lock */
#define RxLockBit       6       /* CAN msg Rx Buffer lock */
#define RxLockBit1      6       /* CAN msg Rx Buffer lock */


/* key vars */
#define OldKeyStatus    r7
#define KeyChange       r8

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


/*******************************************
 * CANBus Message Values
 *******************************************/

#define MsgKey  0x20
#define MsgDim  0x40
#define MsgTemp 0xA0

/***************************************************************************
 * Port Pin Assignements
 ***************************************************************************/

/* CAN Pins */
#define     CanRxP          2      /* In  (INT0) */
#define     CAN_RXDIR_PORT  DDRD
#define     CAN_RXIN_PORT   PIND

#define     CanTxP          1      /* Out */
#define     CAN_TXDIR_PORT  DDRD
#define     CAN_TXOUT_PORT  PORTD
#define     CanSleepP       0      /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD


/* Triac Pins */
#define     TriacSW         2      /* Out */
#define     TriacDIM        3      /* Out */

#define     TRIAC_DIR_PORT   DDRB
#define     TRIAC_OUT_PORT   PORTB
#define     TRIAC_IN_PORT    PINB


/* Led Pins */
#define     Led1P         6
#define     Led2P         6
#define     Led3P         6
#define     LedDrvP         6
#define     LedLoadP        6
#define     LedDebugP       6
#define     LED_DIR_PORT    DDRD
#define     LED_OUT_PORT    PORTD
#define     LED_IN_PORT     PIND


/* Key Pins */
#define     Key1P           5       /* Input + pullup */
#define     Key2P           4
#define     Key3P           4
#define     Key4P           4

#define     KEY_DIR_PORT    DDRD
#define     KEY_OUT_PORT    PORTD
#define     KEY_IN_PORT     PIND


/* InfraRed Receiver Pin */
#define     IrRxP           4       /* Input + pullup */
#define     IR_DIR_PORT     DDRB
#define     IR_OUT_PORT     PORTB
#define     IR_IN_PORT      PINB


/* I2C Bus pins */
#define     SCLP            6       /* SCL Pin number */
#define     SDAP            7       /* SDA Pin number */

#define     I2C_DIR_PORT    DDRB
#define     I2C_OUT_PORT    PORTB
#define     I2C_IN_PORT     PINB


/* DS1621 pins */
#define     TOUTP           5       /* Thermostat output Pin number (Input) */

#define     TOUT_DIR_PORT   DDRB
#define     TOUT_OUT_PORT   PORTB
#define     TOUT_IN_PORT    PINB


#endif
