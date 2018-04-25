/* avr-libc includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* uracoli includes */
#include <board.h>
#include <transceiver.h>
#include <ioutil.h>

/* application related includes */
#include "acc_mma7455.h"


#define PRINT(fmt, ...)	printf_P(PSTR(fmt), ##__VA_ARGS__)
#define ERROR(fmt, ...)	printf_P(PSTR(fmt), ##__VA_ARGS__)
#define DEBUG(fmt, ...)	printf_P(PSTR(fmt), ##__VA_ARGS__)


#define TRX_IRQ_STATE() (TRX_IRQ_PIN & _BV(TRX_IRQ_bp))

#define TRX_SLPTR_HI() do{ PORT_TRX_SLPTR |= MASK_TRX_SLPTR; }while(0)
#define TRX_SLPTR_LO() do{ PORT_TRX_SLPTR &= ~MASK_TRX_SLPTR; }while(0)

#define TRX_RSTN_HI() do{ PORT_TRX_RESET |= MASK_TRX_RESET; }while(0)
#define TRX_RSTN_LO() do{ PORT_TRX_RESET &= ~MASK_TRX_RESET; }while(0)


/** Test SPI interface MCU-Transceiver
 *
 * Write value to some register and read back
 *
 */
uint8_t tst_trx_spi()
{
	uint8_t trx_reg = 0x20;		/* SHORT ADDR [0] register, free for R/W */
	uint8_t tmp;

	trx_reg_write(trx_reg, 0xA5);
	if( (tmp = trx_reg_read(trx_reg)) != 0xA5){
		ERROR("TRX register access failure: read 0x%02X, expected 0x%02X", tmp, 0xA5);
		return 0;
	}

	return 1;
}


/** Test SPI interface MCU-Acceleration Sensor
 *
 * Write value to some register and read back
 *
 */
uint8_t tst_acc_spi()
{
	uint8_t acc_reg = 0x1E;		/* Arbirtary register: time window for 2nd pulse value */
	uint8_t tmp, old;

	old = acc_regrd(acc_reg);
	acc_regwr(acc_reg, 0xA5);
	if( (tmp = acc_regrd(acc_reg)) != 0xA5){
		ERROR("ACC register access failure: read 0x%02X, expected 0x%02X", tmp, 0xA5);
		return 0;
	}else{
		acc_regwr(acc_reg, old);	/* write back original value */
	}
	return 1;
}

uint8_t tst_trxirq()
{
	volatile uint32_t timeout;
	uint8_t irq;

	trx_reg_write(0x02, 0x03);	/* FORCE TRX_OFF */
	trx_reg_write(0x0E, 0xFF);	/* allow all IRQs */

	trx_reg_read(0x0F);	/* clear all IRQ flags */
	if( TRX_IRQ_STATE() != 0 ){
		ERROR("IRQ pin not low after TRX status read");
		return 0;
	}
	trx_reg_write(2, 9);	/* PLL_ON */

	timeout = 0x00000FFF;
	while( (TRX_IRQ_STATE() == 0) && --timeout );	/* wait for IRQ */

	if( timeout == 0){
		ERROR("TRX IRQ did not occur, timeout");
		return 0;
	}

	irq = trx_reg_read(0x0F);	/* clear all IRQ flags */
	if( TRX_IRQ_STATE() != 0 ){
		ERROR("IRQ pin not low after TRX status read");
		return 0;
	}else{
		/* got IRQ, successful */
	}

	trx_reg_write(2, 3);	/* FORCE TRX_OFF */

	return 1;
}


uint8_t tst_trxSLPTR()
{
	volatile uint32_t i;

	trx_reg_write(2, 3);	/* FORCE TRX_OFF */
	trx_reg_write(0x0E, 0xFF);	/* allow all IRQs */

	trx_reg_write(2, 9);	/* PLL_ON */

	for(i=0;i<0xFFF;i++);	/* delay for PLL_LOCK */
	if( (trx_reg_read(0x01)&0x1F) != 0x09){		/* PLL_ON */
		ERROR("could not set PLL_ON");
		trx_reg_write(2, 3);	/* FORCE TRX_OFF */
		return 0;
	}else{
		TRX_SLPTR_HI();
		asm volatile ("nop");
		TRX_SLPTR_LO();
		if( (trx_reg_read(0x01)&0x1F) != 0x02){	/* TX_ACTIVE */
			trx_reg_write(2, 3);	/* FORCE TRX_OFF */
			ERROR("not in TX_ACTIVE after SLP_TR pulse");
			return 0;
		}else{
			/* do nothing */
		}
	}
	trx_reg_write(2, 3);	/* FORCE TRX_OFF */
	return 1;
}

uint8_t tst_trxRSTN()
{
	uint8_t trx_reg = 0x20;		/* SHORT ADDR [0] register, free for R/W */
	uint8_t trx_reg_rstval = 0xFF;	/* the reset value of register */
	uint8_t tmp;
	volatile uint32_t i;

	trx_reg_write(trx_reg, 0xA5);
	if( (tmp = trx_reg_read(trx_reg)) != 0xA5){
		ERROR("TRX register access failure: read 0x%02X, expected 0x%02X", tmp, 0xA5);
		return 0;
	}

	TRX_RSTN_LO();
	/* TODO: delay_us(10) */
	for(i=0;i<0xF;i++);		/* some delay */
	TRX_RSTN_HI();

	if( (tmp = trx_reg_read(trx_reg)) != trx_reg_rstval){
		ERROR("TRX register not reset value: read 0x%02X, expected 0x%02X", tmp, trx_reg_rstval);
		return 0;
	}

	return 1;
}

uint8_t tst_accINT1()
{
	volatile uint16_t timeout;

	if( ACC_IRQ_STATE() ){		/* INT1 line is high, try to clear */

		acc_regwr(0x17, 0x03);	/* clear IRQ */

		if( ACC_IRQ_STATE() ){
			ERROR("INT1 not cleared");
			return 0;
		}
	}

	acc_regwr(0x16, 0x04);		/* Standby Mode */

	acc_regwr(0x16, 0x05);		/* Start Measurement */

	timeout = 0x0F;
	while( (!ACC_IRQ_STATE()) && --timeout );

	if(!timeout){
		ERROR("INT1 interrupt timeout");
		return 0;
	}

	acc_regwr(0x17, 0x03);	/* clear IRQ */

	if( ACC_IRQ_STATE() ){
		ERROR("INT1 not cleared");
		return 0;
	}
	acc_regwr(0x16, 0x04);		/* stop measurement mode */

	return 1;
}


uint8_t tst_accINT2()
{
	return 1;
}


struct{
	const char *name;
	uint8_t (*func)(void);
}testcases[] = {
					{ "SPI TRX",    	tst_trx_spi},
					{ "SPI ACC",    	tst_acc_spi},
					{ "TRX IRQ",   		tst_trxirq},
					{ "TRX SLPTR", 		tst_trxSLPTR  },
					{ "TRX RSTN",  		tst_trxRSTN},
					{ "ACC INT1",  		tst_accINT1},
					{ "ACC INT2",  		tst_accINT2},
				};


/** Run all test cases
 *
 */
uint8_t boardtest()
{
	uint8_t nberrors;

	PRINT("Starting board tests\n\r");

	nberrors = 0;
	for(uint8_t i=0;i<sizeof(testcases)/sizeof(testcases[0]);i++){
		PRINT("TEST[%02d]: %s...", i, testcases[i].name);
		if( testcases[i].func() == 0 ){
			PRINT("..FAIL\n\r");
			nberrors ++;
		}else{
			PRINT("..PASS\n\r");
		}
	}

	PRINT("finished. %d Errors\n\r", nberrors);
	
	return(nberrors == 0);
}


/* EOF */
