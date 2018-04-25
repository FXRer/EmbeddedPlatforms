/*
 * ost.h
 *
 *  Created on: 28.02.2011
 *      Author: dthiele
 */

#ifndef OST_H_
#define OST_H_

typedef void (ost_cb_t)(void);


typedef enum{
	OST_ONESHOT,
	OST_PERIODIC
}ost_type_t;

void ost_start(ost_cb_t *cb, uint32_t duration_us, ost_type_t type);
void ost_restart(void);
uint32_t ost_stop();
uint32_t ost_getperiod();

#endif /* OST_H_ */
