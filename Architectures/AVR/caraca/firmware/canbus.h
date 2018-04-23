#ifndef _CANBUS_H
#define _CANBUS_H

/* CAN Bus constants and definitions */

/**
 * Here is how Timer0 is used for CANBus timing
 * Remember Timer0 run from 0 to FF and then raise
 * the Tim0Interrupt.
 *
 *   Bit Time
 *   +---------+----------+----------+---------+
 *   | PH2_SEG | SYNC_SEG | PROP_SEG | PH1_SEG |
 *   +---------+----------+----------+---------+
 *   ^              ^                          ^
 * TimReload      ExtInt                     TimInt
 * TIM0_RELOAD   TIM0_SYNC               SAMPLING_POINT
 **/

#define SYNC_SEG_SIZE       (x_SYNC_SEG * TIME_QUANTUM)
#define PROP_SEG_SIZE       (x_PROP_SEG * TIME_QUANTUM)
#define PH1_SEG_SIZE        (x_PHASE1_SEG * TIME_QUANTUM)
#define PH2_SEG_SIZE        (x_PHASE2_SEG * TIME_QUANTUM)

#define RJW                 (x_RESYNC_JW * TIME_QUANTUM)

#define BIT_TIME            (SYNC_SEG_SIZE + PROP_SEG_SIZE + PH1_SEG_SIZE + PH2_SEG_SIZE)
#define SYNC_SEG            (256 - BIT_TIME + PH2_SEG_SIZE)
#define PROP_SEG            (SYNC_SEG + SYNC_SEG_SIZE)
#define PH1_SEG             (PROP_SEG + PROP_SEG_SIZE)
#define SWITCH_POINT        (SYNC_SEG)
#define MIDDLE_SWITCH_POINT ( SYNC_SEG + SYNC_SEG_SIZE / 2 )
#define SAMPLING_POINT      0

#define EDGE1               SYNC_SEG
#define EDGE2               PROP_SEG

#define TIM0_RELOAD         (256 - BIT_TIME)
#define TIM0_SYNC           MIDDLE_SWITCH_POINT

#define ClrCanTx    cbi CAN_TXOUT_PORT,CanTxP
#define SetCanTx    sbi CAN_TXOUT_PORT,CanTxP

#define BITRATE             ( XTAL_FREQUENCY / (TIMER0_PRESCALER * BIT_TIME) )
 
#endif
