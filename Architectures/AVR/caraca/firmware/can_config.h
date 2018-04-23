#ifndef _CAN_CONFIG_H
#define _CAN_CONFIG_H

/* CAN Bus configuration */

/* This is a sort of CAN Bus prescaler */
#define	TIME_QUANTUM	5
/* This must be at least 5 to let SOFT resync algorithm to work */

/* Syncseg size MUST be 1 */
#define x_SYNC_SEG      1

/* All other threads run within PROP+PHASE1 segment, if it's too small some threads can't
 * run at all when the CANBus is heavy loaded */

/* Prop segment depend on propagation delay (mainly driver delay and cable length) */
#define x_PROP_SEG      3

#define x_PHASE1_SEG    5

/* This have to be large because Canbus thread process next bit
 * within PHASE2_SEG - RESYNC_JW */
#define x_PHASE2_SEG    7

#define x_RESYNC_JW     2

#endif
