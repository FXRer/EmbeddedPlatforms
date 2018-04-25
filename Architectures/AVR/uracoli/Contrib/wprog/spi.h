/*
 * spi.h
 *
 *  Created on: 23.06.2012
 *      Author: dthiele
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void spi_init(void);
void spi_exit(void);
uint8_t spi_transfer(uint8_t *txbuf, uint8_t *rxbuf, uint8_t nbbytes);

#endif /* SPI_H_ */
