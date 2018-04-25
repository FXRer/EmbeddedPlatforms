/*
  Cosm temperature logger
  
  Remote node application
  
  Hardware:
   - Radiofaro on battery power, temperature sensor internal
*/

#include <avr/wdt.h>
#include <avr/sleep.h>
#include "HardwareRadio.h"

/* === globals ============================================================= */
static volatile uint8_t adcfinished = 0;
static int8_t adc_offset = 0;

/* === functions =========================================================== */

/*
 * \brief Setup watchdog to serve as application timer
 *
 */
static inline void wdt_timersetup(uint8_t timeout)
{
	WDTCSR = (1 << WDCE) | (1 << WDE); /* Enable configuration change */
	WDTCSR = (1 << WDIF) | (1 << WDIE) | /* Enable Watchdog Interrupt Mode */
	(timeout & 0x08 ? _WD_PS3_MASK : 0x00) | (timeout & 0x07);
}

/*
 * \brief Watchdog ISR, used as application timer
 *
 */
ISR(WDT_vect, ISR_NOBLOCK)
{
  /* do nothing, just wake up MCU */
}

ISR(ADC_vect, ISR_BLOCK)
{
	adcfinished = 1;
}

/*
 * \brief Trigger sleep based ADC measurement
 * Function is blocking until flag "adcfinished" is set by ISR
 *
 * @return ADC register value
 */
static inline int16_t adc_measure(void)
{
	set_sleep_mode(SLEEP_MODE_ADC);
	/* dummy cycle */
	adcfinished = 0;
	do
	{
		sleep_mode();
		/* sleep, wake up by ADC IRQ */

		/* check here for ADC IRQ because sleep mode could wake up from
		 * another source too
		 */
	} while (0 == adcfinished); /* set by ISR */
	return ADC ;
}

/**
 * \brief Measure internal temperature sensor via ADC
 *
 * \return The temperature in [degC]
 *
 * \author Daniel Thiele
 */
float measure_tmcu(void)
{
	int32_t val = 0;
	uint8_t nbavg = 5;

	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
	ADCSRB = (1 << MUX5);

	ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX0); /* reference: 1.6V, input Temp Sensor */
	_delay_us(200); /* some time to settle */

	ADCSRA |= (1 << ADIF); /* clear flag */
	ADCSRA |= (1 << ADIE);

	/* dummy cycle after REF change (suggested by datasheet) */
	adc_measure();

	_delay_us(100); /* some time to settle */

	for(uint8_t i=0;i<nbavg;i++){
		val += adc_measure() - adc_offset;
	}

	ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

	return (1.13 * val / (float)nbavg - 272.8);
}

int8_t measure_adc_offset(void)
{
	uint16_t val;

	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
	ADCSRB = 0;
	ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3); /* reference: 1.6V, differential ADC0-ADC0 10x */

	_delay_us(200); /* some time to settle */

	ADCSRA |= (1 << ADIF); /* clear flag */
	ADCSRA |= (1 << ADIE);

	/* dummy cycle after REF change (suggested by datasheet) */
	adc_measure();

	_delay_us(100); /* some time to settle */

	val = adc_measure();

	ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

	return (val);
}

/*
 * \brief Measure internal voltage of ATmega128RFA1
 *
 * Cannot be measured via ADC, use Batmon of TRX part
 *
 */
float measure_vmcu(void)
{
	uint16_t v; /* voltage in [mV] */
	uint8_t vth;

	for(vth=0;vth<32;vth++){
		BATMON = vth & 0x1F;
		BATMON = vth & 0x1F;

		if(0 == (BATMON & (1<<BATMON_OK))){
			break;
		}
	}

	if(vth & 0x10){
		v = 2550 + 75*(vth&0x0F); /* high range */
	}else{
		v = 1700 + 50*(vth&0x0F); /* low range */
	}

	return v / 1000.0;
}

void setup()
{
  /* init all unused pins */
  DDRB = 0x00;  /* as input */
  PORTB = 0xFF; /* pullups */
  DDRD = 0x00;  /* as input */
  PORTD = 0xFF; /* pullups */
  DDRE = 0x00;  /* as input */
  PORTE = 0xFF; /* pullups */
  DDRF = 0x00;  /* as input */
  PORTF = 0xFF; /* pullups */
  DDRG = 0x00;  /* as input */
  PORTG = 0xFF; /* pullups */

  DIDR0 = 0xFF; /* disable all ADC inputs */

  /* we are using async Tx only, never receive
   * anything. Therefore we choose sleep as idle state
   */
  Radio.begin(17, STATE_SLEEP);
  
  adc_offset = measure_adc_offset();
  
  wdt_timersetup(WDTO_8S);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop()
{
  float tmcu, vmcu;
  
  tmcu = measure_tmcu();
  vmcu = measure_vmcu();
  
  Radio.print("ADDR=");
  Radio.print(Radio.nc.short_addr);
  
  Radio.print(", T=");
  Radio.print(tmcu);
  
  Radio.print(", V=");
  Radio.print(vmcu);

  Radio.print('\n');

  Radio.flush();
  while(Radio.tx_in_progress);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
}


