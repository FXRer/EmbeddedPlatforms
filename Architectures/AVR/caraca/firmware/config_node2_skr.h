/** CONFIGURATION **/
#ifndef _CONFIG_H
#define _CONFIG_H

#define NODE_SKR
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
#define HARDWARE_TEST  0    /* 1 to perform functional test on startup */
#define SCHEDULER_LED  0    /* 1 to watch load (one LED - see LedLoadP)
			       2 to watch threads load (one LED per thread) */

#define SIMUL_DEBUG 0       /* Only useful with simulator (disable the check of the transmitted bit) */
#define NOACK_RETRY 0       /* 1 to disable the retry if no ack is received */
#define TRACERX_BIT 0
#define TRACEBITERR 0
#define TRACERESYNC 0       /* This should exclude the TRACEBITERR */

#define SOFT_RESYNC 1       /* if 1 enable soft resyncronization on every falling edge: NEED MORE TESTS */
#define RX_DOUBLE_BUFFER 0  /* Use 2 Rx-Buffers alternativly */
#define TX_QUEUE    0       /* Use 4 Registers as Tx-Queue */

#define IR_DUMP     1       /* Enable promiscuous mode (raw RC5 code) */
#define KEY_DUMP    0       /* Enable promiscuous mode (raw key code) */
#define ENABLE_THERMOSTAT 0 /* Enable thermostat operations (thermometer always enabled) */


extern TxSend
extern e2_Node_ID
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
#define TIMER1_FREQUENCY    10


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

#define UserBits        r20     /* to save space share the same register with KernelBits */
#define lastIrBit       0
#define lastIrToggle    1
#define ThermoLockBit   2       /* thermometer acccess register lock */
#define RxLockBit1      3       /* CAN msg Rx Buffer 1 lock */
;#define RxLockBit2      4       /* CAN msg Rx Buffer 2 lock */
#define TxLockBit       5       /* CAN msg Tx Buffer lock */

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
#define TOn             3     /* reset this to turn Dimmer off */ 
#define TSwitchOn       4     /* set this to turn Dimmer on */
#define TSynced         5     /* used during switching on to avoid flickering */
#define TBrightBit      6     /* Brightness (see Overflow-Int for explanation */
#define TBrightChange   7     /* to signal the Overflow-Int change of Brightness */
#define MAX_BRIGHTNESS  122   /* tried out, should be 100 to be more logical for user */

#define TBright         r9    /* Range: 0(off) to 0x8c(max) */


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
#undef TIME_QUANTUM
#define TIME_QUANTUM    XTAL_FREQUENCY/2000000
/* To allow 10 MHz (2313) and 8Mhz (4433) to communicate
 * This is more a hack, BITRATE should be the variable to set
 * and everything else should be fine, but this approach avoids rounding errors */


/***************************************************************************
 * Port Pin Assignements
 ***************************************************************************/

/* CAN Pins */
#define	    CANRxInterrupt	0

#define     CanRxP          2      /* In  (INT0) */
#define     CAN_RXDIR_PORT  DDRD
#define     CAN_RXIN_PORT   PIND

#define     CanTxP          1      /* Out */
#define     CAN_TXDIR_PORT  DDRD
#define     CAN_TXOUT_PORT  PORTD
#define     CanSleepP       4      /* Out (1 --> CAN82C250 in power save mode) */
#define     CAN_DIR_PORT    DDRD
#define     CAN_OUT_PORT    PORTD


/* Triac Pins */
#define     TriacSW         5      /* TODO PortC Out */

#define     TriacDIM        1      /* Out */

#define     TRIAC_DIR_PORT   DDRB
#define     TRIAC_OUT_PORT   PORTB
#define     TRIAC_IN_PORT    PINB


/* Led Pins */
#define     Led1P           5
#define     Led2P           6
#define     Led3P           7
#define     Led4P           0
#define     LED_DIR_PORT    DDRD
#define     LED_OUT_PORT    PORTD
#define     LED4_DIR_PORT   DDRB
#define     LED4_OUT_PORT   PORTB
#if SCHEDULER_LED == 1 
 #define    LedLoadP        Led1P   /* to see processor load (time not in sleep) */
#endif
#define     LedDebugP       Led3P   /* toggles on every accepted Message */
#define     LedDrvP         Led2P   /* toggles IR */

#define     HeatingP        5       /* Triac is connected here */ 
#define     HeatingFlag     2       /* unused port used as flag for HeatingP
				     * switch on during zero crossing */
/* Key Pins */
#define     Key1P           1       /* Input + pullup */
#define     Key2P           2
#define     Key3P           3
#define     Key4P           4

#define     KEY_DIR_PORT    DDRC
#define     KEY_OUT_PORT    PORTC
#define     KEY_IN_PORT     PINC


/* InfraRed Receiver Pin */
#define     IrRxP           0       /* Input + pullup */
#define     IR_DIR_PORT     DDRD
#define     IR_OUT_PORT     PORTD
#define     IR_IN_PORT      PIND


/* I2C Bus pins */
#define     SCLP            4       /* SCL Pin number */
#define     SDAP            5       /* SDA Pin number */

#define     I2C_DIR_PORT    DDRB
#define     I2C_OUT_PORT    PORTB
#define     I2C_IN_PORT     PINB


/* DS1621 pins */
#if ENABLE_THERMOSTAT==1
#define     TOUTP           3       /* Thermostat output Pin number (Input) */

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

#endif


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





