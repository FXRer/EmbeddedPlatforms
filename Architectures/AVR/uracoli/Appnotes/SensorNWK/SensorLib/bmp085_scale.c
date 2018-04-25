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

/* $Id$ */
/**
 * @file Scale uncompensated values of BMP085 UT and UP to the calibration coefficents
 * 		stored in EEPROM
 * @brief ....
 * @_addtogroup grpApp...
 */

#include "bmp085.h"

static long b5;

/*
 * Scale BMP085 pressure
 *
 * @return Pressure in [Pa]
 */
float bmp085_scale_up(uint32_t up, uint8_t oss, bmp085_calibration_t *cal)
{
    long pressure,x1,x2,x3,b3,b6;
    unsigned long b4, b7;


    b6 = b5 - 4000;
    //*****calculate B3************
    x1 = (b6*b6) >> 12;
    x1 *= cal->b2;
    x1 >>=11;

    x2 = (cal->ac2*b6);
    x2 >>=11;

    x3 = x1 +x2;

    b3 = (((((long)cal->ac1 )*4 + x3) << oss) + 2) >> 2;

    //*****calculate B4************
    x1 = (cal->ac3* b6) >> 13;
    x2 = (cal->b1 * ((b6*b6) >> 12) ) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (cal->ac4 * (unsigned long) (x3 + 32768)) >> 15;

    b7 = ((unsigned long)(up - b3) * (50000 >> oss));
    pressure = (b7 / b4) * 2;

    x1 = pressure >> 8;
    x1 *= x1;
    x1 = (x1 * 3038) >> 16;
    x2 = (pressure * -7357) >> 16;
    pressure += (x1 + x2 + 3791) >> 4;	/* pressure in Pa */

    return (pressure);
}

/*
 * Scale BMP085 temperature
 *
 * @return Temperature in [degC]
 */
float bmp085_scale_ut(uint16_t ut, bmp085_calibration_t *cal)
{
    short temperature;
    long x1,x2;

    x1 = (((long) ut - (long) cal->ac6) * (long) cal->ac5) >> 15;
    x2 = ((long) cal->mc << 11) / (x1 + cal->md);
    b5 = x1 + x2;

    temperature = ((b5 + 8) >> 4);  /* temperature in 0.1°C */

    return (temperature / 10.0);
}


/* EOF */
