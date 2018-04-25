#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "isp.h"

# define ADDR_T unsigned long

static ADDR_T address;
static uint8_t buf[128];

static uint8_t recchar(void)
{
	int c;
	while (EOF == (c = getchar()))
		;
	return c;
}

static uint16_t recword(void)
{
	int c;
	uint16_t ret;

	while (EOF == (c = getchar()));
	ret = (c<<8);
	while (EOF == (c = getchar()));
	ret |= (uint8_t)(c<<0);

	return ret;
}

void avr109()
{
	uint8_t c;
	static volatile uint16_t size;
	uint16_t temp;
	uint16_t pageword;

	c = recchar();

	switch (c)
	{
	/* Enter Programming Mode */
	case 'P': /* 13d */
		isp_start();
		putchar('\r');
		break;

		/* Leave Programming Mode */
	case 'L': /* 13d */
		isp_stop();
		putchar('\r');
		break;

		/* Auto Increment Address */
	case 'a': /* dd */
		putchar('Y'); /* Yes */
		break;

		/* Set Address */
	case 'A': /* ah al 13d */
		address = recword();
		putchar('\r');
		break;

		/* Write Program Memory, Low Byte */
	case 'c': /* dd 13d */
		pageword = recchar();
		putchar('\r');
		break;

		/* Write Program Memory, High Byte */
	case 'C': /* dd 13d */
		pageword |= (recchar()<<8);
		isp_load_page_progmem(address<<1, pageword);
		putchar('\r');
		break;

		/* Issue Page Write */
	case 'm': /* 13d */
		isp_write_page_progmem(address<<1);
		while (isp_poll_busy())
		{
			/* wait */
		}
		putchar('\r');
		break;

		/* Read Lock Bits */
	case 'r': /* dd */
		putchar(0xAA);
		break;

		/* Read Program Memory */
	case 'R': /* 2*dd */
		/* to be implemented */
		break;

		/* Read Data Memory */
	case 'd': /* dd */
		address++;
		break;

		/* Write Data Memory */
	case 'D': /* dd 13d */
		address++;
		putchar('\r');
		break;

		/* Chip Erase */
	case 'e':
		isp_chiperase();
		while (isp_poll_busy())
		{
			/* wait */
		}
		putchar('\r');
		break;

		/* Write Lock Bits */
	case 'l': /* dd 13d */
		c = recchar();
		putchar('\r');
		break;

		/* Read Fuse Bits */
	case 'F': /* dd  */
		putchar(isp_read_fuse('L'));
		break;

		/* Read High Fuse Bits */
	case 'N': /* dd */
		putchar(isp_read_fuse('H'));
		break;

		/* Read Extended Fuse Bits */
	case 'Q': /* dd */
		putchar(isp_read_fuse('E'));
		break;

		/* Select Device Type */
	case 'T': /* dd 13d */
		c = recchar();
		putchar('\r');
		break;

		/* Read Signature Bytes */
	case 's': /* 3*dd */
		isp_read_signature(buf, 3);
		putchar(buf[2]);
		putchar(buf[1]);
		putchar(buf[0]);
		break;

		/* Return Supported Device Codes */
	case 't': /* n*dd 00d */
		putchar(0x77); /* ATmega8 */
		putchar(0x00);
		break;

		/* Return Software Identifier */
	case 'S': /* s[7] */
		printf("AVRBOOT");
		break;

		/* Return Software Version */
	case 'V': /* dd dd */
		putchar('1');
		putchar('0');
		break;

		/* Return Hardware Version */
	case 'v': /* dd dd */
		putchar('1');
		putchar('0');
		break;

		/* Return Programmer Type */
	case 'p': /* dd */
		putchar('S'); // Answer 'SERIAL'.
		break;

		/* Set LED */
	case 'x': /* dd 13d */
		c = recchar();
		putchar('\r');
		break;

		/* Clear LED */
	case 'y': /* dd 13d */
		break;

		/* Exit Bootloader */
	case 'E': /* 13d */
		putchar('\r');
		break;

		/* Check Block Support */
	case 'b': /* 'Y' 2*dd */
		putchar('Y');
		putchar(0);
		putchar(16); /* Replace by page size of DUT */
		break;

		/* Start Block Flash Load */
		/* Start Block EEPROM Load */
	case 'B':
		size = recword(); /* block size */
		c = recchar(); /* get memtype F (flash) or E (eeprom) */
		if ('F' == c)
		{
			ADDR_T tempaddress;

			address <<= 1; // Convert address to bytes temporarily.
			tempaddress = address;  // Store address in page.

			do
			{
				pageword = recchar();
				pageword |= (recchar() << 8);

				isp_load_page_progmem(address, pageword);

				address += 2; // Select next word in memory.
				size -= 2; // Reduce number of bytes to write by two.
			} while (size); // Loop until all bytes written.

			isp_write_page_progmem(tempaddress);
			while (isp_poll_busy())
			{
				/* wait */
			}

			address >>= 1; // Convert address back to Flash words again.

			putchar('\r');

		}
		else if ('E' == c)
		{
			putchar('\r');
		}
		else
		{
			putchar('?'); /* unknown mem type */
		}
		break;

		/* Start Block Flash Read */
		/* Start Block EEPROM Read */
	case 'g': /* 2*dd n*dd */
		size = recword();
		c = recchar(); /* F or E */
		if ('F' == c)
		{
			do{
				temp = isp_read_flash_word(address);
				putchar((uint8_t)(temp>>0));
				putchar((uint16_t)(temp>>8));
				address+=2;
				size -= 2;
			}while(size);
		}
		else if ('E' == c)
		{
			do
			{
				putchar(isp_read_eeprom_byte(address++));
			} while (--size);
		}
		else
		{
			/* ignore */
		}
		break;

		/* Synchronization */
	case 0x1B:
		break;

	default:
		putchar('?');
		break;
	} /* switch(c) */
}

/* EOF */
