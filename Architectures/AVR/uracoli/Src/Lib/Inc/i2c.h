/* Copyright (c) 2011 Daniel Thiele
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#ifndef I2C_H_
#define I2C_H_
/**
  @defgroup grpSensorBusI2C I2C Bus Driver
  @ingroup grpSensorDefs
  @{

*/

/* === includes ============================================================ */

/* include here to publish TWI_STATUS codes to higher modules */
#include <util/twi.h>

/* === macros ============================================================== */

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize TWI module
 *
 * SCL_freq = F_CPU / (16 + 2*TWBR*TWPS)
 *
 * @param freq_Hz Frequency in Hz
 */
void i2c_init(uint32_t freq_Hz);

/**
 * @brief Probe I2C device address by trying to read an arbitrary register
 *
 * @param devaddr  Device address 7-bit left-aligned
 * @return 0:  error, 1: success
 *
 */
uint8_t i2c_probe(uint8_t devaddr);

/**
 * @brief Transfer a number of bytes, 1st write, 2nd read
 *
 * @param devaddr      Device address 7-bit left-aligned
 * @param *writebuf    Pointer to buffer that is written to bus
 * @param bytestowrite Number of bytes to write
 * @param *readbuf     Pointer to buffer to read into
 * @param bytestoread  Number of byte to read
 * @return 0:  error, 1: success
 *
 */
uint8_t i2c_master_writeread(uint8_t devaddr, uint8_t *writebuf, uint8_t bytestowrite, uint8_t *readbuf, uint8_t bytestoread);

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef I2C_H_ */
