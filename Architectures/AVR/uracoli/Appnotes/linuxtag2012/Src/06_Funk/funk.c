/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

/* uracoli libraries */
#include <board.h>
#include <radio.h>

#include "funk.h"

/* === macros ============================================================== */
#define KEY_NONE (255)

/* === globals ============================================================= */
uint8_t LedState[9] = { RED,   GREEN, RED,
                        RED,   OFF,   GREEN,
                        GREEN, GREEN, RED};

static uint8_t RxFrame[TRX_FRAME_SIZE];
static volatile bool TxInProgress;
static volatile uint8_t RadioRxKey;
static volatile uint8_t RadioTxKey;

/* === functions =========================================================== */

/* === radio init =========== */
void xxo_radio_init(void)
{
  radio_init(RxFrame, sizeof(RxFrame));
  radio_set_param(RP_CHANNEL(CHANNEL));
  radio_set_param(RP_IDLESTATE(STATE_RX));
  radio_set_param(RP_TXPWR(-4));

  radio_set_state(STATE_RX);
  RadioRxKey = KEY_NONE;
  RadioTxKey = KEY_NONE;
}

/* === transmit functions === */
void xxo_send(uint8_t key)
{
    static uint8_t seqno = 0;
    xxo_frame_t txbuf;

    /* fill frame information */
    txbuf.fcf = FRAME_CTRL_FIELD;
    txbuf.seq = seqno++;
    txbuf.panid = PANID;
    txbuf.dst = 0xffff;
    txbuf.src = SHORTADDR;
    radio_set_state(STATE_TXAUTO);

    /* fill payload */
    txbuf.key = key;

    /* send frame */
    TxInProgress = true;

    radio_send_frame(sizeof(xxo_frame_t), (uint8_t*)&txbuf, 0);

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (TxInProgress)
    {
        sleep_mode();
    }
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    TxInProgress = false;
}

/* === receive functions === */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
    xxo_frame_t *pframe;
    pframe = (xxo_frame_t *)frm;
    RadioRxKey = pframe->key;
    return frm;
}

uint8_t xxo_radio_get_event(void)
{
uint8_t ret;

    cli();
    ret = RadioRxKey;
    RadioRxKey = KEY_NONE;
    sei();
    return ret;
}

ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;
    uint8_t key;

    key = update_pads(row);

    if (key != KEY_NONE)
    {
        LedState[key] = GREEN;
        if (RadioTxKey == KEY_NONE)
        {
            RadioTxKey = key;
        }
    }

    display_leds(row);

    row ++;
    if (row > 2)
    {
        row = 0;
    }
}

int main(void)
{
    uint8_t radio_key;
    io_init();
    xxo_radio_init();

    set_sleep_mode(SLEEP_MODE_IDLE);

    while(1)
    {

        radio_key = xxo_radio_get_event();
        if (radio_key!= KEY_NONE)
        {
            LedState[radio_key] = RED;
        }
        if (RadioTxKey != KEY_NONE)
        {
            xxo_send(RadioTxKey);
            RadioTxKey = KEY_NONE;
        }

        sleep_mode();
    }

    return 0;
}
