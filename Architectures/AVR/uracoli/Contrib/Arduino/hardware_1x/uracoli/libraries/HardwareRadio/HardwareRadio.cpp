#include "uracoli/const.h"
#include <avr/interrupt.h>
#define HARDWARERADIO_CPP (1)
#include "HardwareRadio.h"

#define BUFFER_FREE (0)
#define BUFFER_USED (0x80)

#define BUFFER_POOL_SIZE (8)

radio_buffer_t buffer_pool[BUFFER_POOL_SIZE];
radio_buffer_t *TxQueue;
radio_buffer_t *RxQueue;


extern "C"
{
    void push_begin(radio_buffer_t **qhead, radio_buffer_t *buf);
    void push_end(radio_buffer_t **qhead, radio_buffer_t *buf);
    radio_buffer_t * pop_begin(radio_buffer_t **qhead);
    radio_buffer_t * pop_end(radio_buffer_t **qhead);
}



HardwareRadio::HardwareRadio(void)
{
    /* empty */
}

void HardwareRadio::begin(void)
{
    this->begin(PHY_DEFAULT_CHANNEL, STATE_RXAUTO,
                DEFAULT_PAN_ID, DEFAULT_SHORT_ADDRESS, DEFAULT_SHORT_ADDRESS);
}

void HardwareRadio::begin(uint8_t channel, uint8_t idlestate)
{
    this->begin(channel, idlestate,
                DEFAULT_PAN_ID, DEFAULT_SHORT_ADDRESS, DEFAULT_SHORT_ADDRESS);
}

void HardwareRadio::begin(uint8_t channel, uint8_t idlestate,
                          uint16_t pan, uint16_t dst, uint16_t src)
{

uint8_t i;

    for(i=0; i<BUFFER_POOL_SIZE; i++)
    {
        /* Mark buffer as empty */
        buffer_pool[i].len = 0;
        buffer_pool[i].next = NULL;
    }

    TxQueue = NULL;
    this->txbuf = NULL;
    this->pwrbuf = NULL;

    this->txseq = 0;
    tx_in_progress  = 0;

    RxQueue = NULL;
    prdbuf = NULL;
    rxbuf = alloc_buffer();
    radio_init(rxbuf->frm, PHY_MAX_FRAME_SIZE);

    channel = (channel>=PHY_MIN_CHANNEL && channel<=PHY_MAX_CHANNEL)?channel:PHY_DEFAULT_CHANNEL;
    radio_set_param(RP_CHANNEL(channel));
    radio_set_param(RP_IDLESTATE((radio_state_t)idlestate));

    this->panid = pan;
    this->dstaddr = dst;
    this->srcaddr = src;

    radio_set_param(RP_PANID(this->panid));
    radio_set_param(RP_SHORTADDR(this->srcaddr));

    radio_set_state((radio_state_t)idlestate);
}


radio_buffer_t * HardwareRadio::alloc_buffer(void)
{
    radio_buffer_t *pret = NULL;
    uint8_t i;

    for(i=0; i< BUFFER_POOL_SIZE; i++)
    {
        if(buffer_pool[i].len == 0)
        {
            buffer_pool[i].len = BUFFER_USED;
            pret = &buffer_pool[i];
            break;
        }
    }
    return pret;
}

void HardwareRadio::free_buffer(radio_buffer_t * pbuf)
{
    if (pbuf != NULL)
    {
        pbuf->len = 0;
        pbuf->idx = 0;
        pbuf->next = NULL;
    }
}


size_t HardwareRadio::write(uint8_t byte)
{

    if (this->pwrbuf == NULL)
    {
        cli();
        this->pwrbuf = alloc_buffer();
        txbuffer_init(this->pwrbuf);
        sei();
    }

    this->pwrbuf->frm[this->pwrbuf->idx] = byte;

    this->pwrbuf->idx++;
    if (this->pwrbuf->idx >= (PHY_MAX_FRAME_SIZE - sizeof(p2p_hdr_t) - 2))
    {
        this->flush();
    }

    return 1;
}

uint8_t HardwareRadio::sendto(uint16_t dst, uint8_t *pbuf, uint8_t size)
{
    radio_buffer_t *rbuf;
    rbuf = alloc_buffer();
    txbuffer_init(rbuf, dst);

    memcpy((&rbuf->frm[rbuf->idx]), pbuf, size);
    rbuf->idx += size;
    push_end(&TxQueue, rbuf);
    this->flush();

    return size;
}


void HardwareRadio::flush(void)
{

    if (this->pwrbuf != NULL)
    {
        if (this->pwrbuf->idx > sizeof(p2p_hdr_t) + 1) /* anything in? */
        {
            cli();
            this->pwrbuf->len = this->pwrbuf->idx + 2;
            push_end(&TxQueue, this->pwrbuf);
            this->pwrbuf = NULL;
            sei();
        }
    }

    /* blocking wait until TxQueue contains buffers */
    while (TxQueue != NULL)
    {

        while( this->tx_in_progress != 0)
        {
            /* blocking wait until TX is done, add sleep */
        }
        /* send next element in txqueue */
        this->tx_in_progress = 1;
        txbuf = pop_begin(&TxQueue);
        radio_set_state((radio_state_t) STATE_TXAUTO);
        radio_send_frame(txbuf->idx+2, txbuf->frm, 0);
    }
}

void HardwareRadio::txbuffer_init(radio_buffer_t *pbuf)
{
    this->txbuffer_init(pbuf, this->dstaddr);
}

void HardwareRadio::txbuffer_init(radio_buffer_t *pbuf, uint16_t dst)
{
    p2p_hdr_t *hdr = (p2p_hdr_t*)&pbuf->frm;

    hdr->fcf = 0x8841; /* Short addressing, PAN_ID compression, frame type DATA */
    hdr->seq = txseq++;
    hdr->pan = this->panid; /* Broadcast */
    hdr->dst = dst; /* Broadcast */
    hdr->src = this->srcaddr; /* Broadcast */
    hdr->cmd = P2P_WUART_DATA;
    pbuf->idx = sizeof(p2p_hdr_t) + 1;
}

int HardwareRadio::available(void)
{
    int ret = 0;

    /* try to get the next buffer from rx queue. */
    if (this->prdbuf == NULL)
    {
        cli();
        this->prdbuf = pop_end(&RxQueue);
        sei();
    }

    if (this->prdbuf != NULL)
    {
        ret = (this->prdbuf->len - this->prdbuf->idx - 2);
        if (ret <= 0)
        {
            cli();
            free_buffer(this->prdbuf);
            this->prdbuf = NULL;
            sei();
            ret = 0;
        }
    }
    return ret;
}

int HardwareRadio::peek(void)
{
int ret = -1; /*EOF*/

    /* try to get the next buffer from rx queue. */
    if (this->prdbuf == NULL)
    {
        cli();
        this->prdbuf = pop_end(&RxQueue);
        sei();
    }

    /* is a buffer available ? */
    if (this->prdbuf != NULL)
    {
        if ((this->prdbuf->idx+2) < this->prdbuf->len)
        {
            ret = this->prdbuf->frm[this->prdbuf->idx];
        }
        else
        {
            free_buffer(this->prdbuf);
            this->prdbuf = NULL;
        }
    }
    return ret;
}

int HardwareRadio::read(void)
{
int ret = EOF;

    /* try to get the next buffer from rx queue. */
    if (this->prdbuf == NULL)
    {
        cli();
        this->prdbuf = pop_end(&RxQueue);
        sei();
    }

    /* is a buffer available ? */
    if (this->prdbuf != NULL)
    {
        if ((this->prdbuf->idx) < this->prdbuf->len)
        {
            ret = this->prdbuf->frm[this->prdbuf->idx];
            this->prdbuf->idx ++;
        }
        else
        {
            free_buffer(this->prdbuf);
            this->prdbuf = NULL;
        }
    }
    return ret;
}


HardwareRadio Radio;

extern "C" uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
    radio_buffer_t * pbuf;
    if ((len > 5) && ((*frm & 7) == 1))
    {
        pbuf = Radio.alloc_buffer();
        if (pbuf != NULL)
        {
            /* another buffer is available, update the current one and
             * add it to the queue. */
            Radio.rxbuf->len = len;
            Radio.rxbuf->idx=sizeof(p2p_hdr_t)+1;
            push_begin(&RxQueue, Radio.rxbuf);
            Radio.rxbuf = pbuf;
            frm = pbuf->frm;
        }
        else
        {
            /* no more buffers, we drop this frame and reuse
             * the last buffer. */
            frm = Radio.rxbuf->frm;
        }
    }
    return frm;
}

/* This function is called from the Radio IRQ Context */
extern "C" void usr_radio_tx_done(radio_tx_done_t status)
{
radio_buffer_t * pbuf;
    /* remove buffer from list and free it. */
    /* tx_status could handled here, at least with some counters. */
    Radio.tx_in_progress = 0;
    Radio.free_buffer(Radio.txbuf);
}


extern "C" void usr_radio_error(radio_error_t err)
{
    /* empty */
}

/**
 */
extern "C" void push_begin(radio_buffer_t **qhead, radio_buffer_t *buf)
{
    if (*qhead == NULL)
    {
        *qhead = buf;
        buf->next = NULL;
    }
    else
    {
        buf->next = *qhead;
        *qhead = buf;
    }
}

extern "C" radio_buffer_t * pop_begin(radio_buffer_t **qhead)
{
    radio_buffer_t * ret = NULL;
    if (*qhead == NULL)
    {
        ret = NULL;
    }
    else
    {
        ret = *qhead;
        *qhead = (radio_buffer_t *)ret->next;
    }
    if (ret != NULL)
    {
        ret->next = NULL;
    }
    return ret;
}

extern "C" void push_end(radio_buffer_t **qhead, radio_buffer_t *buf)
{

    radio_buffer_t * next, *prev;
    buf->next = NULL;

    if (*qhead == NULL)
    {
        *qhead = buf;
    }
    else
    {

        for(prev = NULL, next = *qhead;
            next != NULL;
            prev = next, next = next->next)
        {
            /* run to end of queue */
        }
        prev->next = buf;
    }

}

extern "C" radio_buffer_t * pop_end(radio_buffer_t **qhead)
{
    radio_buffer_t * next, *prev, *ret = NULL;

    if (*qhead == NULL)
    {
        ret = NULL;
    }
    else
    {
        for(prev = NULL, next = *qhead;
            next->next != NULL;
            prev = next, next = next->next)
        {
            /* run to end of queue */
        }

        ret = next;

        if (ret == *qhead)
        {
            *qhead = NULL;
        }
        else
        {
            prev->next = NULL;
        }
    }

    if (ret != NULL)
    {
        ret->next = NULL;
    }
    return ret;
}


extern "C" void flush_queue(radio_buffer_t **qhead)
{
    radio_buffer_t * p;

    do
    {
        p = pop_begin(qhead);
        Radio.free_buffer(p);
    }
    while(p!=NULL);
}
