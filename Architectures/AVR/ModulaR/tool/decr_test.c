#include <stdint.h>
#include <stdio.h>
#include "../speck.h"
#include "../commsPacket.h"


#include "../key.c"

int main(int argc, char **Argv)
{
	struct {
		SPECK_DATA_TYPE iv[SPECK_DATA_WORDS];
		union {
			uint8_t buf[WRITE_SIZE];
			SPECK_DATA_TYPE b[0];
		};
	} RB;
	
	
	while (1)
	{
		SPECK_DATA_TYPE in[SPECK_DATA_WORDS];
		char *p = (char*)&RB;
		int c, i, j, L = 0;
		SPECK_DATA_TYPE *iv = RB.iv, *ptr;
		
		while (L != sizeof(RB) && (c = getchar()) != EOF) {
			*p++ = c;
			L++;
		}
		
		if (!L)
			break;
		
		if (L != sizeof(RB)) {
			fprintf(stderr, "WTF\n");
			break;
		}
		
		for (ptr = RB.b, i = 0; i < WRITE_SIZE; i += sizeof(in), ptr += SPECK_DATA_WORDS) {
				
				speckDecrCbc(ptr, iv, key);
		}
	
		for (i = 0; i < WRITE_SIZE ; i++) {
		
				putchar(RB.buf[i]);
		}
	}
}