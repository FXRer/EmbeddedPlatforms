/* Copyright (c) 2011 ... 2013 Daniel Thiele, Axel Wachtler
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

/* avr-libc inclusions */
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

/* project inclusions */
#include "board.h"
#include "i2c.h"
#include "hif.h"
#if defined(__AVR_MEGA__) /* AVR Mega + AVR Xmega devices */
# define HAS_I2C (1)
#else
# define HAS_I2C (0)
# warning There is currently no I2C implementation for this board.
#endif

#if defined(__AVR_XMEGA__)
TWI_t * twiinst = &TWI_INST; /**< TWI instance */

/*
 * \brief Wait for transfer completion of a single byte
 *
 */
static inline void twi_wait(void)
{
    while( (0 == (twiinst->MASTER.STATUS & TWI_MASTER_WIF_bm)) && (0 == (twiinst->MASTER.STATUS & TWI_MASTER_RIF_bm)) );
}
#endif

/**
 * @brief Initialize TWI module
 *
 * SCL_freq = F_CPU / (16 + 2*TWBR*TWPS)
 *
 * @param freq_Hz Frequency in Hz
 */
void i2c_init(uint32_t freq_Hz)
{
#if HAS_I2C

#if defined(__AVR_XMEGA__)
    twiinst->CTRL = 0;
    twiinst->MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
    twiinst->MASTER.CTRLB = 0;
    twiinst->MASTER.CTRLC = 0;

    /*
     * ftwi = fsys / (2*(5 + TWMBR)) [Hz]
     * TWMBR = fsys / (2*ftwi) - 5
     */
    twiinst->MASTER.BAUD = F_CPU / (2UL*freq_Hz) - 5;
    twiinst->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
#else /* defined(__AVR_XMEGA__) */
    uint8_t psbits = 0; /* Prescaler Bits */
    uint8_t ps; /* Prescaler Value */
    uint8_t br; /* Baudrate */
    if(freq_Hz < 100000UL)
    {
        psbits = 1;
    }
    else if(freq_Hz < 10000UL)
    {
        psbits = 2;
    }
    else
    {
        psbits = 0;
    }

    ps = 4^psbits;
    br = (F_CPU/freq_Hz-16)/(2*ps);

    TWSR &= ~((1<<TWPS1) | (1<<TWPS0)); /* clear */
    TWSR |= psbits;

    TWBR = br;

    #if defined(I2C_ENABLE)
        I2C_ENABLE();
    #endif
#endif /* defined(__AVR_XMEGA__) */

#endif
}

/**
 * @brief Send start condition onto bus
 *
 * @return I2C status code
 */
static inline uint8_t i2c_startcondition(void)
{
#if HAS_I2C
#if defined(__AVR_XMEGA__)
    /* empty, use i2c_master_writeread() */
#else /* defined(__AVR_XMEGA__) */
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    return TW_STATUS;
#endif /* defined(__AVR_XMEGA__) */
#else
    return 0;
#endif
}

/**
 * @brief Send start condition onto bus
 *
 * @return I2C status code
 */
static inline uint8_t i2c_stopcondition(void)
{
#if HAS_I2C
#if defined(__AVR_XMEGA__)
    /* empty, use i2c_master_writeread() */
#else /* defined(__AVR_XMEGA__) */
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    DELAY_US(20);	/* needed for next start condition, to be analyzed */
    return TW_STATUS;
#endif /* defined(__AVR_XMEGA__) */
#else
    return 0;
#endif
}

/**
 * @brief Write a single byte to I2C
 * This can be address byte + RW flag or data byte
 *
 *	@param b Byte to write
 *	@return Status code
 */
static inline uint8_t i2c_writebyte(uint8_t b)
{
#if HAS_I2C
#if defined(__AVR_XMEGA__)
    /* empty, use i2c_master_writeread() */
#else /* defined(__AVR_XMEGA__) */
    TWDR = b;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    return TW_STATUS;
#endif /* defined(__AVR_XMEGA__) */
#else
    return 0;
#endif
}

/**
 * @brief Read a single byte from I2C in master-receive mode
 *
 * @param *b Pointer to byte to read in
 * @param autoack Auto acknowledge the received byte
 *
 *	@return Status code
 */
static inline uint8_t i2c_readbyte(uint8_t *b, uint8_t autoack)
{
#if HAS_I2C
#if defined(__AVR_XMEGA__)
    /* empty, use i2c_master_writeread() */
#else /* defined(__AVR_XMEGA__) */
    if(autoack)
    {
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    }
    else
    {
        TWCR = (1<<TWINT) | (1<<TWEN);
    }
    while(!(TWCR & (1<<TWINT)));
    *b = TWDR;
    return TW_STATUS;
#endif /* defined(__AVR_XMEGA__) */
#else
    return 0;
#endif
}

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
uint8_t i2c_master_writeread(uint8_t devaddr,
                             uint8_t *writebuf,
                             uint8_t bytestowrite,
                             uint8_t *readbuf,
                             uint8_t bytestoread)
{
#if HAS_I2C
    uint8_t ret = 0;

#if defined(__AVR_XMEGA__)
    twiinst->MASTER.ADDR = devaddr & ~0x01; /* write action */
    twi_wait();

    ret = (0 == (TWI_MASTER_RXACK_bm & twiinst->MASTER.STATUS));

    bytestowrite += 1; /* because we pre-decrement in loop condition */
    while( --bytestowrite && (0 == (TWI_MASTER_RXACK_bm & twiinst->MASTER.STATUS)))
    {
        twiinst->MASTER.DATA = *writebuf++;
        twi_wait();
    }

    if(bytestoread > 0)
    {
        twiinst->MASTER.ADDR = devaddr | 0x01; /* read action, same address as set before */
        twi_wait();
        *readbuf++ = twiinst->MASTER.DATA;

        bytestoread += 1; /* because we pre-decrement in loop condition */
        while(--bytestoread && (0 == (TWI_MASTER_RXACK_bm & twiinst->MASTER.STATUS)))
        {
            twiinst->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
            twi_wait();
            *readbuf++ = twiinst->MASTER.DATA;
        };
    }

    twiinst->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
    return ret;
#else /* defined(__AVR_XMEGA__) */
    uint8_t stat;
    uint8_t cmd = 0;
    if(0 < bytestowrite)
    {
        stat = i2c_startcondition();
        if(TW_START == stat)
        {
            cmd = ((devaddr<<1) & 0xFE) | TW_WRITE;
            stat = i2c_writebyte( cmd );
            if(TW_MT_SLA_ACK == stat)
            {
                ret = 1;
                do
                {
                    stat = i2c_writebyte(*writebuf++);
                }
                while(--bytestowrite && (TW_MT_DATA_ACK == stat));
            }
        }
    }

    if(0 < bytestoread)
    {
        stat = i2c_startcondition();

        /* either START or REP_START condition, depending on previous
         * write access or not
         */
        if((TW_REP_START == stat) || (TW_START == stat))
        {
            cmd = ((devaddr<<1) & 0xFE) | TW_READ;
            stat = i2c_writebyte( cmd );
            if(TW_MR_SLA_ACK == stat)
            {
                ret = 1;
                do
                {
                    /* NAK for the last byte, ACK else */
                    stat = i2c_readbyte(readbuf++, (1==bytestoread)?0:1);
                }
                while( --bytestoread && (TW_MR_DATA_ACK == stat));
            }
        }
    }

    i2c_stopcondition();

    return ret;
#endif /* defined(__AVR_XMEGA__) */
#else
    return 0;
#endif
}

/**
 * @brief Probe I2C device address by trying to read an arbitrary register
 *
 * @param devaddr  Device address 7-bit left-aligned
 * @return 0:  error, 1: success
 *
 */
uint8_t i2c_probe(uint8_t devaddr)
{
#if HAS_I2C
    uint8_t wbuf[1] = {0x01}; /* in fact, register address of sensor to be probed */
    uint8_t rbuf[1];

    return(i2c_master_writeread(devaddr, wbuf, 1, rbuf, 1) != 0);
#else
    return 0;
#endif
}

/* EOF */
