#ifndef _COMMS_PACKET_H_
#define _COMMS_PACKET_H_

#ifndef WIN32
#include <stdint.h>
#endif
#include "speck.h"

#define MODULAR_MAX_PACKET		141		//for uploader only. app may use larger but never smaller

#define CMD_ORR_PADDED			0x80	//one extra byte of padding added to avoid ZLPs
#define CMD_WIN_PADDED			0x78	//encapsulates another packet internally

#pragma pack(push,1)
struct CommsPacket {
	uint8_t cmd;
	uint8_t crc;
	uint8_t payload[];
};
#pragma pack(pop)


#pragma pack(push,1)
struct CommsWinPacketHeader {
	uint16_t actualPayloadLen;			//size of the PAYLOAD of the packet that is encapsulated
};
#pragma pack(pop)

#pragma pack(push,1)
struct CommsWinPacket {
	uint8_t cmd;						//necessarily same as "struct CommsPacket"
	uint8_t crc;
	struct CommsWinPacketHeader winHdr;	//length of actual encapsulated packet
	struct CommsPacket actualPacket;	//crc in here is ignored
};
#pragma pack(pop)

#define WINPACKET_OVERHEAD				(sizeof(struct CommsPacket) + sizeof(struct CommsWinPacketHeader))			//cost of a winpacket padding
#define MODULAR_MAX_USEFUL_BYTES		(MODULAR_MAX_PACKET - WINPACKET_OVERHEAD)

#ifdef _LARGE_PACKETS_
#define CRC_LEN_TYPE	uint32_t
#else
#define CRC_LEN_TYPE	uint8_t
#endif


static inline uint8_t commsPacketCalcCrc(const struct CommsPacket *p, CRC_LEN_TYPE payloadLen)
{
	uint8_t crc = ~p->cmd;
	CRC_LEN_TYPE i;
	
	for (i = 0; i < payloadLen; i++)
		crc = (crc << 1) ^ ((crc & 0x80) ? 0xB1 : 0x00) ^ p->payload[i];
	
	return crc;
}

//bootloader CMDs grow down from top, apps can use bottom. all data is little endian
#define COMMS_CMD_GET_INFO		0x7F	//	{} -> struct CommsInfoPacket
#define COMMS_CMD_BOOT_APP		0x7E	//	() -> (bool success), reboot to app. no reply will be sent if app boot works, unless app sends it
#define COMMS_CMD_WRITE_BLOCK	0x7D	//	{ uint24_t addr, uint8_t data[64] } -> {bool success}
#define COMMS_CMD_SECURE_ERASE	0x7C	//	() -> (bool erase_done)



#define WRITE_SIZE			64
#define FLAG_UPLOAD_DONE	0x01	//contains "signature" in first 8 bytes

#pragma pack(push,1)
struct CommsWritePacket {
	SPECK_DATA_TYPE iv[SPECK_DATA_WORDS];
	uint8_t flags; 	//RFU
	uint8_t data[WRITE_SIZE];
};
#pragma pack(pop)


#define BL_INFO_MAGIX		0x4744
#define BL_PROT_VER_0		0
#define BL_PROT_VER_1		1
#define BL_PROT_VER_CUR		BL_PROT_VER_1

#define BL_FLAGS_NEED_ENCR	0x01
#define BL_FLAGS_NEED_PADDING	0x02

#pragma pack(push,1)
struct CommsInfoPacket {
	uint16_t magix; 
	uint16_t base;
	uint16_t size;
	uint16_t blockSz;
	uint8_t blVer;
	uint8_t protoVer;
	uint8_t flags;
	char info[11];
};
#pragma pack(pop)






#endif



