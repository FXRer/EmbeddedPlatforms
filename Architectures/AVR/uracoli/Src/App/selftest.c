/* Copyright (c) 2015 Axel Wachtler, Daniel Thiele
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
 * @file This application tests basic radio connections. It outputs test progress
         and results to serial port.
 * @brief ....
 * @_addtogroup grpApp...
 */

/* avr-libc inclusions */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>


/* uracoli inclusions */
#include <board.h>
#include <transceiver.h>
#include <hif_uart.h>
#include <hif.h>

/* === macros ============================================================== */

#define EOL "\n\r"

/* === types =============================================================== */

typedef enum{
	PASS=0,
	FAIL,
	SKIP,
	MAXRESULT
}testresult_t;
typedef uint8_t testverbose_t;


#define TESTFUNC(name) testresult_t test_##name(testverbose_t verbose)
#define TESTEXEC(name) printf(#name " "); printf(" %s"EOL, result_string[test_##name(0)])

/* === globals ============================================================= */

static volatile uint8_t irq_cause = 0;
static const char *result_string[MAXRESULT] = {"PASS", "FAIL", "SKIP"};

TESTFUNC(regrd){
	uint8_t addr = RG_PART_NUM;

	uint8_t res=trx_reg_read(addr);

	printf("0x%02X=0x%02X", addr, res);
	if(res == RADIO_PART_NUM){
		return PASS;
	}else{
		return FAIL;
	}
}

TESTFUNC(regwr){
	uint8_t addr = RG_IEEE_ADDR_0;
	uint8_t dummy = 0xE6;

	trx_reg_write(addr, dummy);

	if(trx_reg_read(addr) != dummy){
		printf("cannot write to 0x%02X", addr);
		return FAIL;
	}else{
		printf("0x%02X=0x%02X", addr, dummy);
		return PASS;
	}
}

TESTFUNC(rstn){
	uint8_t addr = RG_IEEE_ADDR_0;
	uint8_t rstval = 0x00; /* reset value of register */
	uint8_t dummy = 0xE6;
	uint8_t res;

	trx_reg_write(addr, dummy);
	res=trx_reg_read(addr);

	if(res != dummy){
		printf("cannot write to 0x%02X", addr);
		return FAIL;
	}else{
		TRX_RESET_LOW();
		DELAY_US(TRX_RESET_TIME_US);
		TRX_RESET_HIGH();

		res=trx_reg_read(addr);
		if(res != rstval){
			printf("0x%02X=0x%02X, expected 0x%02X", addr, res, rstval);
			return FAIL;
		}else{
			printf("0x%02X=0x%02X", addr, rstval);
			return PASS;
		}
	}
}

static void irq_handler(uint8_t cause)
{
	irq_cause = cause;
}

static inline uint8_t wait_irq(uint8_t timeout100us)
{
	do {
		DELAY_US(100);
	}while( (0 == irq_cause) && (--timeout100us) );
	return timeout100us;
}

TESTFUNC(irq)
{
	DI_TRX_IRQ();
	trx_reg_write(RG_IRQ_MASK, 0x00);

	/* IRQ handler must be set to irq_handler beforehand */

	trx_bit_write(SR_TRX_CMD, CMD_TRX_OFF);
	do{
		DELAY_US(50);
	}while(trx_bit_read(SR_TRX_STATUS) != TRX_OFF);

#if defined(TRX_IF_RFA1)
	trx_reg_write(RG_IRQ_STATUS, 0xFF);
#else
	trx_reg_read(RG_IRQ_STATUS);
#endif
	if(trx_reg_read(RG_IRQ_STATUS) != 0x00){
		printf("cannot clear IRQ_STATUS");
		return FAIL;
	}

	trx_reg_write(RG_IRQ_MASK, TRX_IRQ_PLL_LOCK);
	irq_cause = 0;
	EI_TRX_IRQ();

	trx_bit_write(SR_TRX_CMD, CMD_PLL_ON);

	if(0==wait_irq(3)){ /* lock within max 300 us */
		printf("timeout 300us");
		return FAIL;
	}else{
		if(irq_cause != TRX_IRQ_PLL_LOCK){
			printf("irq_cause=0x%02X", irq_cause);
			return FAIL;
		}else{
			printf("IRQ_STATUS=PLL_LOCK");
			trx_bit_write(SR_TRX_CMD, CMD_TRX_OFF);
			DI_TRX_IRQ();
			return PASS;
		}
	}
}

TESTFUNC(slptr)
{
#if defined(TRX_IF_RFA1)
	printf("no SLP_TR line");
	return SKIP;
#endif

#if defined(TRX_IRQ_CCA_ED) // AWAKE interrupt
	DI_TRX_IRQ();
	trx_reg_write(RG_IRQ_MASK, 0x00);

	/* IRQ handler must be set to irq_handler beforehand */

	trx_bit_write(SR_TRX_CMD, CMD_TRX_OFF);
	do{
		DELAY_US(50);
	}while(trx_bit_read(SR_TRX_STATUS) != TRX_OFF);

	trx_reg_read(RG_IRQ_STATUS);
	if(trx_reg_read(RG_IRQ_STATUS) != 0x00){
		printf("cannot clear IRQ_STATUS");
		return FAIL;
	}

	trx_reg_write(RG_IRQ_MASK, TRX_IRQ_CCA_ED);
	irq_cause = 0;
	EI_TRX_IRQ();

	TRX_SLPTR_HIGH();
	DELAY_US(20);
	TRX_SLPTR_LOW();

	if(0==wait_irq(3)){ /* lock within max 300 us */
		printf("timeout 300us");
		return FAIL;
	}else{
		if(irq_cause != TRX_IRQ_CCA_ED){
			printf("irq_cause=0x%02X", irq_cause);
			return FAIL;
		}else{
			printf("IRQ_STATUS=CCA_ED");
			DI_TRX_IRQ();
			return PASS;
		}
	}

#else
	printf("cannot test, no AWAKE Irq");
	return SKIP;
#endif
}


int main()
{
	static FILE uartio = FDEV_SETUP_STREAM(hif_putc, hif_getc, _FDEV_SETUP_RW);

	mcu_init();
	trx_io_init(DEFAULT_SPI_RATE);

	DI_TRX_IRQ();
	trx_set_irq_handler(irq_handler);

	TRX_RESET_LOW();
	TRX_SLPTR_LOW();
	DELAY_US(TRX_RESET_TIME_US);
	TRX_RESET_HIGH();

	/* disable IRQ and clear any pending IRQs */
	trx_reg_write(RG_IRQ_MASK, 0);
	trx_reg_read(RG_IRQ_STATUS);

	hif_init(HIF_DEFAULT_BAUDRATE);
	stdin = stdout = stderr = &uartio;
	sei();

	printf("-------------------"EOL);
	printf("Selftest <BUILD %s %s>"EOL, __DATE__, __TIME__);
	printf("Board %s, Radio %s"EOL, BOARD_NAME, RADIO_NAME);
	printf("-------------------"EOL);

	TESTEXEC(regrd);
	TESTEXEC(regwr);
	TESTEXEC(rstn);
	TESTEXEC(irq);
	TESTEXEC(slptr);

	for (;;)
	{
		MCU_SLEEP_IDLE();
	}
}

/* EOF */
