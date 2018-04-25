/**
 * $Id$
 *
 * @file
 * \todo add a description here
 *
 */
/*-----------------------------------------------------------------------------
 *           PROPRIETARY INFORMATION OF ATMEL CORPORATION
 *
 *                    Copyright Atmel Corporation
 *
 *  The scope of use of the material contained herein is limited
 *  strictly to the terms of the LICENSE which grants permission
 *  for the use of the material. Distribution of this material
 *  and all copies thereof to any entity other than to the
 *  EMPLOYEES of the Licensee, as the term is defined in the
 *  LICENSE, is strictly forbidden without a prior written consent
 *  of Atmel Corporation.
 *---------------------------------------------------------------------------*/

#include <avr/io.h> /* as long as gpio_ not implemented */
#include <util/delay.h>

#include "spi.h"

#include "isp.h"

#define PORT_RST PORTE
#define DDR_RST DDRE
#define RST (PE2)

#define ISP_INSTR_LENGTH (4)

/* ISP instruction set, taken from datasheet ATtiny45, Table 20-12 */

/* Read Instructions */
#define ISP_INSTR_READ_PROGMEM_HIGH (0x28)
#define ISP_INSTR_READ_PROGMEM_LOW (0x20)
#define ISP_INSTR_READ_EEPROM (0xA0)
#define ISP_INSTR_READ_LOCKBITS (0x58)
#define ISP_INSTR_READ_SIGNATURE (0x30)
#define ISP_INSTR_READ_CALIBRATION (0x38)

/*
 * \brief Transfer of 4 Bytes (fixed)
 *
 */
static inline void isp_transfer(uint8_t *wbuf, uint8_t *rbuf)
{
	spi_transfer(wbuf, rbuf, ISP_INSTR_LENGTH);
}

/*
 * \brief Enter ISP mode
 *
 * @return >0 if successful, 0 else
 */
uint8_t isp_start(void)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0xAC, 0x53, 0x00, 0x00};

	spi_init();
	PORT_RST &= ~(1<<RST);
	DDR_RST |= (1<<RST);

	_delay_ms(20); /* wait at least 20ms, acc. to programming algorithm */
	isp_transfer(buf, buf);

	/* echo of second byte of write transfer (third byte of read transfer) is expected */
	return(0x53 == buf[2]);
}

/*
 * \brief Exit ISP mode
 *
 */
void isp_stop(void)
{
	spi_exit();
	PORT_RST |= (1<<RST);
}

/*
 * \brief Poll for program operation ready/nbusy
 *
 * @return 1 if busy (programming active), 0 if ready
 */
uint8_t isp_poll_busy(void)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0xF0, 0x00, 0x00, 0x00};
	isp_transfer(buf, buf);
	return (0xFE != buf[ISP_INSTR_LENGTH-1]);
}

void isp_chiperase(void)
{
	static uint8_t wbuf[ISP_INSTR_LENGTH] = {0xAC, 0x80, 0x00, 0x00};
	isp_transfer(wbuf, wbuf);
}

/*
 * \brief Combined instruction for load program memory
 * High byte and low byte
 *
 * Address is a byte address
 *
 * API identical to avr-libc __boot_page_fill_normal(address, data)
 *
 */
void isp_load_page_progmem(uint16_t addr, uint16_t data)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0x48, 0x00, 0x00, 0x00};

	buf[0] = 0x40; /* load low byte first acc. to datasheet */
	buf[2] = addr; /* page offset */
	buf[3] = (uint8_t)(data >> 0);
	isp_transfer(buf, buf);

	buf[0] = 0x48; /* load high byte */
	buf[2] = addr; /* page offset */
	buf[3] = (uint8_t)(data >> 8);
	isp_transfer(buf, buf);

}

void isp_pageload_eemem(uint8_t addr, uint16_t data)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0xC1, 0x00, addr & 0x03, data};

	isp_transfer(buf, buf);
}

void isp_pagewrite_eemem(uint16_t addr)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0xC1, (addr>>8) & 0x03, addr & 0xFC, 0x00};

	isp_transfer(buf, buf);
}

static void isp_write_lockfuse(uint8_t mem, uint8_t val)
{
	uint8_t wbuf[ISP_INSTR_LENGTH] = {0xAC, mem, 0x00, val};
	isp_transfer(wbuf, wbuf);
}

void isp_write_lockbits(uint8_t val)
{
	isp_write_lockfuse(0xE0, val);
}
void isp_write_lfuse(uint8_t val)
{
	isp_write_lockfuse(0xA0, val);
}
void isp_write_hfuse(uint8_t val)
{
	isp_write_lockfuse(0xA8, val);
}
void isp_write_efuse(uint8_t val)
{
	isp_write_lockfuse(0xA4, val);
}

void isp_write_eeprom_byte(uint16_t addr, uint8_t val)
{
	uint8_t wbuf[ISP_INSTR_LENGTH] = {0xAC, (addr << 8), (addr << 0), val};
	isp_transfer(wbuf, wbuf);
}

static inline uint8_t isp_read_block(const uint8_t type, uint16_t addr, uint8_t *buf, uint8_t nbbytes)
{
	uint8_t wbuf[ISP_INSTR_LENGTH] = {type, 0x00, 0x00, 0x00};
	uint8_t rbuf[ISP_INSTR_LENGTH];
	uint8_t ret = 0;

	if(nbbytes > 0){
		do{
			wbuf[1] = (uint8_t)(addr << 8);
			wbuf[2] = (uint8_t)(addr << 0);
			addr++;
			isp_transfer(wbuf, rbuf);
			*buf++ = rbuf[ISP_INSTR_LENGTH-1];
		}while(--nbbytes);
	}

	return ret;
}

uint16_t isp_read_flash_word(uint16_t addr)
{
	uint8_t buf[ISP_INSTR_LENGTH];
	uint16_t ret;

	buf[0] = 0x20; /* low byte */
	buf[1] = (uint8_t)(addr << 8);
	buf[2] = (uint8_t)(addr << 0);
	buf[3] = 0x00;
	isp_transfer(buf, buf);
	ret = buf[3];

	buf[0] = 0x28; /* high byte */
	buf[1] = (uint8_t)(addr << 8);
	buf[2] = (uint8_t)(addr << 0);
	buf[3] = 0x00;
	isp_transfer(buf, buf);
	ret |= (uint16_t)(buf[3] << 8);

	return ret;
}

uint8_t isp_read_eeprom_byte(uint16_t addr)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {ISP_INSTR_READ_EEPROM, (uint8_t)(addr>>8), (uint8_t)(addr>>0), 0x00};

	isp_transfer(buf, buf);

	return buf[ISP_INSTR_LENGTH-1]; /* last byte */
}

int16_t isp_read_lockbits(void)
{
	return 0;
}

uint8_t isp_read_signature(uint8_t *buf, uint8_t nbbytes)
{
	return isp_read_block(ISP_INSTR_READ_SIGNATURE, 0x00, buf, nbbytes);
}

/*
 * \brief Read Fuses
 * fixed 3 Bytes: LO, HI, EXT
 *
 * @param fuse 'E', 'H', 'L' for Extended, High, Low
 * @return The fuse value
 */
uint8_t isp_read_fuse(uint8_t fuse)
{
	static uint8_t instr_lfuse[ISP_INSTR_LENGTH] = {0x50, 0x00, 0x00, 0x00};
	static uint8_t instr_hfuse[ISP_INSTR_LENGTH] = {0x58, 0x08, 0x00, 0x00};
	static uint8_t instr_efuse[ISP_INSTR_LENGTH] = {0x50, 0x08, 0x00, 0x00};
	static uint8_t rbuf[ISP_INSTR_LENGTH];
	uint8_t *wbuf = 0x0000;
	uint8_t ret;

	switch(fuse){
	case 'E':
		wbuf = instr_efuse;
		break;
	case 'H':
		wbuf = instr_hfuse;
		break;
	case 'L':
		wbuf = instr_lfuse;
		break;
	default:
		break;
	}

	if(wbuf != 0x0000){
		isp_transfer(wbuf, rbuf);
		ret = rbuf[ISP_INSTR_LENGTH-1]; /* last byte */;
	} else {
		ret = 0x00;
	}
	return ret;
}

/*
 * \brief Read Fuses
 * fixed 3 Bytes: LO, HI, EXT
 *
 * @return The number of bytes read
 */
int16_t isp_read_fuses(uint8_t *buf)
{
	static uint8_t instr_lfuse[ISP_INSTR_LENGTH] = {0x50, 0x00, 0x00, 0x00};
	static uint8_t instr_hfuse[ISP_INSTR_LENGTH] = {0x58, 0x08, 0x00, 0x00};
	static uint8_t instr_efuse[ISP_INSTR_LENGTH] = {0x50, 0x08, 0x00, 0x00};
	static uint8_t rbuf[ISP_INSTR_LENGTH];

	isp_transfer(instr_lfuse, rbuf);
	*buf++ = rbuf[ISP_INSTR_LENGTH-1]; /* last byte */
	isp_transfer(instr_hfuse, rbuf);
	*buf++ = rbuf[ISP_INSTR_LENGTH-1]; /* last byte */
	isp_transfer(instr_efuse, rbuf);
	*buf++ = rbuf[ISP_INSTR_LENGTH-1]; /* last byte */

	return 3;
}



int16_t isp_read_calibration(void)
{
	return 0;
}

void isp_write_page_progmem(const uint16_t addr)
{
	uint8_t buf[ISP_INSTR_LENGTH] = {0x4C, (uint8_t)(addr>>8), (uint8_t)(addr>>0), 0x00};

	isp_transfer(buf, buf);
}

/* EOF */
