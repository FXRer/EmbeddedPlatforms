/* Copyright (c) 2013 - 2015 Daniel Thiele, Tom Magnier, Axel Wachtler
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

#ifndef HardwareRadio_h
#define HardwareRadio_h

/**
 * @file
 * @brief Arduino Radio Class.
 * @addtogroup grpContribArduino
 * @{
 */

#include <string.h>
#include "uracoli/board.h"
#include "uracoli/radio.h"
#include "uracoli/p2p_protocol.h"
#include "Stream.h"

/** Minimun radio channel number in 2.4 GHz band */
#define PHY_MIN_CHANNEL (11)
/** Maximun radio channel number in 2.4 GHz band */
#define PHY_MAX_CHANNEL (26)
/** Default radio channel number in 2.4 GHz band */
#define PHY_DEFAULT_CHANNEL (17)

/** Maximum size of a IEEE 802.15.4 frame */
#define PHY_MAX_FRAME_SIZE (127)

/** Element of a chained list of frame buffers */
typedef struct radio_buffer
{
    /** pointer to next list element or NULL if list terminates */
    struct radio_buffer *next;
    /** Length of payload */
    uint8_t len;
    /** array that can store a maximum IEEE 802.15.4 frame */
    uint8_t frm[PHY_MAX_FRAME_SIZE];
    /** read/write index */
    uint8_t idx;
}
radio_buffer_t;

# if defined(__cplusplus) || defined(DOXYGEN)
/** Hardware Radio class */
class HardwareRadio : public Stream
{
  private:
    uint8_t txseq;
    void txbuffer_init(radio_buffer_t *pbuf);
    void txbuffer_init(radio_buffer_t *pbuf, uint16_t dst);

    /* user reads from this buffer */
    radio_buffer_t *prdbuf;
    /* user writes to this buffer */
    radio_buffer_t *pwrbuf;

    uint16_t srcaddr;
    uint16_t dstaddr;
    uint16_t panid;
  public:
    /* buffer which is currently transmited */
    radio_buffer_t *txbuf;
    /* buffer which is currently used for reception */
    radio_buffer_t *rxbuf;

    volatile uint8_t tx_in_progress;
    node_config_t nc;

    /** Allocate a radio buffer */
    radio_buffer_t * alloc_buffer(void);

    /** Free a radio buffer */
    void free_buffer(radio_buffer_t * pbuf);


    /** constructor */
    HardwareRadio(void);

    /** Starting the hardware radio class with default parameters
     *  @note The following default parameters are used implicitely.
     *        - channel: @ref PHY_DEFAULT_CHANNEL
     *        - idlestate: @ref STATE_RXAUTO
     *        - pan id: @ref DEFAULT_PAN_ID
     *        - destination address: @ref DEFAULT_SHORT_ADDRESS
     *        - source address: @ref DEFAULT_SHORT_ADDRESS
     */
    void begin(void);

   /**  Starting the hardware radio class with explicit parameters
     * @param channel radio channel
     *          (11 - 26 for 2.4GHz radios, 0 - 10 for SubGHz radios)
     *  @param idlestate default state of the radio, supported values are listed
     *           in @ref radio_state_t.
     *  @note The following default parameters are used implicitely.
     *        - pan id: @ref DEFAULT_PAN_ID
     *        - destination address: @ref DEFAULT_SHORT_ADDRESS
     *        - source address: @ref DEFAULT_SHORT_ADDRESS
     */
    void begin(uint8_t channel, uint8_t idlestate);

    /**  Starting the hardware radio class with explicit parameters
     * @param channel radio channel
     *          (11 - 26 for 2.4GHz radios, 0 - 10 for SubGHz radios)
     *  @param idlestate default state of the radio, supported values are listed
     *           in @ref radio_state_t.
     *  @param pan _p_ersonal _a_rea _n_etwor ID
     *  @param dst 16 bit destination address
     *  @param src 16 bit node address
     */
    void begin(uint8_t channel, uint8_t idlestate,
               uint16_t pan, uint16_t dst, uint16_t src);

    /** return number of available bytes in current RX buffer */
    virtual int available(void);

    /** Returns the next byte (character) of incoming data (RX)
     *  without removing it from the internal serial buffer.
     *  @return EOF (-1) if no data available, otherwise a value
     *          from 0 ... 255
     */
    virtual int peek(void);
    /** Returns the next byte (character) of incoming data (RX)
     *  @return EOF (-1) if no data available, otherwise a value
     *          from 0 ... 255
     */
    virtual int read(void);
    /**
     * flush TX and RX queues.
     * RX queue data are discarded, TX data is sent.
     */
    virtual void flush(void);

    /** write a byte to the TX stream */
    virtual size_t write(uint8_t);

    #if ! defined(DOXYGEN)
    using Print::write; // pull in write(str) and write(buf, size) from Print
    #else
    /** write a string to the TX stream
     * @param str \\0 terminated string
     */
    void write(char * str);

    /** write a binary buffer (buf, size) to the TX stream
     * @param buf pointer to the buffer
     * @param size number of bytes in the buffer.
     */
    void write(uint8_t *buf, uint8_t size);
    #endif

    /** send binary data direct to a node, adressed by dst.
     * @param dst destination address
     * @param pbuf pointer to data array
     * @param size number of bytes to transmit
     * @return number of bytes transmitted
     * @note  At the end of this routine @ref flush is called,
     *        and therefore the data collected in @ref write and
     *        and @ref print are sent out to.
     */
    uint8_t sendto(uint16_t dst, uint8_t *pbuf, uint8_t size);

};

#  ifndef HARDWARERADIO_CPP
    extern HardwareRadio Radio;
#  endif
# endif
 /** @} */
#endif /*#ifndef HardwareRadio_h*/
