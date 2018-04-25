/* Copyright (c) 2014, Daniel Thiele
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

#ifndef RTC_H
#define RTC_H

/**
 * @addtogroup grpTimer
 * @{
 */

/* === includes ============================================================ */

/* === macros ============================================================== */

/* the following macros are defined by avr-gcc
 *
 * order is important because __AVR_MEGA__ is also defined on Xmega devices
 */
#if defined(__AVR_XMEGA__)
#define RTC_AVR_XMEGA
#elif defined(__AVR_MEGA__)
#define RTC_AVR_MEGA
#else
#error "MCU class not defined, dont know what to do"
#endif

/* setup macros to end up with a tick of 1 second */
#if defined(RTC_AVR_MEGA)
#define RTC_PRESCALER (256)
#define RTC_PRESCALER_BITS (6)
#define RTC_HWTICK_NB (128)
#elif defined(RTC_AVR_MEGA_WATCHDOG)
    /* empty */
#elif defined(RTC_AVR_XMEGA)
    /* empty */
#else
    /* empty */
#endif /* defined(RTC_MEGA_AVR) */

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTC initialisation
 *
 * @param *tickfunc Pointer to rtc callback function.
 * @ingroup grpRTC
*/
void rtc_init(void (*tickfunc)(void));

/**
 * @brief This function starts the RTC
 *
 */
void rtc_start(void);

/**
 * @brief This function stops the RTC
 *
 */
void rtc_stop(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef RTC_H */
