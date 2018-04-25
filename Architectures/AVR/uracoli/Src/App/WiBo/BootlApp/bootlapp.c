/* Copyright (c) 2010 - 2015 Daniel Thiele, Axel Wachtler
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

/**
 * @file
 * @brief Wireless bootloader application, resides in bootloader section
 *
 * @author Daniel Thiele
 *
 * @ingroup grpAppWiBo
 *
 *
 * Description of compile flavours:
 *
 * WIBO_FLAVOUR_KEYPRESS
 *   run bootloader only if key is pressed
 *
 * WIBO_FLAVOUR_KEYPRESS_KEYNB
 *   only valid if WIBO_FLAVOUR_KEYPRESS is defined, the number of the key
 *   that is evaluated if stay in bootloader or jump to application
 *
 *
 * WIBO_FLAVOUR_TIMEOUT <n>
 *   stay in bootloader until <n> seconds have elapsed; timeout is reset
 *   upon each valid bootloader packet received, to prevent a premature
 *   exit from bootloader
 *
 *
 * WIBO_FLAVOUR_MAILBOX
 *   flag from application to stay in bootloader, typically a certain register with a secret value
 *
 * WIBO_FLAVOUR_MAILBOX_REGISTER
 *    the register to find secret value
 *    example: #define WIBO_FLAVOUR_MAILBOX_REGISTER (GPRR0)
 * WIBO_FLAVOUR_MAILBOX_CODE
 *    the secret value
 *    example: #define WIBO_FLAVOUR_MAILBOX_CODE (0xB5)
 */

/* avr-libc inclusions */
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>

/* uracoli inclusions */
#include <board.h>
#include <ioutil.h>
#include <transceiver.h>
#include <p2p_protocol.h>

#ifndef _SW_VERSION_
#define _SW_VERSION_ (0x05)
#endif

/* serial debug printout via USART0, using uracoli ioutil/hif module */
#ifndef SERIALDEBUG
// #    define SERIALDEBUG (1)
#endif

#if defined(SERIALDEBUG)
#    include <avr/interrupt.h>
#    include <hif.h>
#    define DEBUG(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#    define DEBUG(fmt, ...) // empty
#endif

#define EOL "\r\n"


/* incoming frames are collected here */
static union
{
    uint8_t data[MAX_FRAME_SIZE];
    p2p_hdr_t hdr;
    p2p_wibo_data_t wibo_data;
    p2p_wibo_finish_t wibo_finish;
    p2p_wibo_target_t wibo_target;
    p2p_wibo_addr_t wibo_addr;
} rxbuf;

#define PAGEBUFSIZE (SPM_PAGESIZE)

#if SPM_PAGESIZE > 255
typedef uint16_t pageidx_t;
#else
typedef uint8_t pageidx_t;
#endif

/* the only outgoing command, create in SRAM here
 *
 * hdr is assigned after reading nodeconfig
 */
static p2p_ping_cnf_t pingrep = {
    .hdr.cmd = P2P_PING_CNF,
    .hdr.fcf = 0x8841, /* short addressing, frame type: data, no ACK requested */
    .version = _SW_VERSION_,
    .appname = APP_NAME,
    .boardname = BOARD_NAME
    };

/* collect memory page data here, FLASH and EEPROM */
static uint8_t pagebuf[PAGEBUFSIZE];

/* IEEE802.15.4 parameters for communication */
static node_config_t nodeconfig;

#if defined(WIBO_FLAVOUR_TIMEOUT)
#  if WIBO_FLAVOUR_TIMEOUT * 100ul > 65535
#    error WIBO_FLAVOUR_TIMEOUT too large, max 655
#  endif
static uint16_t wibo_timeout = WIBO_FLAVOUR_TIMEOUT * 100ul; /* in 10 ms steps */
#endif  /* defined(WIBO_FLAVOUR_TIMEOUT) */

/*
 * \brief Support to update WIBO itself
 * Put a little snippet at the end of bootloader that copies code from start
 * of application section to start of bootloader section
 *
 *
 */
#if defined(WIBO_FLAVOUR_BOOTLUP)

void __attribute__ ((section (".bootlup"))) bootlup(void)
{
    const uint32_t addr_to = (uint32_t)BOOTLOADER_ADDRESS*2; /* byte address of bootloader */
    const uint8_t nbpages=3072/SPM_PAGESIZE; /* maximum of 3k bootloader at the moment */
    pageidx_t i;
    for(uint8_t page=0;page<nbpages;page++)
    {
        boot_page_erase(addr_to+page*SPM_PAGESIZE);
        boot_spm_busy_wait ();      // Wait until the memory is erased

        for (i=0; i<SPM_PAGESIZE; i+=2)
        {
            uint16_t w = pgm_read_word(page*SPM_PAGESIZE+i);
            boot_page_fill (addr_to+page*SPM_PAGESIZE+i, w);
        }

        boot_page_write (addr_to+page*SPM_PAGESIZE); // Store buffer in flash page
        boot_spm_busy_wait ();// Wait until the memory is erased
    }

    boot_rww_enable ();

    jump_to_bootloader();
}
#endif /* defined(WIBO_FLAVOUR_BOOTLUP) */

/*
 * \brief Program a page
 * (1) Erase the page containing at the given address
 * (2) Fill the page buffer
 * (3) Write the page buffer
 * (4) Wait for finish
 *
 * @param addr The address containing the page to program
 * @param *buf Pointer to buffer of page content
 */
static inline void boot_program_page(uint32_t addr, uint8_t *buf)
{
    pageidx_t i = SPM_PAGESIZE;

    DEBUG("Write Flash addr=%04lX"EOL, addr);
    boot_page_erase(addr);
    boot_spm_busy_wait();

    do
    {
        /* Set up little-endian word */
        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill(addr + SPM_PAGESIZE - i, w);

        i -= 2;
    } while (i);

    boot_page_write(addr);
    boot_spm_busy_wait();
}

#if defined(WIBO_FLAVOUR_TIMEOUT)
#  if __AVR_ARCH__ > 100
#    error WIBO_FLAVOUR_TIMEOUT currently not supported on Xmega CPUs
#  else /* megaAVR/tinyAVR */
/*
 * Initialize timer 1, running in CTC mode with a rate of 100 Hz.  For
 * megaAVR/tinyAVR, we have to support CPU frequencies between 1 MHz
 * and 16 MHz.  We use prescaler 8 (=> clock between 125 kHz and 2
 * MHz), and set OCR1A to a value between 1250 and 20000.
 */
static void setup_timer(void)
{
    OCR1A = F_CPU / (8 * 100);
    TCCR1B = _BV(WGM12) /* CTC mode */ | _BV(CS11) /* prescaler 8 */;
#    ifdef TIFR1
    TIFR1 = _BV(OCF1A);
#    else
    TIFR = _BV(OCF1A);
#    endif
}

/*
 * Check for timer hitting TOP, happens each 10 ms.  If so, count down
 * the timeout.  Return false if not timed out yet.  Otherwise, stop
 * timer 1, and return true, so the caller can leave the bootloader,
 * and jump to application.
 */
static bool check_timeout(void)
{
#    ifdef TIFR1
    if ((TIFR1 & _BV(OCF1A)) == 0)
        return false;
    TIFR1 = _BV(OCF1A);
#    else
    if ((TIFR & _BV(OCF1A)) == 0)
        return false;
    TIFR = _BV(OCF1A);
#    endif
    if (--wibo_timeout == 0)
    {
        OCR1A = 0;
        TCCR1B = 0;
        return true;
    }
    return false;
}
#  endif  /* __AVR_ARCH__ > 100 */
#endif  /*defined(WIBO_FLAVOUR_TIMEOUT)  */

int __attribute__((OS_main))
main(void)
{
    typedef void (*func_ptr_t)(void) __attribute__((noreturn));
    const func_ptr_t jmp_app = (func_ptr_t) 0x0000;

    /* do not initialize variables to save code space, BSS segment sets all variables to zero
     *
     * compiler throws warning, please ignore
     */
    uint8_t *ptr;
    uint8_t tmp;
    uint16_t datacrc = 0; /* checksum for received data */
#if FLASHEND > 0x7FFF
    uint32_t addr = 0;
#else
    uint16_t addr = 0;
#endif

    pageidx_t pagebufidx = 0;
    uint8_t deaf = 0;

    /* Target for program data
     'F' : flash memory
     'E' : eeprom memory
     'X' : None, dry run
     */
    uint8_t target = 'F'; /* for backwards compatibility */
    uint8_t run_bootloader = 0;

#if defined(WIBO_FLAVOUR_TIMEOUT)
    setup_timer();
    run_bootloader = 1;
#endif
    /* only stay in bootloader if key is pressed */

#if !defined(NO_KEYS)
    KEY_INIT();
    if(KEY_GET() != 0)
    {
        WIBO_MAILBOX_SET();
    }
#endif /* defined(WIBO_FLAVOUR_KEYPRESS) */

    LED_INIT();
    LED_SET(0);

#if BOARD_TYPE == BOARD_PINOCCIO
    nodeconfig.channel = eeprom_read_byte((uint8_t *) 8179);
    nodeconfig.pan_id = eeprom_read_word((uint16_t *) 8180);
    nodeconfig.short_addr = eeprom_read_word((uint16_t *) 8182);
#else
    get_node_config(&nodeconfig);
#endif

    trx_io_init(DEFAULT_SPI_RATE);
    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    TRX_RESET_HIGH();

    trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);

#if (RADIO_TYPE == RADIO_AT86RF230A) || (RADIO_TYPE == RADIO_AT86RF230B)
    trx_reg_write(RG_PHY_TX_PWR, 0x80); /* set TX_AUTO_CRC bit, and TX_PWR = max */
#else
    trx_reg_write(RG_TRX_CTRL_1, 0x20); /* set TX_AUTO_CRC bit */
#endif

    /* setup network addresses for auto modes */
    pingrep.hdr.pan = nodeconfig.pan_id;
    pingrep.hdr.src = nodeconfig.short_addr;

    trx_set_panid(nodeconfig.pan_id);
    trx_set_shortaddr(nodeconfig.short_addr);

    /* use register write to save code space, overwrites Bits CCA_REQUEST CCA_MODE[1] CCA_MODE[0]
     * which is accepted
     */
    trx_reg_write(RG_PHY_CC_CCA, nodeconfig.channel);

#if RADIO_TYPE == RADIO_AT86RF212
    /* reset value, BPSK-40 */
    /* trx_reg_write(RG_TRX_CTRL_2, 0x24); */

    /* +5dBm acc. to datasheet AT86RF212 table 7-15 */
    trx_reg_write(RG_PHY_TX_PWR, 0x84);
#endif /* RADIO_TYPE == RADIO_AT86RF212 */

    trx_reg_write(RG_CSMA_SEED_0, nodeconfig.short_addr); /* some seeding */
    trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);

#if defined(SERIALDEBUG)
    static FILE usart_stdio = FDEV_SETUP_STREAM(hif_putc, NULL, _FDEV_SETUP_WRITE);
    stdout = stderr = &usart_stdio;
    hif_init(HIF_DEFAULT_BAUDRATE);
    printf("WIBO Bootlapp Serial Debug"EOL, 0);
    printf("PANID=0x%04X SHORTADDR=0x%04X CHANNEL=0x%02X"EOL,
            nodeconfig.pan_id, nodeconfig.short_addr, nodeconfig.channel);
#  if defined(GICR) // older devices, like ATmega8
    GICR = _BV(IVCE);
    GICR = _BV(IVSEL);
#  elif defined(MCUCR) // newer ATmegas
    MCUCR = _BV(IVCE);
    MCUCR = _BV(IVSEL);
#  else // Xmega
    _PROTECTED_WRITE(PMIC_CTRL, PMIC_IVSEL_bm);
#  endif
    MCU_IRQ_ENABLE();
#endif

    if(WIBO_MAILBOX_IS_SET())
    {
        run_bootloader = 1;
    }

    while(run_bootloader)
    {
        LED_CLR(0);

#if defined(TRX_IF_RFA1)
        while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_RX_END))
        {
#if defined(WIBO_FLAVOUR_TIMEOUT)
            if (check_timeout() && !WIBO_MAILBOX_IS_SET())
            {
                run_bootloader = 0;
                break;
            }
#endif  /* defined(WIBO_FLAVOUR_TIMEOUT) */
        }
        trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_RX_END); /* clear the flag */
#else
        while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TRX_END))
        {
#if defined(WIBO_FLAVOUR_TIMEOUT)
            if (check_timeout() && !WIBO_MAILBOX_IS_SET() )
            {
                run_bootloader = 0;
                break;
            }
#endif  /* defined(WIBO_FLAVOUR_TIMEOUT) */
        }
#endif

#if defined(WIBO_FLAVOUR_TIMEOUT)
        /* reset timeout */
        wibo_timeout = WIBO_FLAVOUR_TIMEOUT * 100ul; /* in 10 ms steps */
#endif  /* defined(WIBO_FLAVOUR_TIMEOUT) */

        trx_frame_read(rxbuf.data,
                sizeof(rxbuf.data) / sizeof(rxbuf.data[0]), &tmp); /* dont use LQI, write into tmp variable */

        LED_SET(0);
        /* light as long as actions are running */

        switch (rxbuf.hdr.cmd)
        {

        case P2P_PING_REQ:
            if (0 == deaf)
            {
                pingrep.hdr.dst = rxbuf.hdr.src;
                pingrep.hdr.seq++;
                pingrep.crc = datacrc;

                trx_reg_write(RG_TRX_STATE, CMD_TX_ARET_ON);

                /* no need to make block atomic since no IRQs are used */
                TRX_SLPTR_HIGH();
                TRX_SLPTR_LOW();
                trx_frame_write(sizeof(p2p_ping_cnf_t) + 2,
                        (uint8_t*) &pingrep);
                /*******************************************************/

                DEBUG("Pinged by 0x%04X"EOL, rxbuf.hdr.src);

#if defined(TRX_IF_RFA1)
                while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TX_END));
                trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_TX_END); /* clear the flag */
#else
                while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TRX_END));
#endif /* defined(TRX_IF_RFA1) */
                trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
            } /* (0 == deaf) */
            break;

        case P2P_WIBO_TARGET:
            target = rxbuf.wibo_target.targmem;
            DEBUG("Set Target to %c"EOL, target);
            break;

        case P2P_WIBO_RESET:
            DEBUG("Reset"EOL, 0);

            addr = SPM_PAGESIZE; /* misuse as counter */
            ptr = pagebuf;
            do
            {
                *ptr++ = 0xFF;
            } while (--addr);

            addr = 0;
            datacrc = 0;
            pagebufidx = 0;
            deaf = 0;
            break;

        case P2P_WIBO_ADDR:
            DEBUG("Set address: 0x%08lX"EOL, rxbuf.wibo_addr.address);
            addr = rxbuf.wibo_addr.address;
            pagebufidx = 0;
            break;

        case P2P_WIBO_DATA:
#if defined(SERIALDEBUG)
            PRINTF("Data[%d]", rxbuf.wibo_data.dsize);
            for(uint8_t j=0;j<rxbuf.wibo_data.dsize;j++)
            {
                PRINTF(" %02X", rxbuf.wibo_data.data[j]);
            }
            PRINT(EOL);
#endif
            tmp = rxbuf.wibo_data.dsize;
            ptr = rxbuf.wibo_data.data;
            do
            {
                datacrc = _crc_ccitt_update(datacrc, *ptr);
                pagebuf[pagebufidx++] = *ptr;
                if (pagebufidx >= PAGEBUFSIZE)
                {
                    /* LED off to save current and avoid flash corruption
                     *  because of possible voltage drops
                     */
                    LED_CLR(0);
                    if (target == 'F') /* Flash memory */
                    {
                        boot_program_page(addr, pagebuf);
                    }
                    else if (target == 'E')
                    {
                        uint16_t ee_addr = addr; /* map to 16-bit */
                        eeprom_write_block(pagebuf, (void*) ee_addr,
                                pagebufidx);
                        eeprom_busy_wait();
                    }
                    else
                    {
                        /* unknown target, dry run */
                    }

                    /* also for dry run! */
                    addr += SPM_PAGESIZE;
                    pagebufidx = 0;
                }
                ptr++;
            } while (--tmp);
            break;
#if defined(WIBO_FLAVOUR_BOOTLUP)
        case P2P_WIBO_BOOTLUP:
            bootlup();
            break;
#endif

        case P2P_WIBO_FINISH:
            DEBUG("Finish"EOL, 0);
            if (target == 'F') /* Flash memory */
            {
                boot_program_page(addr, pagebuf);
            }
            else if (target == 'E')
            {
                uint16_t ee_addr = addr; /* map to 16-bit */
                eeprom_write_block(pagebuf, (void*) ee_addr, pagebufidx);
                eeprom_busy_wait();
            }
            else
            {
                /* unknown target, dry run */
            }

            /* also for dry run! */
            addr += SPM_PAGESIZE;
            pagebufidx = 0;
            break;

        case P2P_WIBO_EXIT:
            run_bootloader = 0;
            break;

        case P2P_WIBO_DEAF:
            deaf = 1;
            break;
        default:
            /* unknown or unhandled command */
            break;
        }; /* switch (rxbuf.hdr.cmd) */
    }

#if defined(WIBO_FLAVOUR_TIMEOUT)
    wibo_timedout:
        WIBO_MAILBOX_CLR();
#endif  /* defined(WIBO_FLAVOUR_TIMEOUT) */
        DEBUG("Exit"EOL, 0);
#  if defined(GICR) // older devices, like ATmega8
        GICR = _BV(IVCE);
        GICR = 0;
#  elif defined(MCUCR) // newer ATmegas
        MCUCR = _BV(IVCE);
        MCUCR = 0;
#  else // Xmega
        _PROTECTED_WRITE(PMIC_CTRL, 0);
#  endif
        LED_CLR(0);
        boot_rww_enable();
#if defined(__AVR_ATmega256RFR2__)
        __asm__ volatile("clr   r30     \n\t"
                    "clr    r31     \n\t"
                    "ijmp   \n\t" );
#else
        jmp_app();
#endif


}

/* EOF */
