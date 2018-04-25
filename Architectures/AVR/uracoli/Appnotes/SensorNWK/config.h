/*
 * config.h
 *
 *  Created on: 23.03.2013
 *      Author: dthiele
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define SENSORNWK_DEFAULT_CHANNEL (17)
#define SENSORNWK_DEFAULT_PANID (0x4422)
#define SENSORNWK_DEFAULT_ADDR (0x0)

/*
 * Frame Control Field of IEEE 802.15.4 MAC header
 */
#define SENSORNWK_DEFAULT_FCF (0x8861)

#define SENSORNWK_DEFAULT_COORD_ADDR (0x0000)

#define SENSORNWK_TIME_RX_SLOT_US (20000UL)

#endif /* CONFIG_H_ */
