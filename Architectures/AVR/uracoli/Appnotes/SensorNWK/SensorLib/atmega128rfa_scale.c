/*
 * atmega128rfa.c
 *
 *  Created on: 25.03.2012
 *      Author: dthiele
 */

#include "atmega128rfa.h"

float atmega128rfa_scale_temperature(uint16_t adcraw)
{
    return(1.13*adcraw - 272.8);
}

/* EOF */
