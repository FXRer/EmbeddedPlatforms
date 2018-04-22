#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../commsPacket.h"
#include "../speck.h"


//assumes little endian machine that can access unaligned data. too bad, i am lazy!

static void rol1xor(SPECK_DATA_TYPE *ck, SPECK_DATA_TYPE *data)
{
	uint8_t *buf = (uint8_t*)ck;
	uint8_t i, c = buf[sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]) - 1] >> 7;
	
	for (i = 0; i < sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]); i++) {
		uint8_t b = buf[i], newC = b >> 7;
		
		buf[i] = (b << 1) | c;
		c = newC;
	}
	
	for (i = 0; i < SPECK_DATA_WORDS; i++)
		ck[i] ^= data[i];
}


static void ror8xor(SPECK_DATA_TYPE *ck, SPECK_DATA_TYPE *data)
{
	uint8_t *buf = (uint8_t*)ck;
	uint8_t i, t = buf[0];
	
	//ROR8
	for (i = 0; i < (uint8_t)sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]) - 1; i++)
		buf[i] = buf[i + 1];
	buf[i] = t;
	
	//xor
	for (i = 0; i < SPECK_DATA_WORDS; i++)
		ck[i] ^= data[i];
}

static int encrAndWriteBlock(SPECK_DATA_TYPE *data, uint32_t len, FILE *urandom, const SPECK_DATA_TYPE *keyE, SPECK_DATA_TYPE *checksumP)
{
	SPECK_DATA_TYPE iv[SPECK_DATA_WORDS], i;
	uint8_t *buf = (uint8_t*)data;
	
	//pick IV
	if (sizeof(iv) != fread(&iv, 1, sizeof(iv), urandom)) {
		fprintf(stderr, "random read fail\n");
		return -1;
	}
	
	//write IV
	for (i = 0; i < sizeof(iv); i++)
		putchar(((uint8_t*)iv)[i]);
	
	//encrypt using SPECK in PCBC mode
	for (i = 0; i < len; i += sizeof(SPECK_DATA_TYPE[SPECK_DATA_WORDS]), data += SPECK_DATA_WORDS) {
		
		if (checksumP)
			rol1xor(checksumP, data);
		
		speckEncrCbc(data, iv, keyE);
	}
	
	//output data
	for (i = 0; i < len; i++)
		putchar(*buf++);
	
	return 0;
}

int main(int argc, char** argv)
{
	uint8_t buf[WRITE_SIZE];
	SPECK_DATA_TYPE keyS[SPECK_KEY_WORDS], keyE[SPECK_NUM_ROUNDS];
	SPECK_DATA_TYPE checksum[SPECK_DATA_WORDS] = {0,};
	FILE *f;
	int L, i, ret = -1, nBlocks = 0;
	
	if (argc != 2 || strlen(argv[1]) != 2 * sizeof(SPECK_DATA_TYPE[SPECK_KEY_WORDS])) {
		fprintf(stderr, "USAGE: %s <KEY in hex(%u bytes)> < infile > outfile to encrypt using said key\n", argv[0], (int)sizeof(SPECK_DATA_TYPE[SPECK_KEY_WORDS]));
		return -1;
	}
	
	f = fopen("/dev/urandom", "r");
	if (!f) {
		fprintf(stderr, "random open fail\n");
		return -1;
	}
	
	for (L = 0; L < sizeof(SPECK_DATA_TYPE[SPECK_KEY_WORDS]); L++) {
		
		uint8_t *dst = (uint8_t*)keyS;
		int i;
		
		sscanf(argv[1] + L * 2, "%2x", &i);
		dst[L] = i;
	}
	
	fprintf(stderr, "USING key {");
	for (i = 0; i < SPECK_KEY_WORDS; i++)
		fprintf(stderr, "0x%X, ", keyS[i]);
	fprintf(stderr, "}\n");
	
	speckKeySched(keyE, keyS);
	fprintf(stderr, "EXPANDED KEY SCHEDULE (for decryption):");
	for (i = 0; i < SPECK_NUM_ROUNDS; i++)
		fprintf(stderr, " 0x%x,", keyE[i]);
	fprintf(stderr, "\n");
	
	while(1) {
		
		L = 0;
		while (L != WRITE_SIZE && (i = getchar()) != EOF)
			buf[L++] = i;
		
		if (!L)
			break;
		
		while (L != WRITE_SIZE)
			buf[L++] = 0;
		
		ret = encrAndWriteBlock((SPECK_DATA_TYPE*)buf, WRITE_SIZE, f, keyE, checksum);
		if (ret)
			goto out;
		
		nBlocks++;
	}
	fprintf(stderr, "%u block(s) written\n", nBlocks);
	memset(buf, 0, WRITE_SIZE);
	memcpy(buf, checksum, sizeof(checksum));
	ret = encrAndWriteBlock((SPECK_DATA_TYPE*)buf, WRITE_SIZE, f, keyE, NULL);
	if (ret)
		goto out;
	
	ret = 0;
	
out:
	fclose(f);
	return 0;
}