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

#ifndef SENSORNWK_H_
#define SENSORNWK_H_

/* === includes ============================================================ */
#include <board.h>

/* === macros ============================================================== */

typedef struct
{
    /* start of 802.15.4 conform MAC header */
    uint16_t FCF;			/**< Frame control field acc. to 802.15.4 */
    uint8_t seqnumber;		/**< sequence number */
    uint16_t destpanid;		/**< PAN ID of destination */
    uint16_t destaddr;		/**< Short address of destination */
    uint16_t srcaddr;		/**< Short address of source */
    /* end of 802.15.4 conform MAC header */

    uint8_t boardtype;		/**< board type identifier */
    uint8_t frametype;		/**< frame type identifier */
} sensornwk_hdr_t;

typedef struct
{
    uint16_t crc;
} sensornwk_ftr_t;

typedef struct
{
    sensornwk_hdr_t hdr;
    sensornwk_ftr_t ftr;
} sensornwk_association_req_t;

typedef struct
{
    sensornwk_hdr_t hdr;
    sensornwk_ftr_t ftr;
} sensornwk_association_rep_t;


/* === types =============================================================== */

typedef enum
{
    SENSORNWK_DEVTYPE_NONE,
    SENSORNWK_DEVTYPE_COORD, /**< main powered coordinator: always Rx */
    SENSORNWK_DEVTYPE_NODE, /**< battery powered node: async Tx + Rx slot + Sleep */
} sensornwk_devtype_t;

typedef enum
{

    /* Commands that are common for all boards */
    SENSORNWK_FRAMETYPE_NOP,				/**< NOP */
    SENSORNWK_FRAMETYPE_HEREAMI,			/**< here am i, sent on each startup */
    SENSORNWK_FRAMETYPE_WUART,			/**< Wireless UART, interpret frame data as binary stream */
    SENSORNWK_FRAMETYPE_PING_REQ,			/**< Ping */
    SENSORNWK_FRAMETYPE_PING_REP,			/**< Reply to a ping */
    SENSORNWK_FRAMETYPE_INFO_REQ,			/**< Get node infomation */
    SENSORNWK_FRAMETYPE_ASSOCIATION_REQ,
    SENSORNWK_FRAMETYPE_ASSOCIATION_REP,
    SENSORNWK_FRAMETYPE_JBOOTL,			/**< Jump into bootloader */

    /* Wibo, all boards */
    SENSORNWK_FRAMETYPE_WIBO_DATA, 		/**< Feed a node with data */
    SENSORNWK_FRAMETYPE_WIBO_FINISH, 		/**< Force a write of all received data */
    SENSORNWK_FRAMETYPE_WIBO_RESET, 		/**< Reset data stream */
    SENSORNWK_FRAMETYPE_WIBO_EXIT, 		/**< Exit bootloader and jump to application vector */

    /* rose 231 */
    SENSORNWK_FRAMETYPE_ROSE_INFO_REP,			/**< Reply node information */
    SENSORNWK_FRAMETYPE_ROSE_DATA,
    SENSORNWK_FRAMETYPE_ROSE_FAST,
    SENSORNWK_FRAMETYPE_ROSE_CFG,

    /* muse231 */
    SENSORNWK_FRAMETYPE_MUSE_INFO_REP,			/**< Reply node information */
    SENSORNWK_FRAMETYPE_MUSE_DATA,
    SENSORNWK_FRAMETYPE_MUSE_ACCSTREAM,
    SENSORNWK_FRAMETYPE_MUSE_CFG,
    SENSORNWK_FRAMETYPE_MUSE_TAP,

    /* museII, both 232 and RFA variant */
    SENSORNWK_FRAMETYPE_MUSEII_DATA,
    SENSORNWK_FRAMETYPE_MUSEII_INFO_REP,
    SENSORNWK_FRAMETYPE_MUSEII_CFG,
    SENSORNWK_FRAMETYPE_MUSEII_ACCSTREAM,
    SENSORNWK_FRAMETYPE_MUSEII_MAGSTREAM,

    /* radiofaro */
    SENSORNWK_FRAMETYPE_RADIOFARO_DATA,

    /* generic sensor data */
    SENSORNWK_FRAMETYPE_SENSORS_DATA,
} sensornwk_frametype_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void sensornwk_init(sensornwk_devtype_t devtype, uint8_t boardtype);
    node_config_t* sensornwk_get_config(void);
    void sensornwk_tx(uint8_t *data, uint8_t len, sensornwk_frametype_t frametype, uint8_t ack_req);
    uint8_t sensornwk_txqueue(uint16_t dest_addr, uint8_t *data, uint8_t len, uint8_t frametype);
    uint8_t * cb_sensornwk_rx(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed);
    void cb_sensornwk_done(void);
    void sensornwk_tx_association_req(void);
    void cb_sensornwk_association_cnf(uint16_t addr);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* SENSORNWK_H_ */
