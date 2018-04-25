/* avr-libc includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

/* uracoli includes */
#include <board.h>
#include <transceiver.h>
#include <ioutil.h>

/* application related includes */
#include "acc_mma7455.h"
#include "lgee.h"

#ifndef WORKCHANNEL
#define WORKCHANNEL	(17)
#endif

#define WDT_ENABLE_SYSRESET()	do{}while(0)
#define WDT_DISABLE_SYSRESET()	do{}while(0)

static inline void wdt_setup(timeout)
{
	WDTCSR &= ~(0x0F);
	WDTCSR |= timeout;
	WDTCSR = (1 << WDCE) | (1 << WDE); /* Enable configuration change */
	WDTCSR = (1 << WDIF) | (1 << WDIE) | /* Enable Watchdog Interrupt Mode */
			(1 << WDCE) | (1 << WDE); /* Disable Watchdog System Reset Mode if unintentionally enabled */
}

ISR(WDT_vect) {
	wdt_setup(WDTO_1S);
}

static uint8_t txbuf[MAX_FRAME_SIZE];

int main(void)
{

	lgee_dataframe_t *frm = (lgee_dataframe_t*) txbuf;

	MCUSR &= ~_BV(WDRF); /* Clear WDRF if it has been unintentionally set */
	wdt_setup(WDTO_1S);

	/* some power reduction */
	PRR |= _BV(PRTWI) | _BV(PRTIM2) | _BV(PRTIM0) | _BV(PRTIM1) | _BV(PRUSART0)
			| _BV(PRADC);

	/* prepare frame */
	frm->FCF = 0x1000;
	frm->seqnb = 0;
	frm->temperature = 0;

	/* Init MCU peripherals */
	LED_INIT();
	trx_io_init( SPI_RATE_1_2);
	LED_SET_VALUE( LED_MAX_VALUE);
	LED_SET_VALUE(0);

	/* Initialize the transceiver */
	TRX_RESET_LOW();
	TRX_SLPTR_LOW();
	DELAY_US( TRX_RESET_TIME_US);
	TRX_RESET_HIGH();
	trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);

	/* Setup transceiver */
	trx_bit_write(SR_CHANNEL, WORKCHANNEL);
	trx_bit_write(SR_TX_AUTO_CRC_ON, 1);

	/* IRQs required to wake up MCU: PLL_LOCK, AWAKE_END (=CCA_ED), TRX_END */
	trx_reg_write(RG_IRQ_MASK, TRX_IRQ_TRX_END | TRX_IRQ_PLL_LOCK
			| TRX_IRQ_CCA_ED); /* shared with AWAKE_END */

	DI_TRX_IRQ();

	/* set transceiver to sleep */
	TRX_SLPTR_HIGH();

	/* Initialize the accelerometer MMA7455 */
	acc_init();
	acc_setmode( ACC_MODE_STANDBY);

	TRX_IRQ_INIT();
	ACC_IRQ_INIT();

	/* clear all pending flags
	 * TODO: add macro in board.h
	 */
	PCIFR = 0xFF;

	DI_ACC_IRQ();

	acc_regwr(RG_MCTL, 0x04); /* set to 2g-mode */
	acc_regrd( RG_XOUT8); /* dummy read to clear IRQ */

	set_sleep_mode( SLEEP_MODE_PWR_DOWN);

	sei();

	for (;;) {

		acc_setmode( ACC_MODE_MEASURE);
		acc_regrd(RG_XOUT8); /* read possible pending interrupts */
		EI_ACC_IRQ(); /* to wake up by ACC IRQ */

		/* failure handling: if the MCU is not waken up by ACC IRQ,
		 * it will be reset (after awaken from watchdog)
		 */
		WDT_ENABLE_SYSRESET();
		sleep_mode();

		/* sleeping, wake up by INT1/DRDY interrupt */

		WDT_DISABLE_SYSRESET();
		DI_ACC_IRQ(); /* avoid interrupting on falling edge */

		TRX_SLPTR_LOW(); /* wake up transceiver */

		frm->x = acc_regrd(RG_XOUT8); /* ACC IRQ should go low after this instruction */
		frm->y = acc_regrd(RG_YOUT8);
		frm->z = acc_regrd(RG_ZOUT8);

		acc_setmode(ACC_MODE_STANDBY);

		EI_TRX_IRQ(); /* enable transceiver interrupt */

		/* failure handling: if the MCU is not waken up by ACC IRQ,
		 * it will be reset (after awaken from watchdog)
		 */
		WDT_ENABLE_SYSRESET();
		sleep_mode();

		/* sleeping, wake up by AWAKE_END interrupt (TRX) */

		WDT_DISABLE_SYSRESET();
		trx_reg_read( RG_IRQ_STATUS);
		trx_reg_write(RG_TRX_STATE, CMD_PLL_ON);

		/* failure handling: if the MCU is not waken up by ACC IRQ,
		 * it will be reset (after awaken from watchdog)
		 */
		WDT_ENABLE_SYSRESET();
		sleep_mode();

		/* sleeping, wake up by PLL_LOCK interrupt */
		WDT_DISABLE_SYSRESET();

		trx_reg_read(RG_IRQ_STATUS);

		LED_SET(1);
		TRX_SLPTR_HIGH();
		TRX_SLPTR_LOW();

		frm->seqnb++;
		trx_frame_write(sizeof(lgee_dataframe_t), (uint8_t*) frm);

		/* failure handling: if the MCU is not waken up by ACC IRQ,
		 * it will be reset (after awaken from watchdog)
		 */
		WDT_ENABLE_SYSRESET();
		sleep_mode();

		/* sleeping, wake up by TRX_END interrupt */

		WDT_DISABLE_SYSRESET();
		LED_CLR(1);

		trx_reg_read(RG_IRQ_STATUS);
		trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);

		TRX_SLPTR_HIGH(); /* set transceiver to sleep */
		DI_TRX_IRQ();

		/* wait for INT1 low to avoid interrupting on falling edge
		 * should be low here already
		 */
		while (ACC_IRQ_PIN & _BV(ACC_IRQ_bp));

		/* regular case: watchdog is used as timer to continue application loop infinitely, do not reset */
		WDT_DISABLE_SYSRESET();
		sleep_mode();

		/* sleeping, wake up by watchdog */

	}

	/* this point is never reached */
}

/* EOF */
