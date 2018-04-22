#ifndef _FLASH_H_
#define _FLASH_H_

#define FLASH_BLOCK_SZ	64
#define APP_START		0xcc0
#define APP_LEN			(0x2000 - APP_START)

void flashErase(uint16_t dstAddr);
void flashWrite(uint16_t dstAddr, const void *srcData);



#endif
 