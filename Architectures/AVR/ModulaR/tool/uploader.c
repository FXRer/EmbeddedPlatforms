#ifdef WIN32
#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "../commsPacket.h"
#include "winTypes.h"
#include "hidapi.h"
#include "../speck.h"


//this assumes it runs on a little-endian CPU that supports unaligned accesses. Too bad, I am lazy
static bool mNeedPadding = true;	//better safe then sorry by default. this is the 1-byte padding for hardware that cnanot accept 8-byte multiple packets. not needed on windows as we round WAAY up on windows

#ifdef WIN32
#define USE_WIN_PADDING		1
#else
#define USE_WIN_PADDING		0
#endif

//return payload len of RXed packet or negative on error
static int8_t oneCmd(hid_device *handle, const struct CommsPacket *pktTx, uint8_t txPayloadLen, struct CommsPacket *pktRx)
{
	unsigned char bufTx[MODULAR_MAX_PACKET + 1 /* report id*/] = {0,}, rawRx[MODULAR_MAX_PACKET + 1 /* report id*/] = {0,};
	struct CommsPacket *pktTxReal = (struct CommsPacket*)(bufTx + 1);
	unsigned nRetries = 8, rxPayloadLen, rxLen;
	
	if (USE_WIN_PADDING) {
		struct CommsWinPacket *winPkt = (struct CommsWinPacket*)pktTxReal;
		
		winPkt->cmd = CMD_WIN_PADDED;
		winPkt->winHdr.actualPayloadLen = txPayloadLen;
		winPkt->actualPacket = *pktTx;
		memcpy(winPkt->actualPacket.payload, pktTx->payload, txPayloadLen);
		txPayloadLen = MODULAR_MAX_PACKET - sizeof(struct CommsPacket);		//always max sz
		winPkt->crc = commsPacketCalcCrc(pktTxReal, txPayloadLen);
	}
	else {
		*pktTxReal = *pktTx;
		memcpy(pktTxReal->payload, pktTx->payload, txPayloadLen);
		pktTxReal->crc = commsPacketCalcCrc(pktTxReal, txPayloadLen);
		
		if (mNeedPadding && !((txPayloadLen + sizeof(struct CommsPacket)) % 8)) {
			pktTxReal->cmd |= CMD_ORR_PADDED;
			txPayloadLen++;
		}
	}
	
	bufTx[0] = 1;		//report #1 always

	while(1) {
		
		if (!nRetries--)
			return -1;
		
	//	fprintf(stderr, "sending pkt %d\n", pktTxReal->cmd);
		
		if (hid_write(handle, bufTx, 1 /* report id*/ + txPayloadLen + sizeof(struct CommsPacket)) < 0)
			return -1;
		
		while ((rxLen = hid_read_timeout(handle, (char*)rawRx, MODULAR_MAX_PACKET + 1 /* report id*/, 500)) > 0) {
		
			if (rxLen < sizeof(struct CommsPacket) + 1)
				continue;
			
			//copy to real buffer - skip report number
			rxLen--;
			memcpy(pktRx, rawRx + 1, rxLen);
			
			//fprintf(stderr, "getting pkt %d in resp to pkt %d with total len %u\n", pktRx->cmd, pktTxReal->cmd, rxLen);
			
			rxPayloadLen = rxLen - sizeof(struct CommsPacket);
			
			if (commsPacketCalcCrc(pktRx, rxPayloadLen) != pktRx->crc)
				break;
			
			//if win padded, decode right away
			if (pktRx->cmd == CMD_WIN_PADDED) {
				
				struct CommsWinPacket *winPkt = (struct CommsWinPacket*)pktRx;
				
				
				//sanity check
				if (rxPayloadLen < winPkt->winHdr.actualPayloadLen + WINPACKET_OVERHEAD)
					continue;
				
				rxPayloadLen = winPkt->winHdr.actualPayloadLen;
				*pktRx = winPkt->actualPacket;
				memmove(pktRx->payload, winPkt->actualPacket.payload, rxPayloadLen);
			}
			
			if (pktRx->cmd != (pktTx->cmd &~ CMD_ORR_PADDED))
				continue;
			
	//		fprintf(stderr, "crc pass\n");
			return rxPayloadLen;
		}
	}
}


//At least linux will queue up bufers from device even while we're not expecting them. This will drain them.
static void drainOsBuf(hid_device *handle)
{
	struct CommsPacket pkt;
	
	while (hid_read_timeout(handle, (char*)&pkt, sizeof(struct CommsPacket), 50) > 0);
}

static bool showUsbInfo(hid_device *handle)
{
	wchar_t manuf[256], prod[256];
	
	if (hid_get_manufacturer_string(handle, manuf, 256) || hid_get_product_string(handle, prod, 256)) {
		fprintf(stderr, "NO USB INFO AVAIL\n");
		return false;
	}

	fprintf(stderr, "Dev USB info: '%ls' by '%ls'\n", prod, manuf);
	return true;
}

static bool getVerAndFlashInfo(hid_device *handle, int *flashBaseP, int *flashSizeP, int *flashBlockP, bool *encrP)
{
	uint8_t buf[MODULAR_MAX_PACKET];
	struct CommsPacket *pktP = (struct CommsPacket*)buf;
	struct CommsInfoPacket *nfo = (struct CommsInfoPacket*)&pktP->payload;
	char stringInfo[sizeof(nfo->info) + 1];
	
	pktP->cmd = COMMS_CMD_GET_INFO;
	

	if (oneCmd(handle, pktP, 0, pktP) < (int)sizeof(struct CommsInfoPacket)) {
		fprintf(stderr, "NO BL INFO AVAIL\n");
		return false;
	}
	
	
	if (nfo->magix != BL_INFO_MAGIX) {
		fprintf(stderr, "BL does not speak our language\n");
		return false;
	}
	
	memcpy(stringInfo, nfo->info, sizeof(nfo->info));
	stringInfo[sizeof(nfo->info)] = 0;
	*encrP = !!(nfo->flags & BL_FLAGS_NEED_ENCR);
	mNeedPadding = !!(nfo->flags & BL_FLAGS_NEED_PADDING);
	
	
	fprintf(stderr, "BL '%s' v%u (proto v%u%s). Flash: 0x%04X - 0x%04X with (%u x %u-byte blocks)%s\n",
		stringInfo, nfo->blVer, nfo->protoVer, mNeedPadding ? " <PAD>":"", nfo->base, nfo->size + nfo->base, nfo->size / nfo->blockSz, nfo->blockSz,
		*encrP ? " <ENCRYPTION REQUIRED>" : "");
	
	if (nfo->blVer < BL_PROT_VER_CUR) {
		fprintf(stderr, "BL proto too old\n");
		return false;
	}
	
	if ((nfo->size % nfo->blockSz) || (nfo->blockSz & (nfo->blockSz - 1)) || (nfo->blockSz > WRITE_SIZE)) {
		fprintf(stderr, "BL provided flash info that is implausible\n");
		return false;
	}
	
	if (flashBaseP)
		*flashBaseP = nfo->base;
	if (flashSizeP)
		*flashSizeP = nfo->size;
	if (flashBlockP)
		*flashBlockP = nfo->blockSz;

	return true;
}

int main(int argc, char* argv[])
{
	uint8_t buf[MODULAR_MAX_PACKET];
	struct CommsPacket *pkt = (struct CommsPacket*)buf;
	struct CommsWritePacket *wpkt = (struct CommsWritePacket*)pkt->payload;
	int ret = 0, fileSz, i, nChunks, addr, flashBase, flashSize, flashBlock;
	bool needEncr = false;
	hid_device *handle;
	FILE *file;

	ret = hid_init();
	if (ret) {
		fprintf(stderr, "Failed to load hid lib\n");
		goto out_hidlib;
	}

	//Find the device using the VID & PID,
	handle = hid_open(0x4744, 0x5043, NULL);
	if (!handle) {
		fprintf(stderr, "Failed to find device\n");
		ret = ENODEV;
		goto out_hidopen;
	}

	drainOsBuf(handle);

	if (!showUsbInfo(handle))
		goto out_earlyerr;
	
	if (!getVerAndFlashInfo(handle, &flashBase, &flashSize, &flashBlock, &needEncr))
		goto out_earlyerr;
	
	addr = flashBase;
	
	if (argc != 2 || !(file = fopen(argv[1], "r"))) {
		fprintf (stderr, "USAGE: %s <UPDATE FILE>\n", argv[0]);
		goto out_nofile;
	}
	
	if (fseek(file, 0, SEEK_END) < 0) {
		fprintf (stderr, "FIO error 1. Bad.\n");
		goto out_fio;
	}
	
	fileSz = ftell(file);
	rewind(file);
	
	if (flashBlock != WRITE_SIZE) {
		fprintf (stderr, "flash block size not as expected\n");
		goto out_filesz;
	}
	
	if (needEncr) {
		
		if (fileSz % (flashBlock + sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]))) {
			fprintf (stderr, "update not properly sized for encrypted data\n");
			goto out_filesz;
		}
		
		nChunks = fileSz / (flashBlock + sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]));
		if (nChunks < 2) {
			fprintf (stderr, "Update size (%u bytes) too small to be meaningful\n", fileSz);
			goto out_filesz;
		}
		nChunks--;	//do not count the "sig"
	}
	else {
		nChunks = (fileSz + flashBlock - 1) / flashBlock;
	}
	
	if (nChunks * flashBlock > flashSize) {
		fprintf (stderr, "Update size (%u bytes) larger than flash size (%u bytes)\n", fileSz, flashSize);
		goto out_filesz;
	}
	
	fprintf(stderr, "%u byte update found (%u blocks)\n", nChunks * flashBlock, nChunks);
	
	//do erase
	fprintf(stderr, "Erasing.");
	do {
		pkt->cmd = COMMS_CMD_SECURE_ERASE;
		fprintf(stderr, ".");
		ret = oneCmd(handle, pkt, 0, pkt);
		if (ret < 1) {
			fprintf (stderr, "USB error. Bad.\n");
			ret = -1;
			goto out_fio;
		}
		
	} while(!pkt->payload[0]);
	fprintf(stderr, "\n");
	
	//do write
	fprintf(stderr, "Writing...\n");
	for (i = 0; i < nChunks; i++, addr += flashBlock) {
		
		memset(buf, 0, sizeof(buf));
		pkt->cmd = COMMS_CMD_WRITE_BLOCK;
		
		if (needEncr) {
			if (sizeof(wpkt->iv) != fread(&wpkt->iv, 1, sizeof(wpkt->iv), file)) {
				fprintf (stderr, "FIO error 3. Bad.\n");
				goto out_fio;
			}
		}
		
		ret = (int)fread(wpkt->data, 1, flashBlock, file);
		if ((needEncr || i != nChunks - 1) && ret != flashBlock) {
			fprintf (stderr, "FIO error 2. Bad.\n");
			goto out_fio;
		}
		
		fprintf(stderr, "%2u.%02u%% [%3u/%3u @ 0x%04X]     \r", i * 100 / nChunks, (i * 100 % nChunks) * 100 % nChunks, i, nChunks, addr);
		
		ret = oneCmd(handle, pkt, sizeof(struct CommsWritePacket), pkt);
		if (ret < 1) {
			fprintf (stderr, "USB error. Bad.\n");
			ret = -1;
			goto out_fio;
		}

		if (!pkt->payload[0]) {
			fprintf (stderr, "Write error. Bad.\n");
			ret = -1;
			goto out_fio;
		}
	}
	
	//send "DONE" packet
	fprintf(stderr, "                                                \rCompleting write...\n");
	if (needEncr) {
		
		if (sizeof(wpkt->iv) != fread(&wpkt->iv, 1, sizeof(wpkt->iv), file)) {
			fprintf (stderr, "FIO error 4. Bad.\n");
			goto out_fio;
		}
		
		ret = (int)fread(wpkt->data, 1, flashBlock, file);
		if (ret != flashBlock) {
			fprintf (stderr, "FIO error 5. Bad.\n");
			goto out_fio;
		}
	}
	
	wpkt->flags = FLAG_UPLOAD_DONE;
	ret = oneCmd(handle, pkt, sizeof(struct CommsWritePacket), pkt);
	if (ret < 1) {
		fprintf (stderr, "USB error 2 . Bad.\n");
		ret = -1;
		goto out_fio;
	}

	if (!pkt->payload[0]) {
		fprintf (stderr, "Write complete error. Bad.\n");
		ret = -1;
		goto out_fio;
	}

	fprintf(stderr, "FLASHING SUCCESS - booting app...\n");
	
	pkt->cmd = COMMS_CMD_BOOT_APP;
	ret = oneCmd(handle, pkt, 0, pkt);
	if (ret < 1) {
		fprintf (stderr, "USB error 3 . Bad.\n");
		ret = -1;
		goto out_fio;
	}
	
	if (!pkt->payload[0]) {
		fprintf (stderr, "Boot error. Bad.\n");
		ret = -1;
		goto out_fio;
	}

	fprintf(stderr, "SUCCESS...\n");
	
	ret = 0;

out_filesz:
out_fio:
	fclose(file);
	
out_nofile:
	;

out_earlyerr:
	;

out_hidopen:
	(void)hid_exit();

out_hidlib:
	return ret;
}
