/*
 * spi.c
 *
 *  Created on: 23.06.2012
 *      Author: dthiele
 */

#include <avr/io.h>

#include "spi.h"

#define DDR_SPI  (DDRB)   /**< DDR register for SPI port */
#define PORT_SPI (PORTB)  /**< PORT register for SPI port */

#define SPI_MOSI (1<<PB2)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<PB3)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<PB1)  /**< PIN mask for SCK pin */
#define SPI_SS   (1<<PB0)  /**< PIN mask for SS pin */

#define SPI_DATA_REG SPDR  /**< abstraction for SPI data register */

void spi_init(void)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |= SPI_SCK | SPI_SS;

    SPCR = ((1<<SPE) | (1<<MSTR));

    SPCR |= ((1<<SPR1) | (1<<SPR0) );
    SPSR &= ~(1<<SPI2X);
}

void spi_exit(void)
{
    SPCR = 0;

    DDR_SPI  &= ~(SPI_MOSI | SPI_SCK | SPI_SS);
    PORT_SPI  &= ~(SPI_MOSI | SPI_SCK | SPI_SS);
}

uint8_t spi_transfer(uint8_t *txbuf, uint8_t *rxbuf, uint8_t nbbytes)
{
	do{
		SPDR = *txbuf++;
		while(0 == (SPSR & (1<<SPIF)));
		*rxbuf++ = SPDR;
	}while(--nbbytes);
	return nbbytes;
}

/* EOF */
