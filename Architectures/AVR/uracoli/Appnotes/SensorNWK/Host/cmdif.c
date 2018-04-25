/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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
 * @brief UART command interface
 *
 * @ingroup
 */

/* === includes ============================================================ */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include <board.h>
#include <radio.h>

#include "cmdif.h"
#include <sensornwk.h>
#include <sensorproto.h>

#include <i2c.h>
#include <hmc5883l.h>
#include <sht21.h>
#include <bma250.h>
#include <bmp085.h>
#include <ak8975.h>

/* === macros ============================================================== */
#define MAXLINELEN (80)

/* === types =============================================================== */

/* === globals ============================================================= */
static uint8_t lnbuf[MAXLINELEN + 1];

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/*
 * \brief Wait for complete line, no character echoing
 *
 * @return 1 for line completed, 0 else
 */
static inline uint8_t getline()
{
    uint16_t inchar;
    static uint8_t idx = 0;

    inchar = getchar();
    if (EOF != inchar)
    {
        lnbuf[idx] = 0x00; /* NULL terminated string */
        if ((inchar == '\n') || (inchar == '\r'))
        {
            idx = 0;
            return 1;
        }
        else if (idx < MAXLINELEN)
        {
            lnbuf[idx++] = inchar;
        }
        else
        {
            /* TODO: catch line length overflow */
        }
    }

    return 0;
}

/**
 * @brief Split a null terminated string.
 *
 * This function creates argc,argv style data from a null
 * terminated string. The splitting is done on the base of
 * spaces (ASCII 32).
 *
 * @param  txtline  string to split
 * @param  maxargs  maximum number of arguments to split
 * @retval argv     array of pointers, that store the arguments
 * @return number of arguments splitted (argc)
 */
static inline int split_args(char *txtline, int maxargs, char **argv)
{
    uint8_t argc = 0, nextarg = 1;

    while ((*txtline != 0) && (argc < maxargs))
    {
        if (*txtline == ' ')
        {
            *txtline = 0;
            nextarg = 1;
        }
        else
        {
            if (nextarg)
            {
                argv[argc] = txtline;
                argc++;
                nextarg = 0;
            }
        }
        txtline++;
    }

    return argc;
}

/*
 * \brief Command to echo a string
 * This is intended to check the UART connection
 *
 * @param params Pointer to list of strings containing the arguments
 *               checking for valid number of arguments is done outside this function
 *
 */
static inline void cmd_echo(const char **params)
{
    printf("%s\n", params[0]);
}

static inline void cmd_info(const char **params)
{
    uint16_t addr = strtoul(params[0], (void*) 0x0000, 16);
    if (0 == sensornwk_txqueue(addr, (uint8_t*) 0x0000, 0,
                               SENSORNWK_FRAMETYPE_INFO_REQ))
    {
        printf("Requesting info from 0x%04X\n", addr);
    }
    else
    {
        printf("Tx queue error\n");
    }

}

static inline void cmd_jbootl(const char **params)
{
    uint16_t addr = strtoul(params[0], (void*) 0x0000, 16);
    if (0 == sensornwk_txqueue(addr, (uint8_t*) 0x0000, 0,
                               SENSORNWK_FRAMETYPE_JBOOTL))
    {
        printf("Jumping to bootloader from 0x%04X\n", addr);
    }
    else
    {
        printf("Tx queue error\n");
    }

}

static inline void cmd_setch(const char **params)
{
    uint8_t ch = strtoul(params[0], (void*) 0x00, 10);
    if ((11 <= ch) && (26 >= ch))
    {
        printf("Setting Channel to %d\n", ch);
        radio_set_param(RP_CHANNEL(ch));
    }
    else
    {
        printf("Invalid Channel %d, allowed [11..26]\n", ch);
    }
}

static inline void cmd_cfg(const char **params)
{
    uint16_t short_addr = strtoul(params[0], (void*) 0x0000, 16);
    static sensorproto_muse_cfg_t cfg;

    cfg.wdto = strtoul(params[2], (void*) 0x0000, 16);
    if (0 == sensornwk_txqueue(short_addr, (uint8_t*) &cfg,
                               sizeof(sensorproto_muse_cfg_t),
                               SENSORNWK_FRAMETYPE_MUSE_CFG))
    {
        printf("Queuing for SHORT_ADDR=0x%02X\n", short_addr);
    }
    else
    {
        printf("Queuing failed\n");
    }
}

static inline void cmd_i2cscan(const char **params)
{
    printf("--Scanning I2C [0x00 .. 0x7F]--\n");

    for(uint8_t i=0; i<0x7F; i++)
    {
        uint8_t addr = i<<1;

        if(i2c_probe(addr))
        {
            printf("I2C.Device at %02X\n", addr);
        }
        _delay_us(100);
    }
}

static inline void cmd_hmc5883l(const char **params)
{
    uint8_t addr = SENSORS_I2CADDR_HMC5883;
    sensors_xyz_result_t xyz;
    sensors_xyz_result_t cal_positive, cal_negative;

    printf("-- HMC5883L --\n");

    printf("Probing (0x%02X) .. ", addr);
    if(
        ('H' == sensors_regrd(addr, RG_HMC5883L_IDENT_A)) &&
        ('4' == sensors_regrd(addr, RG_HMC5883L_IDENT_B)) &&
        ('3' == sensors_regrd(addr, RG_HMC5883L_IDENT_C))

    )
    {
        printf("OK\n");

        hmc5883l_init(addr);
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_B, 0x60);
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x10);

        /* HMC5883L calibration values
         * Self-test Measurement of HMC5883L is used, one structure for positive
         * bias (X=+1.16 Ga, Y=+1.16 Ga, Z=+1.08 Ga), one for negative bias
         * (X=-1.16 Ga, Y=-1.16 Ga, Z=-1.08 Ga)
         *
         */
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_B, HMC5883L_GAINCAL << 5);
        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x11); /* positive bias */
        hmc5883l_trigger();
        _delay_ms(200);
        hmc5883l_readresult(&cal_positive);

        sensors_regwr(SENSORS_I2CADDR_HMC5883, RG_HMC5883L_CONFIG_A, 0x12); /* negative bias */
        hmc5883l_trigger();
        _delay_ms(200);
        hmc5883l_readresult(&cal_negative);

        hmc5883l_trigger();
        _delay_ms(50);
        hmc5883l_readresult(&xyz);

        printf(", CX[Ga]=%f, CY[Ga]=%f, CZ[Ga]=%f\n",
               (double)sensors_hmc5883l_scale(xyz.x,
                                              cal_positive.x,
                                              cal_negative.x,
                                              3,
                                              'X' ),
               (double)sensors_hmc5883l_scale(xyz.y,
                                              cal_positive.y,
                                              cal_negative.y,
                                              3,
                                              'Y' ),
               (double)sensors_hmc5883l_scale(xyz.z,
                                              cal_positive.z,
                                              cal_negative.z,
                                              3,
                                              'Z' )
              );
    }
    else
    {
        printf("FAIL\n");
    }
}

static inline void cmd_sht21(const char **params)
{
    uint16_t t, rh;

    sht21_init(SENSORS_I2CADDR_SHT21);

    sht21_triggerMeas(STH21_CMD_TMEAS_NOHOLD);
    _delay_ms(100);
    sht21_readMeas(&t);

    sht21_triggerMeas(STH21_CMD_RHMEAS_NOHOLD);
    _delay_ms(100);
    sht21_readMeas(&rh);

    printf("-- SHT21 --\n");
    printf("T=%f[degC], RH=%f[degC]\n", (double)sht21_scale_temperature(t), (double)sht21_scale_humidity(rh));
}

static inline void cmd_bma250(const char **params)
{
    uint8_t addr = SENSORS_I2CADDR_BMA250;
    sensors_xyz_result_t xyz;
    float x,y,z;
    float sens;
    uint8_t t;

    printf("-- BMA250 --\n");
    printf("Probing (0x%02X) .. ", addr);
    if(i2c_probe(addr))
    {
        printf("OK\n");

        bma250_init(addr);

        bma250_trigger();
        bma250_readresult(&xyz);

        bma250_trigger();
        t=bma250_read_temperature();

        sens = 3.9;	/* 3.9mg/LSB */
        x=bma250_scale_acceleration(xyz.x, sens);
        y=bma250_scale_acceleration(xyz.y, sens);
        z=bma250_scale_acceleration(xyz.z, sens);

        printf("X=%f[g], Y=%f[g], Z=%f[g], ABS=%f[g]\n", x,y,z, (double)sqrt(x*x+y*y+z*z) );
        printf("T=%f[degC]\n", (double)bma250_scale_temperature(t));
    }
    else
    {
        printf("FAIL\n");
    }

}

static inline void cmd_bmp085(const char **params)
{
    static bmp085_calibration_t cal;
    uint16_t ut;
    uint32_t up;

    float t, p;

    bmp085_init(SENSORS_I2CADDR_BMP085);
    bmp085_readee(&cal);

    bmp085_ut_trigger();
    _delay_ms(10);
    ut = bmp085_ut_read();

    bmp085_up_trigger();
    _delay_ms(30);
    up = bmp085_up_read();

    t=bmp085_scale_ut(ut, &cal);
    p=bmp085_scale_up(up, BMP085_DEFAULT_OSRS, &cal);

    printf("-- BMP085 --\n");
    printf("AC1=%d, AC2=%d, AC3=%d, AC4=%u, AC5=%u, AC6=%u, B1=%d, B2=%d, MB=%d, MC=%d, MD=%d\n",
           cal.ac1, cal.ac2, cal.ac3,
           cal.ac4, cal.ac5, cal.ac6,
           cal.b1, cal.b2, cal.mb, cal.mc, cal.md);
    printf("UT=%u UP=%lu\n", ut, up);
    printf("T=%f[degC], P=%f[Pa]\n", (double)t, (double)p);
}

static inline void cmd_ak8975(const char **params)
{
    uint8_t addr = SENSORS_I2CADDR_AK8975;
    sensors_xyz_result_t xyz;

    ak8975_init(addr);

    printf("-- AK8975 --\n");
    printf("Probing (0x%02X) .. ", addr);
    if(0x48 == sensors_regrd(addr, RG_AK8975_WIA))
    {
        printf("OK\n");

        ak8975_trigger();
        _delay_ms(10);
        ak8975_readresult(&xyz);

        printf("X=%d, Y=%d, Z=%d\n", xyz.x, xyz.y, xyz.z);
        printf("X=%f[uT], Y=%f[uT], Z=%f[uT]\n", xyz.x*0.3, xyz.y*0.3, xyz.z*0.3);
    }
    else
    {
        printf("FAIL\n");
    }
}

/* List of commands that are available at the command interface */
const struct
{
    const char *name; /**< Name of the command */
    void (*execfunc)(const char**); /**< Function to be called */
    uint8_t nbparams; /**< Expected number of parameters */
    const char *description; /**< Textual description of the command to print help screen */
} commands[] =
{
    { "echo", cmd_echo, 1, "Echo a string" },
    { "info", cmd_info,	1, "Get node information" },
    { "setch", cmd_setch, 1, "Set Channel" },
    { "jbootl", cmd_jbootl, 1, "Jump to bootloader" },
    { "i2cscan", cmd_i2cscan, 0, "Scan for I2C devices" },
    { "hmc5883l", cmd_hmc5883l, 0, "Sensor HMC5883L" },
    { "sht21", cmd_sht21, 0, "Sensor SHT21" },
    { "bma250", cmd_bma250, 0, "Sensor BMA250" },
    { "bmp085", cmd_bmp085, 0, "Sensor BMP085" },
    { "ak8975", cmd_ak8975, 0, "Sensor AK8975" },
    { "cfg", cmd_cfg, 3, "Config Node" },
};

/*
 * \brief Parsing a shell input line
 *
 * @param *ln String containing the command line to parse
 */
static inline void process_cmdline(char *ln)
{
    char *params[10];
    uint8_t nbparams;
    uint8_t i;
    uint8_t found = 0;

    nbparams = split_args(ln, 10, params);

    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (!strcasecmp(params[0], commands[i].name))
        {
            if (commands[i].nbparams == (nbparams - 1))   /* substract the command itself */
            {
                commands[i].execfunc((const char**) &params[1]);
                found = 1;
            }
            else
            {
                puts_P(PSTR("ERR Parameter"));
            }
        }
    }

    if (!found)
    {
        puts_P(PSTR("ERR Unknown command"));
    }
}

void cmdif_task(void)
{
    if (getline())
    {
        process_cmdline((char*) lnbuf);
    }
}

/* EOF */
