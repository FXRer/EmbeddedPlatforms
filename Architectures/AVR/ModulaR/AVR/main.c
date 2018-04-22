#include "usbconfig.h"
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include "../commsPacket.h"
#include "usbdrv.h"
#include "osccal.h"
#include "flash.h"
#include "../speck.h"
#include "../bl.h"






//ModulaR bootloader
// (c) 2018 Dmitry.GR

#define BL_VER					2
#define USB_PACKET_SZ			8

extern SPECK_DATA_TYPE __key_start[], __key_end[];


const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor, size must match usbconfig.h */

	0x06, 0x00, 0xFF,												// Usage Page = 0xFF00 (Vendor Defined Page 1)
	0x09, 0x01,														// USAGE (Vendor Usage 1)
	
	0xA1, 0x01,														// Collection (Application)
	0x85, 0x01,														// !!! all that follows is report #1
	0x15, 0x01,						 								//	 Logical Minimum (1)
	0x26, 0xFF, 0x00,				   								//	 Logical Maximum (255)
	0x19, 0x01,														//   Usage Minimum (1)
	0x29, 0xFF,														//	 Usage Maximum (255)
	0x75, 0x08,						 								//	 Report Size (8 bit)
	0x95, MODULAR_MAX_PACKET,										//   REPORT_COUNT (packet sz)
	0x81, 0x00,							 							//	 Input (Data, Array, Abs)
	0x15, 0x01,						 								//	 Logical Minimum (1)
	0x26, 0xFF, 0x00,				   								//	 Logical Maximum (255)
	0x19, 0x01,														//   Usage Minimum (1)
	0x29, 0xFF,														//	 Usage Maximum (255)
	0x75, 0x08,						 								//	 Report Size (8 bit)
	0x95, MODULAR_MAX_PACKET,										//   REPORT_COUNT (packet sz)
	0x91, 0x00,                        								//   Output (Data, Array, Abs)
	0xC0,

};

PROGMEM static const char mUsbDescriptorString0[] = { /* language descriptor */
    4,          /* sizeof(usbDescriptorString0): length of descriptor in bytes */
    3,          /* descriptor type */
    0x09, 0x04, /* language index (0x0409 = US-English) */
};

PROGMEM static const uint16_t  mUsbDescriptorStringVendor[] = {
    USB_STRING_DESCRIPTOR_HEADER(9),
    'd', 'm', 'i', 't', 'r', 'y', '.', 'G', 'R'
};

PROGMEM static const uint16_t  mUsbDescriptorStringDevice[] = {
    USB_STRING_DESCRIPTOR_HEADER(10),
    'M', 'o', 'd', 'u', 'l', 'a', 'R', ' ', 'B', 'L' 
};

static uint16_t mUsbDescriptorStringSnum[] = {
    USB_STRING_DESCRIPTOR_HEADER(0), 
};


static void *mDescrs[] = {(void*)mUsbDescriptorString0, (void*)mUsbDescriptorStringVendor, (void*)mUsbDescriptorStringDevice, (void*)mUsbDescriptorStringSnum};
static uint8_t mBufIn[MODULAR_MAX_PACKET + 1], mBufOut[MODULAR_MAX_PACKET + 1];
struct Flags {
	uint8_t haveIn: 1;
	uint8_t useWinPad: 1;
};

//extreme space saving as these are faster to access and use less code
#define mHaveBytes GPIOR1
#define mOutBufPos GPIOR2
#define mOutBytesLeft OCR0A
#define mFlags ((volatile struct Flags*)&GPIOR0)



usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq)
{
	uint8_t len, idx = rq->wValue.bytes[0];
	void *descr;
	
	if (idx > 3)
		return 0;
	
	descr = mDescrs[idx];
	usbMsgPtr = (usbMsgPtr_t)descr;
	
	if (idx == 3) 	//snum - in ram
		len = *(uint8_t*)descr;
	else			//others - in rom
		len = pgm_read_byte(descr);

	return len;
}

void usbFunctionWriteOut(uint8_t *data, uint8_t len)
{
	uint8_t i;

/*
	//only EP 2 is writes accepted
	if (usbRxToken != 2)
		return;
*/	
	//cannot accept data if still processing last data packet
	if (mFlags->haveIn)
		return;
	
	//too big to fit?
	if ((uint8_t)(sizeof(mBufIn) - mHaveBytes) < len) {
		mHaveBytes = 0;
		return;
	}
	
	//copy it in
	for (i = 0; i < len; i++)
		mBufIn[mHaveBytes++] = *data++;

	//if not end of app packet, we're done
	if (len == USB_PACKET_SZ)
		return;
	
	mFlags->haveIn = 1;
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void *)data;
	
	/* The following requests are never used. But since they are required by
	* the specification, we implement them in this example.
	*/
	
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
	
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			return 0;
		}
		else if(rq->bRequest == USBRQ_HID_SET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			return 0;
		}
		else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = data;	//who cares?
			return 1;
		}
		else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			//idleRate = rq->wValue.bytes[1];
		}
	}
	else{
		/* no vendor specific requests implemented */
	}
	return 0;   /* default for not implemented requests: return no data back to host */
}

void usbEventResetReady(void)
{
	/* Disable interrupts during oscillator calibration since
	 * usbMeasureFrameLength() counts CPU cycles.
	 */
	cli();
	calibrateOscillator();
	sei();
}

bool packetCanSend(void)
{
	return mOutBytesLeft == 0;
}

static __attribute__((noinline)) uint8_t doCommsPacketCalcCrc(const struct CommsPacket *p, uint8_t payloadLen)	//noinline for size
{
	return commsPacketCalcCrc(p, payloadLen);
}

void packetSend(const struct CommsPacket *pkt, uint8_t payloadLen)	//YOU must make sure you CAN send
{
	struct CommsPacket *dstForCmdAndData = (struct CommsPacket*)(mBufOut + 1);
	struct CommsPacket *dstForCrc = (struct CommsPacket*)(mBufOut + 1);
	uint8_t payloadLenForCrcAndTx = payloadLen;
	uint8_t i;
	
	
	//win-pad as needed
	if (mFlags->useWinPad) {
		struct CommsWinPacket *win = (struct CommsWinPacket*)(mBufOut + 1);
		
		for (i = 0; i < sizeof(mBufOut); i++)		//do not leak anything
			mBufOut[i] = 0;
		win->cmd = CMD_WIN_PADDED;
		win->winHdr.actualPayloadLen = payloadLen;
		payloadLenForCrcAndTx = MODULAR_MAX_PACKET - sizeof(struct CommsPacket);		//always max sz
		dstForCmdAndData = &win->actualPacket;
	}
	
	dstForCmdAndData->cmd = pkt->cmd;
	for (i = 0; i < payloadLen; i++)
		dstForCmdAndData->payload[i] = pkt->payload[i];
	dstForCrc->crc = doCommsPacketCalcCrc(dstForCrc, payloadLenForCrcAndTx);
	
	mBufOut[0] = 1;	//report id
	mOutBufPos = 0;
	mOutBytesLeft = payloadLenForCrcAndTx + sizeof(struct CommsPacket) + 1 /* report id */ + 1 /* due to how we send things */;
}

void packetRxRelease(void)
{
	mHaveBytes = 0;
	mFlags->haveIn = 0;
}

uint8_t packetRx(struct CommsPacket **dst)	//return payload len or PACKET_RX_FAIL if none
{
	struct CommsPacket *pkt = (struct CommsPacket*)(mBufIn + 1);	//ignore report descriptor
	uint8_t payloadLen, haveBytes;

	if (!mFlags->haveIn)
		return PACKET_RX_FAIL;
	
	haveBytes = mHaveBytes - 1;		//report id
	payloadLen = haveBytes - sizeof(struct CommsPacket);
	
	
	if (haveBytes >= sizeof(struct CommsPacket)) {
		if (pkt->cmd & CMD_ORR_PADDED) {
			if (!payloadLen)
				goto fail;
			payloadLen--;
			pkt->cmd &=~ (uint8_t)CMD_ORR_PADDED;
		}
		
		if (pkt->crc != doCommsPacketCalcCrc(pkt, payloadLen))
			goto fail;
	
		//win padded packet processing
		if (pkt->cmd == CMD_WIN_PADDED) {
			struct CommsWinPacket *winPkt = (struct CommsWinPacket*)pkt;
	
			//forever use win padding now
			mFlags->useWinPad = 1;
			*dst = &winPkt->actualPacket;
			return winPkt->winHdr.actualPayloadLen;
		}
	
		*dst = pkt;
		return payloadLen;
	}

fail:
	packetRxRelease();
	return PACKET_RX_FAIL;
}


void usbWork(void)
{
	usbPoll();
	
	if (usbConfiguration && usbInterruptIsReady() && mOutBytesLeft) {
		
		uint8_t now, realOutBytesLeft = mOutBytesLeft - 1;
		uint8_t *ptr = mBufOut + mOutBufPos;
		
		if (realOutBytesLeft >= USB_PACKET_SZ) {
			now = USB_PACKET_SZ;
			mOutBytesLeft -= USB_PACKET_SZ;
		}
		else {
			now = realOutBytesLeft;
			mOutBytesLeft = 0;
		}

		mOutBufPos += now;
		usbSetInterrupt(ptr, now);
	}
}

static void __attribute__((naked,noreturn)) callWithNewStack(void* param, void* ptr)
{
	asm volatile(
		"cli					\r\n"
		"ldi  r28, 0x5F			\r\n"
		"out  0x3D, r28	;SPL	\r\n"
		"ldi  r28, 0x02			\r\n"
		"out  0x3E, r28	;SPH	\r\n"
		"sei					\r\n"
		"movw r30, r22			\r\n"
		"ijmp					\r\n"
	);
	
	(void)param;
	(void)ptr;
}

void __attribute__((naked)) rol1xor(SPECK_DATA_TYPE *ck, SPECK_DATA_TYPE *data)
{
	asm volatile(
		"	clc				\n"
		"	ldi  r20, %0	\n"
		"	movw r30, r24	\n"	//Z = ck
		"	movw r26, r22	\n"	//X = data
		"1:					\n"
		"	ld   r18, Z		\n"
		"	ld   r19, X+	\n"
		"	rol	 r18		\n"
		"	eor  r18, r19	\n"
		"	st	 Z+, r18	\n"
		"	dec	 r20		\n"
		"	brne 1b			\n"
		
		"	movw r30, r24	\n"	//Z = ck  (rewind)
		
		"	clr  r21		\n"
		"	adc  r21, r1	\n"	//grab C into r0
		
		"	ld   r20, Z		\n"
		"	eor	 r20, r21	\n"
		"	st	 Z, r20		\n"
		
		"	ret				\n"
		:
		:"M"((uint8_t)sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]))
	);
	
	//shut up gcc
	(void)ck;
	(void)data;
}

static void blPoll(uint16_t *eraseAddrP, uint16_t *writeAddrP, uint16_t *firstWordP, SPECK_DATA_TYPE *checksumP)
{
	bool haveEncr = __key_start != __key_end;
	struct CommsPacket* pkt;
	uint8_t payloadSz, i;
	uint16_t addr;
	
	if (packetCanSend() && ((payloadSz = packetRx(&pkt)) != PACKET_RX_FAIL))	//order is important here
	{
		switch (pkt->cmd) {
			case COMMS_CMD_SECURE_ERASE: {
				addr = *eraseAddrP;

				if (addr) {
					
					flashErase(addr);
					addr += FLASH_BLOCK_SZ;
					
					if (addr - APP_START == APP_LEN) {
						addr = 0;
						pkt->payload[0] = 1;
					}
					else
						pkt->payload[0] = 0;
					
					*eraseAddrP = addr;
				}
				goto one_byte_payload_reply;
				break;
			}
			case COMMS_CMD_WRITE_BLOCK: {
				struct CommsWritePacket *wpkt = (struct CommsWritePacket*)pkt->payload;
				bool ret = false;
				
				if (payloadSz != sizeof(struct CommsWritePacket))
					break;

				if (*eraseAddrP)
					goto writedone;

				if (!*writeAddrP)
					goto writedone;

				if (haveEncr) {
					SPECK_DATA_TYPE *iv = wpkt->iv, *ptr = (SPECK_DATA_TYPE*)wpkt->data;
					
					//PCBC mode
					for (i = 0; i < WRITE_SIZE; i += sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]), ptr += SPECK_DATA_WORDS) {
						
						speckDecrCbc(ptr, iv, __key_start);
						
						if (!(wpkt->flags & FLAG_UPLOAD_DONE))
							rol1xor(checksumP, ptr);
					}
				}
			
				//check for write done before checking for overflow since else we can never use last flash block
				if (wpkt->flags & FLAG_UPLOAD_DONE) {
					
					*writeAddrP = 0;
					
					if (haveEncr) {
						
						//verify checksum is valid
						for (i = 0; i < (uint8_t)sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]); i++)
							if (wpkt->data[i] != ((uint8_t*)checksumP)[i])
								goto writedone;
						
						//verify other bytes are zeroes as expected
						for (; i < WRITE_SIZE; i++)
							if (wpkt->data[i])
								goto writedone;
					}
					
					//success? write first word now
					for (i = 2; i < WRITE_SIZE; i++)
						wpkt->data[i] = 0xFF;
					*(uint16_t*)wpkt->data = *firstWordP;
					addr = APP_START;
				}
				else {
					addr = *writeAddrP;
					if (addr - APP_START >= APP_LEN)	//too much upload
						goto writedone;
					*writeAddrP = addr + FLASH_BLOCK_SZ;
					
					if (*firstWordP == 0xFFFF) {	//if first word, save first it and do not flash it
						
						*firstWordP = *(uint16_t*)wpkt->data;
						*(uint16_t*)wpkt->data = 0xFFFF;
					}
				}
				
				flashWrite(addr, wpkt->data);
				ret = true;
				
		writedone:
				pkt->payload[0] = ret;
				TCCR1 = 0;	//if we had a counter, stop it now
				
		one_byte_payload_reply:
				payloadSz = 1;
				break;
			}
			case COMMS_CMD_BOOT_APP: {
				if (payloadSz != 0)
					break;
				
				//reply only sent if we do not load app
				pkt->payload[0] = 0;
				
				//do not boot if no image there
				if (pgm_read_word(APP_START) != 0xFFFF)
					callWithNewStack(NULL, (void*)(APP_START / 2));
				
				goto one_byte_payload_reply;
				break;
			}
			case COMMS_CMD_GET_INFO: {
				static const PROGMEM struct CommsInfoPacket nfo = {.magix = BL_INFO_MAGIX, .base = APP_START, .size = APP_LEN, .blockSz = FLASH_BLOCK_SZ, .blVer = BL_VER, .protoVer = BL_PROT_VER_CUR, .flags = BL_FLAGS_NEED_PADDING, .info = "ModulaR BL!"}; 
				struct CommsInfoPacket *dst = (struct CommsInfoPacket*)pkt->payload;
				const uint8_t* bufP = (const uint8_t*)&nfo;
				
				for (payloadSz = 0; payloadSz < (int8_t)sizeof(nfo); payloadSz++)
					pkt->payload[payloadSz] = pgm_read_byte(bufP++);
				
				//winPad supercedes 1-byte padding for 8-byte boundary fixing as out full packet sz is never a multiple if 8
				//but we'll let the host figure that shit out, because why should we have to?
				
				if (haveEncr)
					dst->flags |= BL_FLAGS_NEED_ENCR;
				
				TCCR1 = 0;	//if we had a counter, stop it now
				break;
			}
			default: {
				goto noreply;
				break;
			}
		}
		
		packetSend(pkt, payloadSz);
	
	noreply:
		packetRxRelease();
	}
}

static void bootloaderGuts(void *withTimeout)
{
	uint16_t eraseAddr = APP_START, writeAddr = APP_START, firstWord = 0xffff;
	SPECK_DATA_TYPE checksum[SPECK_DATA_WORDS] = {0,};
	uint8_t cyLeft = 10;	//2 second or so
	
	mOutBufPos = 0;
	mOutBytesLeft = 0;
	mHaveBytes = 0;
	*(uint8_t*)mFlags = 0;	//clear "haveIn" and "useWinPad"
	//implied by above:	packetRxRelease();
	
	if (withTimeout) {
		TCCR1 = 0x0F;			//clk = CPU / 16384
		GTCCR = 2;				//reset prescaler
		TCNT1 = 0;				//reset counter
		TIFR = 4;				//clear overflow flag
	}
		
	while (1) {
	
		usbWork();
		blPoll(&eraseAddr, &writeAddr, &firstWord, checksum);
		
		if (TIFR & 4) {			//happens every 256 ms
			
			TIFR = 4;			//clear overflow flag
			if (!--cyLeft)
				break;
		}
	}
	
	callWithNewStack(NULL, (void*)(APP_START / 2));
}

void bootloader(bool withTimeout)
{
	callWithNewStack((void*)withTimeout, &bootloaderGuts);
}

int __attribute__((noreturn)) main(void)
{
	wdt_reset();
	wdt_disable();
	
	usbInit();
	sei();

	bootloader(pgm_read_word(APP_START) != 0xFFFF);
	
	while(1);
}

void getUsbCaps(packet_sz_t *maxPacketSzP, uint8_t *blFlagsP)
{
	*maxPacketSzP = MODULAR_MAX_USEFUL_BYTES;
	*blFlagsP = BL_CAP_FLAGS_NEED_PADDING | BL_CAP_BL_VERSION_REPORTING;
}

uint8_t blGetBlVersion(void)
{
	return BL_VER;
}

void usbReenumerate(const void *newDeviceStrDescr, const void *newSnumStrDescr)
{
	mDescrs[2] = (void*)newDeviceStrDescr;
	mDescrs[3] = (void*)newSnumStrDescr;
	
	cli();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	uint8_t i = 0;
	do {
		_delay_ms(1);
	} while(--i);
	usbDeviceConnect();
	sei();
}
