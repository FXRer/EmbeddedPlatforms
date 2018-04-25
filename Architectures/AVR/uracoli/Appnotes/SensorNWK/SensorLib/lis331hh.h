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

#ifndef LIS331HH_H
#define LIS331HH_H

/* === includes ============================================================ */

/* === macros ============================================================== */

/* Register Map */
#define RG_LIS331HH_CTRL_REG1 (0x20)
#define RG_LIS331HH_CTRL_REG2 (0x21)
#define RG_LIS331HH_CTRL_REG3 (0x22)
#define RG_LIS331HH_CTRL_REG4 (0x23)
#define RG_LIS331HH_CTRL_REG5 (0x24)
#define RG_LIS331HH_HP_FILTER_RESET (0x25)
#define RG_LIS331HH_REFERENCE (0x26)
#define RG_LIS331HH_STATUS_REG (0x27)
#define RG_LIS331HH_OUT_X_L (0x28)
#define RG_LIS331HH_OUT_X_H (0x29)
#define RG_LIS331HH_OUT_Y_L (0x2A)
#define RG_LIS331HH_OUT_Y_H (0x2B)
#define RG_LIS331HH_OUT_Z_L (0x2C)
#define RG_LIS331HH_OUT_Z_H (0x2D)
#define RG_LIS331HH_INT1_CFG (0x30)
#define RG_LIS331HH_INT1_SOURCE (0x31)
#define RG_LIS331HH_INT1_THS (0x32)
#define RG_LIS331HH_INT1_DURATION (0x33)
#define RG_LIS331HH_INT2_CFG (0x34)
#define RG_LIS331HH_INT2_SOURCE (0x35)
#define RG_LIS331HH_INT2_THS (0x36)
#define RG_LIS331HH_INT2_DURATION (0x37)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void lis331_init(uint8_t addr);
    void lis331_trigger(void);
    void lis331_powerdown(void);
    void lis331hh_readresult(sensors_xyz_result_t *result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef LIS331HH_H */
