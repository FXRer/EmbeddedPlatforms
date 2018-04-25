/**
 * $Id: isp.h 2618 2012-03-28 07:56:34Z dthiele $
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

#ifndef ISP_H_
#define ISP_H_

#include <stdint.h>

uint8_t isp_start(void);
void isp_stop(void);
void isp_chiperase(void);

void isp_load_page_progmem(uint16_t addr, uint16_t data);

void isp_write_lockbits(uint8_t val);
void isp_write_lfuse(uint8_t val);
void isp_write_hfuse(uint8_t val);
void isp_write_efuse(uint8_t val);
void isp_write_eeprom(uint16_t addr, uint8_t val);
void isp_write_page_progmem(uint16_t addr);

uint16_t isp_read_flash_word(uint16_t addr);
uint8_t isp_read_eeprom_byte(uint16_t addr);
int16_t isp_read_lockbits(void);
uint8_t isp_read_signature(uint8_t *buf, uint8_t nbbytes);
uint8_t isp_read_fuse(uint8_t fuse);
int16_t isp_read_fuses(uint8_t *buf);
int16_t isp_read_calibration(void);

uint8_t isp_poll_busy(void);


#endif /* ISP_H_ */
