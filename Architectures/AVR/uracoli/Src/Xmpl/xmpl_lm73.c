/* Copyright (c) 2013 Axel Wachtler
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

/* $Id$ */
/* Example for using the I2C ISL29020 light sensor */
#include <stdlib.h>
#include "board.h"
#include "ioutil.h"
#include "i2c.h"
#include "sensors/lm73.h"
#include "hif.h"
#include "xmpl.h"


/* === includes ============================================================ */

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
bool do_measure;

/* === prototypes ========================================================== */
bool process_command(int chr, lm73_ctx_t *plm73);
void reg_dump(lm73_ctx_t *plm73);

/* === functions =========================================================== */

void xmpl_init(void)
{
    mcu_init();
    /* setting up UART and adjusting the baudrate */
    hif_init(HIF_DEFAULT_BAUDRATE);
    LED_INIT();
    LED_SET(0);
    MCU_IRQ_ENABLE();
#if HIF_TYPE == HIF_AT90USB
    /*
     * Wait for terminal user pressing a key so there is time to
     * attach a terminal emulator after the virtual serial port has
     * been established within the host OS.
     */
    do
    {
        inchar = hif_getc();
    }
    while (EOF == inchar);
#endif

    /* init i2c bus and check presence of sensor */
    i2c_init( 4000000UL );

}

int main(void)
{
lm73_ctx_t lm73;
uint16_t temp_raw;
float temp_fl;
uint8_t status, config;
int chr;

    xmpl_init();

    PRINTF(EOL"LM73 Temerature Sensor Example : %s : %ld bit/s"EOL,
            BOARD_NAME, HIF_DEFAULT_BAUDRATE);

    sensor_create_lm73(&lm73, false, LM73_ADDR, 0);
    if (lm73.g.last_error != SENSOR_ERR_OK)
    {
        PRINTF("ERROR[%d]: init, addr=0x%x"EOL, lm73.g.last_error, LM73_ADDR);
    }
    else
    {
        PRINTF("OK: init, addr=0x%x"EOL, LM73_ADDR);
    }

    do_measure = 1;

    while (1)
    {
        chr = hif_getc();
        if (chr != -1)
        {
            do_measure = process_command(chr, &lm73);
        }
        if (do_measure)
        {
            temp_raw = lm73_get(&lm73);
            temp_fl = lm73_scale(temp_raw);
            DELAY_US(10);
            config =  lm73_byte_read(&lm73, LM73_PTR_CFG);
            DELAY_US(10);
            status =  lm73_byte_read(&lm73, LM73_PTR_STATUS);


            PRINTF("temp_raw=%d %f", temp_raw, temp_fl);
            if ( (config & LM73_CFG_PWR_DOWN) )
            {
                PRINT(" PWR_DOWN");
            }
            if ((status & LM73_STATUS_DAV) == 0)
            {
                PRINT(" NO:DATA");
            }
            if (status & LM73_STATUS_TLOW)
            {
                PRINT(" TLOW");
            }
            if (status & LM73_STATUS_THIGH)
            {
                PRINT(" THIGH");
            }
            PRINT(EOL);
            DELAY_MS(1000);
            LED_TOGGLE(0);
        }
    }
}

bool process_command(int chr, lm73_ctx_t *plm73)
{
    uint8_t tmp, res;
    uint16_t t_limit;
    bool rv = 0;

    if (chr == 'P')
    {
        PRINT("power up"EOL);
        tmp = lm73_byte_read(plm73, LM73_PTR_CFG);
        tmp &= ~LM73_CFG_PWR_DOWN;
        DELAY_US(LM73_T_BUS_FREE_US);
        lm73_byte_write(plm73, LM73_PTR_CFG, tmp);
    }
    else if (chr == 'p')
    {
        PRINT("power down"EOL);
        tmp = lm73_byte_read(plm73, LM73_PTR_CFG);
        tmp |= LM73_CFG_PWR_DOWN;
        DELAY_US(LM73_T_BUS_FREE_US);
        lm73_byte_write(plm73, LM73_PTR_CFG, tmp);
    }
    else if (chr == 'o')
    {
        PRINT("one shot conversion"EOL);
        tmp = lm73_byte_read(plm73, LM73_PTR_CFG);
        tmp |= (LM73_CFG_PWR_DOWN | LM73_CFG_ONE_SHOT);
        DELAY_US(LM73_T_BUS_FREE_US);
        lm73_byte_write(plm73, LM73_PTR_CFG, tmp);
        rv = 1;
    }
    else if (chr == 'r')
    {
        PRINT("register dump"EOL);
        reg_dump(plm73);
    }
    else if (chr == 'L')
    {
        PRINT("enter low limit in °C:");
        t_limit = hif_get_dec_number();
        PRINTF(" %d°C, reg=0x%04x"EOL, t_limit, t_limit * LM73_TEMP_TO_REG_SCALE);
        lm73_set_lower_limit(plm73, t_limit * LM73_TEMP_TO_REG_SCALE );
    }
    else if (chr == 'H')
    {
        PRINT("enter high limit in °C:");
        t_limit = hif_get_dec_number();
        PRINTF(" %d°C, reg=0x%04x"EOL, t_limit, t_limit * LM73_TEMP_TO_REG_SCALE);
        lm73_set_upper_limit(plm73, t_limit * LM73_TEMP_TO_REG_SCALE );
    }
    else if (chr == 'R')
    {
        PRINT("enter resolution [0,1,2,3]:");
        res = hif_get_dec_number();
        PRINTF(" %d\n", res);
        tmp = lm73_byte_read(plm73, LM73_PTR_STATUS);
        tmp &= ~LM73_STATUS_RES;
        tmp |= ((res << 5) & LM73_STATUS_RES);
        DELAY_US(LM73_T_BUS_FREE_US);
        lm73_byte_write(plm73, LM73_PTR_STATUS, tmp);
    }
    else if (chr == 'a')
    {
        PRINT("clear alert bit"EOL);
        tmp = lm73_byte_read(plm73, LM73_PTR_CFG);
        tmp |= LM73_CFG_ALERT_RST;
        DELAY_US(LM73_T_BUS_FREE_US);
        lm73_byte_write(plm73, LM73_PTR_CFG, tmp);
        rv = 1;
    }
    else if (chr == 'm' || chr == ' ')
    {
        PRINT("run measurement"EOL);
        rv = 1;
    }
    else if (chr == 'h')
    {
        PRINT("Help"EOL\
              "P/p - power up/down"EOL\
              "o   - one shot conversion"EOL\
              "r   - print register dump"EOL\
              "H/L - set high/low limit"EOL\
              "R   - set resolution"EOL\
              "a   - reset alert bit"EOL\
              "m   - run measurement"EOL
             );
    }
    return rv;
}

void reg_dump(lm73_ctx_t *plm73)
{
    uint8_t tmp;
    uint16_t temperature;

    temperature = lm73_word_read(plm73, LM73_PTR_TEMP);
    PRINTF("\n"\
           " TEMP[%d]   = 0x%04x"EOL,
           LM73_PTR_TEMP, temperature, temperature * LM73_REG_TO_TEMP_SCALE );

    tmp = lm73_byte_read(plm73, LM73_PTR_CFG);
    PRINTF(" CONFIG[%d] = 0x%02x", LM73_PTR_CFG, tmp);
    if (tmp & LM73_CFG_ONE_SHOT)
    {
        PRINT(" ONE_SHOT");
    }
    if (tmp & LM73_CFG_ALERT_EN)
    {
        PRINT(" ALERT_EN");
    }
    if (tmp & LM73_CFG_ALERT_POL)
    {
        PRINT(" ALERT_POL");
    }
    if (tmp & LM73_CFG_ALERT_RST)
    {
        PRINT(" ALERT_RST");
    }
    if (tmp & LM73_CFG_PWR_DOWN)
    {
        PRINT(" PWR_DOWN");
    }
    PRINT("\n");

    temperature = lm73_word_read(plm73, LM73_PTR_THIGH);
    PRINTF(" THIGH[%d]  = 0x%04x t_high = %f°C"EOL,
           LM73_PTR_THIGH, temperature, temperature * LM73_REG_TO_TEMP_SCALE);

    temperature = lm73_word_read(plm73, LM73_PTR_TLOW);
    PRINTF(" TLOW[%d]   = 0x%04x t_high = %f°C"EOL,
           LM73_PTR_TLOW, temperature, temperature * LM73_REG_TO_TEMP_SCALE);

    tmp = lm73_byte_read(plm73, LM73_PTR_STATUS);
    PRINTF(" STATUS[%d] = 0x%02x", LM73_PTR_STATUS, tmp);
    if (tmp & LM73_STATUS_DAV)
    {
        PRINT(" DAV");
    }
    if (tmp & LM73_STATUS_TLOW)
    {
        PRINT(" TLOW");
    }
    if (tmp & LM73_STATUS_THIGH)
    {
        PRINT(" THIGH");
    }
    if (tmp & LM73_STATUS_ALERT)
    {
        PRINT(" ALERT");
    }
    PRINTF(" 1%cbit", ((tmp & LM73_STATUS_RES) >> 5) + '1');
    PRINT(EOL);

    PRINTF(" ID[%d]     = 0x%04x"EOL""EOL,
           LM73_PTR_ID, lm73_word_read(plm73, LM73_PTR_ID));
}

/* E_O_F */
