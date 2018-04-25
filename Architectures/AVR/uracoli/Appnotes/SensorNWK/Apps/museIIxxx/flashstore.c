/*
 * flashstore.c
 *
 *  Created on: 03.04.2012
 *      Author: dthiele
 */

/* avr-libc inclusions */
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

/* DEBUG only */
#include <board.h>
#include <ioutil.h>
#include <util/delay.h>

typedef struct
{
    uint8_t pagebuf[SPM_PAGESIZE];	/* buffer for writing flash pages */
    uint32_t flashidx; /* current idx of flash address, used for read and write */
    uint16_t pagebufidx;	/* current idx for filling pagebuf */
} flash_storage_t;

static const uint32_t flashstore_baseaddr = 0x1800; /* 0x0C00 word-addres as byte address */
static flash_storage_t flashstore;

void __attribute__ ((section (".bootloader"))) boot_program_page (uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;

    sreg = SREG;
    cli();

    eeprom_busy_wait ();

    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.

    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.

        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill (page + i, w);
    }

    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.

    boot_rww_enable ();

    SREG = sreg;
}

/*
 * \brief
 *
 * @param size Size to use for buffer
 */
void flashstore_enter(uint16_t size)
{
    flashstore.flashidx = flashstore_baseaddr;
    flashstore.pagebufidx = 0;
}

/*
 * \brief Store measurement set to flash
 */
void flashstore_feed(uint8_t *chunk, uint8_t len)
{
    memcpy(&flashstore.pagebuf[flashstore.pagebufidx], chunk, len);

    flashstore.pagebufidx += len;
    if(flashstore.pagebufidx >= SPM_PAGESIZE)
    {
        flashstore.pagebufidx = 0;
        boot_program_page(flashstore.flashidx, flashstore.pagebuf);
        flashstore.flashidx += SPM_PAGESIZE;
    } /* if(flashstore.pagebufidx >= SPM_PAGESIZE) */
}

/*
 * \brief Flush the data collected till here
 */
void flashstore_leave(void)
{

}

/* EOF */
