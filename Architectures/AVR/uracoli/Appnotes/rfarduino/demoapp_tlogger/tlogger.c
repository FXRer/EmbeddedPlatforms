/* Copyright (c) 2011 - 2012 Daniel Thiele, Axel Wachtler
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
 * @file Demo Application based on P2P protocol
 *       Measure temperature via RFA128 internal sensor and
 *       format to line and send via Wuart frames
 *
 * @brief
 * @ingroup
 */

#define ISRAVEN ( defined(ravrf230a) || defined(ravrf230b) )


/* === includes ============================================================ */

/* avr-libc inclusions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>

/* uracoli inclusions */
#include "board.h"
#include "ioutil.h"
#include "transceiver.h"
#include "radio.h"
#include "p2p.h"

#if ISRAVEN
#include "hif.h"
#endif

/* === macros ============================================================== */

#define APP_VERSION (0x01)
#define APP_NAME "rfa_temperature"

#define EOL "\n"


/* === types =============================================================== */

typedef enum
{
	APPSTATE_IDLE, APPSTATE_MEAS, APPSTATE_TRX,
} appstate_t;

/* === globals ============================================================= */

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

/* IEEE802.15.4 parameters for communication */
const node_config_t PROGMEM nc_flash =
{
	.short_addr = 0xFECA,
	.pan_id = 0x3412,
	.channel = 17
};

static node_config_t NodeConfig;

static uint8_t rxbuf[MAX_FRAME_SIZE];
static uint8_t txbuf[MAX_FRAME_SIZE];

/* prototype to be used in wuart_stream setup */
int wuart_putc(int c, FILE *stream);

static FILE wuart_stream = FDEV_SETUP_STREAM(wuart_putc, NULL, _FDEV_SETUP_WRITE);
#if ISRAVEN
static FILE hif_stream = FDEV_SETUP_STREAM(hif_putc, hif_getc, _FDEV_SETUP_RW);	
#endif

static volatile uint8_t adcfinished = 0;
static int8_t adc_offset = 0;

static volatile appstate_t appstate;
static volatile appstate_t nextstate;

/* === prototypes ========================================================== */

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
	nextstate = APPSTATE_MEAS;
}

/*
 * \brief Radio receive
 *
 *  Callback from module "radio"
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
		int8_t ed, uint8_t crc_fail)
{
	static uint8_t lastseq = 255;

	p2p_hdr_t *hdr = (p2p_hdr_t*) frm;
	switch (hdr->cmd)
	{

	case P2P_PING_REQ:
		break;
	case P2P_PING_CNF:
		break;
	case P2P_JUMP_BOOTL:
		jump_to_bootloader();
		break;

	default:
		break;
	}

	lastseq = hdr->seq;

	return frm;
}

/*
 * \brief Radio error
 *
 *  Callback from module "radio"
 */
void usr_radio_error(radio_error_t err)
{
	/* TODO */
}

/*
 * \brief Radio Tx done
 *
 *  Callback from module "radio"
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
	switch (status)
	{
	case TX_OK:
		break;
	default:
		break;
	} /* switch(status) */

	nextstate = APPSTATE_IDLE;
}

/*
 * \brief Initialize Application
 *
 */
void app_init(void)
{
	char cfg_location = '?';

	/* === read node configuration data ===================================== */
	/* 1st trial: read from EEPROM */
	if (get_node_config_eeprom(&NodeConfig, 0) == 0)
	{
		/* using EEPROM config */;
		cfg_location = 'E';
	}
	/* 2nd trial: read from FLASHEND */
	else if (get_node_config(&NodeConfig) == 0)
	{
		/* using FLASHEND config */;
		cfg_location = 'F';
	}
	/* 3rd trial: read default values compiled into the application */
	else
	{
		/* using application default config */;
		memcpy_P(&NodeConfig, (PGM_VOID_P) & nc_flash, sizeof(node_config_t));
		cfg_location = 'D';
	}

	/* === setup global variables =========================================== */

	/* === setup radio ====================================================== */
	radio_init(rxbuf, sizeof(rxbuf));
	radio_set_param(RP_CHANNEL(NodeConfig.channel));
	radio_set_param(RP_PANID(NodeConfig.pan_id));
	radio_set_param(RP_SHORTADDR(NodeConfig.short_addr));
	radio_set_param(RP_IDLESTATE(STATE_SLEEP));

	//p2p_init(NodeConfig.short_addr, NodeConfig.pan_id);

#if defined(SR_RND_VALUE)
	/* TODO: if the random value is not available in HW, e.g. AT86RF230A/B,
	 * we need to find another method of seeding CSMA  */
	trx_reg_write(RG_CSMA_SEED_0, trx_bit_read(SR_RND_VALUE) | (trx_bit_read(SR_RND_VALUE) << 4));
	trx_reg_write(RG_CSMA_SEED_1, trx_bit_read(SR_RND_VALUE) | (trx_bit_read(SR_RND_VALUE) << 4));
#endif

	/* Welcome Blink */
	LED_SET(0);
	LED_SET(1);
	_delay_ms(20);
	LED_CLR(0);
	LED_CLR(1);
}

#if ISRAVEN

float measure_tmcu(void)
{
	char ln[80] = "########";
	char *p = ln;
	char c;

	while(EOF != fgetc(&hif_stream)); /* flush */

	fprintf(&hif_stream, "temp\n"); /* command to request temperature */
	
	/* wait for line */
	while( (c=fgetc(&hif_stream)) == EOF )
	{
		if(c != '\n'){
			*p++ = c;
		} else {
			*p = 0x00; /* Terminator */
			break;
		}
	}

	/* parse out temperature value */
	
	return 12.34;
}

uint16_t measure_vmcu(void)
{
	return 12.34;
}

#else /* #if ISRAVEN */

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
 * \brief Supply voltage measurement
 * Method: set 1.1V reference as input and AVCC as reference
 * 	this returns a negative result: AVCC = 1.1V - result
 *
 * \return The MCU internal voltage in [mV]
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
uint16_t measure_vmcu(void)
{
	uint16_t v; /* voltage in [mV] */
	uint8_t vth;

	for(vth=0;vth<32;vth++){
		trx_reg_write(RG_BATMON, vth & 0x1F);

		if(0 == trx_bit_read(SR_BATMON_OK)){
			break;
		}
	}

	if(vth & 0x10){
		v = 2550 + 75*(vth&0x0F); /* high range */
	}else{
		v = 1700 + 50*(vth&0x0F); /* low range */
	}

	return v;
}

#endif /* else if ISRAVEN */

/*
 * \brief Stdio compatible function to put character to send via WUART
 *
 * Can be used to setup stream for printf() functions
 */
int wuart_putc(int c, FILE *stream)
{
	static uint8_t idx = sizeof(p2p_wuart_data_t);

	txbuf[idx++] = c;

	if( ('\n' == c) || ('\r' == c) || (idx >= (MAX_FRAME_SIZE - 2)) )
        {
		p2p_hdr_t *hdr = (p2p_hdr_t*) txbuf;
		hdr->fcf = 0x8841;
		hdr->seq += 1;
		hdr->pan = NodeConfig.pan_id;
		hdr->dst = 0xffff;
		hdr->src = NodeConfig.short_addr;
		hdr->cmd = P2P_WUART_DATA;
		radio_set_state(STATE_TX);
		radio_set_state(STATE_TXAUTO);
		radio_send_frame(idx + 2, txbuf, 1); /* +2: add CRC bytes (FCF) */
		idx = sizeof(p2p_wuart_data_t);
	}

	return c;
}

/*
 * \brief Save MCUSR to variable and disable watchdog
 * This must be done very early after reset, placing to .init3 section is done
 * in the forward declaration above
 *
 */
void __attribute__((naked)) __attribute__((section(".init3"))) get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

int main()
{
	float t=0;
	float v=0;

#if defined(radiofaro)
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
#endif



	LED_INIT();

	app_init();

	wdt_reset();
/* DEBUG */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	for(;;){
		sleep_mode();
	}
/*********/
	
	wdt_timersetup(WDTO_4S);

	stdout = &wuart_stream;

	sei();

#if ISRAVEN
	hif_init(HIF_DEFAULT_BAUDRATE);
	printf(EOL""EOL"ADDR=0x%04X Raven Demo application: Temperature logger"EOL, NodeConfig.short_addr);
#else
	adc_offset = measure_adc_offset();
	printf(EOL""EOL"ADDR=0x%04X RadioFaro Demo application: Temperature logger"EOL, NodeConfig.short_addr);
#endif



	set_sleep_mode(SLEEP_MODE_IDLE);
	appstate = nextstate = APPSTATE_TRX;

	for (;;)
	{

		if (appstate != nextstate)
		{
			switch (nextstate)
			{
			case APPSTATE_IDLE:
				set_sleep_mode(SLEEP_MODE_PWR_DOWN);
				break;
			case APPSTATE_MEAS:
				LED_SET(0);
				DELAY_MS(5);
				LED_CLR(0);

				t = measure_tmcu();
				v = measure_vmcu();

				nextstate = APPSTATE_TRX;

				/* Fake,
				 *
				 * fall through
				 */
			case APPSTATE_TRX:
				set_sleep_mode(SLEEP_MODE_IDLE);

				printf("ADDR=0x%04X, T=%.1f, V=%.2f"EOL,
					NodeConfig.short_addr, (double)t, (double)v/1000);

				break;
			}
			appstate = nextstate;
		}

		sleep_mode();
	} /* for(;;); */

	/* never reaches this point */

}
/* EOF */
