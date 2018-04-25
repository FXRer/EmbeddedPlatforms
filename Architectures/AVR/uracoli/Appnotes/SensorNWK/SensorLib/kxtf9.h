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

#ifndef KXTF9_H_
#define KXTF9_H_

/* === includes ============================================================ */

/* === macros ============================================================== */

/* Register Map */
#define RG_KXTF9_XOUT_HPF_L (0x00)
#define RG_KXTF9_XOUT_HPF_H (0x01)
#define RG_KXTF9_YOUT_HPF_L (0x02)
#define RG_KXTF9_YOUT_HPF_H (0x03)
#define RG_KXTF9_ZOUT_HPF_L (0x04)
#define RG_KXTF9_ZOUT_HPF_H (0x05)
#define RG_KXTF9_XOUT_L (0x06)
#define RG_KXTF9_XOUT_H (0x07)
#define RG_KXTF9_YOUT_L (0x08)
#define RG_KXTF9_YOUT_H (0x09)
#define RG_KXTF9_ZOUT_L (0x0A)
#define RG_KXTF9_ZOUT_H (0x0B)
#define RG_KXTF9_DCST_RESP (0x0C)
#define RG_KXTF9_WHO_AM_I (0x0F)
#define RG_KXTF9_TILT_POS_CUR (0x10)
#define RG_KXTF9_TILT_POS_PRE (0x11)
#define RG_KXTF9_INT_SRC_REG1 (0x15)
#define RG_KXTF9_INT_SRC_REG2 (0x16)
#define RG_KXTF9_STATUS_REG (0x18)
#define RG_KXTF9_INT_REL (0x1A)
#define RG_KXTF9_CTRL_REG1 (0x1B)
#define RG_KXTF9_CTRL_REG2 (0x1C)
#define RG_KXTF9_CTRL_REG3 (0x1D)
#define RG_KXTF9_INT_CTRL_REG1 (0x1E)
#define RG_KXTF9_INT_CTRL_REG2 (0x1F)
#define RG_KXTF9_INT_CTRL_REG3 (0x20)
#define RG_KXTF9_DATA_CTRL_REG (0x21)
#define RG_KXTF9_TILT_TIMER (0x28)
#define RG_KXTF9_WUF_TIMER (0x29)
#define RG_KXTF9_TDT_TIMER (0x2B)
#define RG_KXTF9_TDT_H_THRESH (0x2C)
#define RG_KXTF9_TDT_L_THRESH (0x2D)
#define RG_KXTF9_TDT_TAP_TIMER (0x2E)
#define RG_KXTF9_TDT_TOTAL_TIMER (0x2F)
#define RG_KXTF9_TDT_LATENCY_TIMER (0x30)
#define RG_KXTF9_TDT_WINDOW_TIMER (0x31)
#define RG_KXTF9_SELF_TEST (0x3A)
#define RG_KXTF9_WUF_THRESH (0x5A)
#define RG_KXTF9_TILT_ANGLE (0x5C)
#define RG_KXTF9_HYST_SET (0x5F)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    void kxtf9_init(uint8_t addr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KXTF9_H_ */
