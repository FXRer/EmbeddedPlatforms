/*
 * lgee.h
 *
 * $Id$
 */

#ifndef LGEE_H_
#define LGEE_H_

#include <stdint.h>

#define LGEE_WORKCHANNEL (17)

/* Frame that is received by littleGee from Host
 *
 * Configure the node and its sensors
 */
typedef struct lgee_cfgframe_tag{
	uint16_t FCF;		/**< 802.15.4 frame control field */
	uint8_t seqnb;
	uint8_t smprate;	/**< sample rate 1..250 [Hz] */
	uint8_t gmode;		/**< g-mode of MMA7455 (2,4,8) */
	uint16_t crc;
}lgee_cfgframe_t;

/* Frame structure that is sent from littleGee to Host containing the sensor data
 *
 */
typedef struct lgee_dataframe_tag{
	uint16_t FCF;			/**< 802.15.4 frame control field */
	uint8_t seqnb;
	int8_t x;				/**< x-axis */
	int8_t y;				/**< y-axis */
	int8_t z;				/**< z-axis */
	uint8_t temperature;	/**< AVR integrated temperature sensor value */
	uint16_t crc;
}lgee_dataframe_t;

#endif /* LGEE_H_ */
