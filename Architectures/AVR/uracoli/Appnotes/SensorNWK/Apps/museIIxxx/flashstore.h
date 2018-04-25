/*
 * flashstore.h
 *
 *  Created on: 07.04.2012
 *      Author: dthiele
 */

#ifndef FLASHSTORE_H_
#define FLASHSTORE_H_

void flashstore_enter(uint16_t size);
void flashstore_feed(uint8_t *chunk, uint8_t len);
void flashstore_leave(void);

#endif /* FLASHSTORE_H_ */
