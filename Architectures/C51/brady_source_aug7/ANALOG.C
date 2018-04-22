//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net ANALOG.C
//
// This module handles the analog inputs which are external temperature
// sensor, the on-chip temperature sensor, and operating voltage.
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include "C8051f.h"
#include "net.h"
#include "serial.h"
#include "analog.h"

extern char xdata text[];
UINT idata cpu_temperature, air_temperature, cpu_voltage;
UCHAR idata mux_select;



//--------------------------------------------------------------------------
// Initialize the A/D converter
// 
//--------------------------------------------------------------------------
void init_adc(void)
{
	ADC0CN = 0x40;		// Disable ADC, right justified, norm tracking
	REF0CN = 0x07;		// Enable internal reference
	AMX0CF = 0x00;		// Set all inputs to single ended
	mux_select = MUX_CPU_TEMP;	  // CPU on-chip temp sensor
	AMX0SL = MUX_CPU_TEMP; 	
	ADC0CF = 0x80;		// Sysclk / 16, gain = 1
	ADCEN = 1;			// Enable ADC			
}



//--------------------------------------------------------------------------
// This function is a little state machine which reads one analog
// inputs at a time, out of the 3 possible inputs
//  1. On-chip temperature
//  2. External air temperature
//  3. CPU operating voltage
//--------------------------------------------------------------------------
void read_analog_inputs(void)
{
	UINT delay;
	UINT idata ad_counts;
   ULONG idata temp_long;
   static UINT save_ad_counts;

	ADBUSY = 1;		 // Start a conversion
	
	// Wait 20 usec here, alternatively could poll ADBUSY and
	// wait for it to go to zero
	delay = 40;	 	
   while (delay--);
   
	// Data is right justified so high 4 bits are in ADC0H and
	// low 8 bits are at high end of ADC0L.  Full scale is 2.43
	// volts typical which produces 4096 counts. All the math
	// is done in fixed point which is much faster than than
	// floating point on an 8051.
	ad_counts = ((0x0F & ADC0H) << 8) | ADC0L;
		
	switch (mux_select)
	{
		case MUX_CPU_TEMP:
		// This is the on-chip temperature sensor. Using the data
		// sheet equation V = 0.00286T(Celsius) + 0.776
      // This becomes  T(Farenheit) = 0.37338 * A/D - 456.4 
		// which I approximate with T = 28/75 * ad_counts - 456
		if (ad_counts < 2000)
      {
		   cpu_temperature = ((28 * ad_counts) / 75) - 456;
		}
		else cpu_temperature = 0;
		AMX0SL = MUX_CPU_VOLTS;		// Select AIN1 for next read
		mux_select = MUX_CPU_VOLTS;
		break;

	   case MUX_CPU_VOLTS:
		// The A/D sees CPU operating voltage divided by 2.00
		// Scale result so it is 100 X actual voltage
		// Conversion is 1685.6 counts per volt so compute
		// Volts * 100 = 0.11865 * ad_counts 
		// which I approximate with 7/59 * ad_counts 
		cpu_voltage = (7 * ad_counts) / 59;
		
		// Save ad_counts for computing air temperature
      save_ad_counts = ad_counts;
		AMX0SL = MUX_AIR_TEMP;		// Select on-chip temp sensor
		mux_select = MUX_AIR_TEMP;
		break;

      	
		case MUX_AIR_TEMP:
		// This is thermistor to sense air temperature 
		// Using ratio here gets rid of voltage dependency
		// Computation is based on a curve fit to thermistor table
		// T (Farenheit) = 168.41 - 91.011 * ratio of ad_counts
		temp_long = 91L * (ULONG)ad_counts;
      air_temperature =  168 - (UINT)(temp_long / save_ad_counts);
                        
      AMX0SL = MUX_CPU_TEMP;	 
      mux_select = MUX_CPU_TEMP;
		break;


		default:
		// Should not get here but if it does, switch
		// to reading CPU temperature
		AMX0SL = MUX_CPU_TEMP;
		mux_select = MUX_CPU_TEMP;
		break;
  	}
}
 

 