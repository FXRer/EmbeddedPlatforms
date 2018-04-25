/* Copyright (c) 2013 - 2014 Axel Wachtler
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

#ifndef LEDPS_H
#define LEDPS_H
/**
  @defgroup grpSensorLEDPS LEDPS - Using a LED as Photo Sensor
  @ingroup grpSensorDefs
  @{

   Implementation for using a LED as photo sensor.
   The LED is connected between two port pins, so that it can
   be reverse charged.
 */

/* === includes ============================================================ */
#include "sensor_defs.h"
#include <avr/io.h>

/* === macros ============================================================== */
/** Debgging driver with HIF*/
#define LEDPS_DEBUG (0)
#if LEDPS_DEBUG == 1
# include "hif.h"
# define DBG_LEDPS(...) PRINTF(__VA_ARGS__)
#else
# define DBG_LEDPS(...)
#endif
/** Sampling Macro */
#define DO_LED_PS(port, ddr, pin, anode, cathode, ledps_cnt) \
        do { \
            ddr |= (_BV(anode) | _BV(cathode)); \
            port &= ~_BV(anode); \
            port |= _BV(cathode); \
            _delay_us(5); \
            cli(); \
            ddr &= ~_BV(cathode); \
            port &= ~_BV(cathode); \
            ledps_cnt = 0x7fff; \
            do { \
                _delay_us(20); \
            } while ((pin & _BV(cathode)) && --ledps_cnt);\
            sei(); \
            ddr |= (_BV(anode) | _BV(cathode));\
       } while(0)


/* === types =============================================================== */

/** LED context */
typedef struct
{
    sensor_driver_t g; /** driver structure */
    char portid;
    uint8_t panode;
    uint8_t pcathode;
} ledps_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/* === low level functions ================================================== */


/**
 * @brief sample port
 * Note: The following macros needs to be defined in board_<name>.h
 * @code
 * #define LEDPS_PORTx (1)
 * #define LEDPS_PORT  ('x')
 * #define LEDPS_ANODE (y)
 * #define LEDPS_CATHODE (z)
 * x: name of PORT, upper case letter A...F
 * y: port pin for anode 0 ... 7
 * z: port pin for cathode 0 ... 7
 * @endcode
 */
static inline uint16_t sample_port(char portid, uint8_t anode_pin, uint8_t cathode_pin);
static inline uint16_t sample_port(char portid, uint8_t anode_pin, uint8_t cathode_pin)
{
uint16_t rv = 0xffff;
    /* transform portid to upper case letter */
    if (portid >= 97)
    {
        portid -= 32;
    }
    switch (portid)
    {
        #if defined(LEDPS_PORTA) && defined(PORTA)
        case 'A':
            DO_LED_PS(PORTA, DDRA, PINA, anode_pin, cathode_pin, rv);
            break;
        #endif
        #if defined(LEDPS_PORTB) && defined(PORTB)
        case 'B':
            DO_LED_PS(PORTB, DDRB, PINB, anode_pin, cathode_pin, rv);
            break;
        #endif
        #if defined(LEDPS_PORTC) && defined(PORTC)
        case 'C':
            DO_LED_PS(PORTC, DDRC, PINC, anode_pin, cathode_pin, rv);
            break;
        #endif
        #if defined(LEDPS_PORTD) && defined(PORTD)
        case 'D':
            DO_LED_PS(PORTD, DDRD, PIND, anode_pin, cathode_pin, rv);
            break;
        #endif
        #if defined(LEDPS_PORTE) && defined(PORTE)
        case 'E':
            DO_LED_PS(PORTE, DDRE, PINE, anode_pin, cathode_pin, rv);
            break;
        #endif
        #if defined(LEDPS_PORTF) && defined(PORTF)
        case 'F':
            DO_LED_PS(PORTF, DDRF, PINF, anode_pin, cathode_pin, rv);
            break;
        #endif
        default:
            rv = 0xaaaa;
            break;
    }
    return rv;
}

/* === high level sensor functions ========================================= */
/** this function is empty */
static inline void ledps_trigger(void *pctx, bool one_shot)
{
}

/** Retrieve value from LED-Photo-Sensor */
static inline uint8_t ledps_get_val(void *pctx, uint8_t *pdata)
{
    sensor_light_t *p = (sensor_light_t *)pdata;
    ledps_ctx_t *pcfg = (ledps_ctx_t *)pctx;
    if( p )
    {
        p->type = SENSOR_DATA_LIGHT;
        p->sensor = SENSOR_LEDPS;
        p->light = sample_port(pcfg->portid, pcfg->panode, pcfg->pcathode);
    }
    return sizeof(sensor_light_t);
}

/** Retrieve raw value from LED-Photo-Sensor */
static inline uint8_t ledps_get_raw(void *pctx, uint8_t *pdata)
{
    ledps_ctx_t *pcfg = (ledps_ctx_t*)pctx;
    uint16_t rv;
    if (pdata)
    {
        rv = sample_port(pcfg->portid, pcfg->panode, pcfg->pcathode);
        pdata[0] = 0x81;
        pdata[1] = SENSOR_LEDPS;
        pdata[2] = rv/256;
        pdata[3] = rv&255;
    }
    return 4;
}

/** LED-Photo-Sensor does not support sleep. */
static inline void ledps_sleep(void *pctx)
{
}

/**
 * Create an instance of a ledps sensor and initialize the sensor.
 *
 * @return sizeof(ledps_ctx_t)
 *
 */
static inline uint8_t sensor_create_ledps(void *pdata, bool raw,
                                          char portid, uint8_t panode, uint8_t pcathode)
{
    uint8_t rv = sizeof(ledps_ctx_t);
    ledps_ctx_t *pcfg;

    if (pdata != NULL)
    {
        /* init generic sensor data */
        pcfg = (ledps_ctx_t *)pdata;
        pcfg->g.id = SENSOR_LEDPS;
        pcfg->g.f_trigger = ledps_trigger;
        pcfg->g.f_get_val = raw ?  ledps_get_raw : ledps_get_val;
        pcfg->g.f_sleep = ledps_sleep;
        // ... more functions to setup

        /* init private sensor data */
        pcfg->portid = portid;
        pcfg->panode = panode;
        pcfg->pcathode = pcathode;
        DBG_LEDPS("ledps: port: %c ano: %d, cath: %d\n",
                  pcfg->portid, pcfg->panode, pcfg->pcathode);
        /* initialize sensor hardware */

    }
    return rv;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */
#endif  /* #ifndef LEDPS_H */
