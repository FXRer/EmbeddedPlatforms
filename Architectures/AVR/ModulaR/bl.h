#ifndef _BL_H_
#define _BL_H_

#include <stdbool.h>
#include <stdint.h>
#include "commsPacket.h"


#define BL_CAP_FLAGS_FTR_OUT_SUPPORTED					0x01
#define BL_CAP_FLAGS_NEED_PADDING						0x02
#define BL_CAP_FLAGS_CDC_SUPPORTED						0x04
#define BL_CAP_BL_VERSION_REPORTING						0x08

#define packet_sz_t uint8_t


#define PACKET_RX_FAIL			((packet_sz_t)-1LL)

#define CDC_STOP_BITS_1		0
#define CDC_STOP_BITS_1_5	1
#define CDC_STOP_BITS_2		2

#define CDC_PARITY_NONE		0
#define CDC_PARITY_ODD		1
#define CDC_PARITY_EVEN		2
#define CDC_PARITY_MARK		3
#define CDC_PARITY_SPACE	4

struct CdcAcmLineCoding {
	uint32_t baudrate;
	uint8_t nStopBits;	//0->1, 2->1.5, 2->2
	uint8_t party;		//none, odd, even, mark, space
	uint8_t dataBits;
} __attribute__((packed));

typedef void (*BlCdcCfgF)(struct CdcAcmLineCoding *cfg);	//modifies cfg to match what was actualyl set
typedef void (*BlCdcBulkRxF)(void *data, uint32_t len);


bool packetCanSend(void);
void packetSend(const struct CommsPacket *pkt, packet_sz_t payloadLen);
packet_sz_t packetRx(struct CommsPacket **dst);
void packetRxRelease(void);
void usbWork(void);
void bootloader(bool withTimeout);
void getUsbCaps(packet_sz_t *maxPacketSzP, uint8_t *flagsP);	//BL_CAP_FLAGS_*
void usbReenumerate(const void *newDeviceStrDescr, const void *newSnumStrDescr);
uint8_t blGetBlVersion(void);		//only if BL_CAP_BL_VERSION_REPORTING


#endif
