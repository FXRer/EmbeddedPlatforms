/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "captouch.h"

/* === macros ============================================================== */
#define NSAMPLES (16) /* must match the inline asm below! */
#define NSAMPLE  (2)
#define KEY_NONE (255)

/* === globals ============================================================= */
uint8_t LedState[9] = { RED,   GREEN, RED,
                        RED,   OFF,   GREEN,
                        GREEN, GREEN, RED};

static uint8_t SamplesPortD[NSAMPLES], SamplesPortG[NSAMPLES];
static uint8_t PadState[9];

/* === functions =========================================================== */

/*
 * Sample port D's input pins for their attached capacitance.
 *
 * First, the port is set to output, at low level, to discharge the
 * capacitor attached to the pin.  Then, the port is turned into an
 * input, and the input pullups are applied.  This causes the input
 * capacitor to be slowly charged, while the digital input register is
 * continuously sampled 16 times.  As the timing of this part is
 * crucial, inline assembly is used to quickly (and in equal time
 * steps) sample the input data into registers, from where they can be
 * stored into their final destination later on (by the compiler).
 */
static void sample_port_d(void)
{
    PORTD = 0;
    DDRD = 0xe0;
    __asm ("nop");
    DDRD = 0;
    PORTD = 0xe0;
    __asm ("in %[s0], %[pind]" "\n\t"
           "in %[s1], %[pind]" "\n\t"
           "in %[s2], %[pind]" "\n\t"
           "in %[s3], %[pind]" "\n\t"
           "in %[s4], %[pind]" "\n\t"
           "in %[s5], %[pind]" "\n\t"
           "in %[s6], %[pind]" "\n\t"
           "in %[s7], %[pind]" "\n\t"
           "in %[s8], %[pind]" "\n\t"
           "in %[s9], %[pind]" "\n\t"
           "in %[s10], %[pind]" "\n\t"
           "in %[s11], %[pind]" "\n\t"
           "in %[s12], %[pind]" "\n\t"
           "in %[s13], %[pind]" "\n\t"
           "in %[s14], %[pind]" "\n\t"
           "in %[s15], %[pind]"
           :
           /* output operands */
           [s0] "=r" (SamplesPortD[0]),
           [s1] "=r" (SamplesPortD[1]),
           [s2] "=r" (SamplesPortD[2]),
           [s3] "=r" (SamplesPortD[3]),
           [s4] "=r" (SamplesPortD[4]),
           [s5] "=r" (SamplesPortD[5]),
           [s6] "=r" (SamplesPortD[6]),
           [s7] "=r" (SamplesPortD[7]),
           [s8] "=r" (SamplesPortD[8]),
           [s9] "=r" (SamplesPortD[9]),
           [s10] "=r" (SamplesPortD[10]),
           [s11] "=r" (SamplesPortD[11]),
           [s12] "=r" (SamplesPortD[12]),
           [s13] "=r" (SamplesPortD[13]),
           [s14] "=r" (SamplesPortD[14]),
           [s15] "=r" (SamplesPortD[15])
           :
           /* input operands */
           [pind] "I" (_SFR_IO_ADDR(PIND)));
}

/*
 * Same as for port D above, but only portpin G0 is used as an input,
 * all other pins are not used for that purpose (but are sampled
 * anyway, as sampling always applies to a full port).
 */
static void sample_port_g(void)
{
    PORTG = 0;
    DDRG = 0x3f;
    __asm ("nop");
    DDRG = 0;
    PORTG = 0x3f;
    __asm ("in %[s0], %[ping]" "\n\t"
           "in %[s1], %[ping]" "\n\t"
           "in %[s2], %[ping]" "\n\t"
           "in %[s3], %[ping]" "\n\t"
           "in %[s4], %[ping]" "\n\t"
           "in %[s5], %[ping]" "\n\t"
           "in %[s6], %[ping]" "\n\t"
           "in %[s7], %[ping]" "\n\t"
           "in %[s8], %[ping]" "\n\t"
           "in %[s9], %[ping]" "\n\t"
           "in %[s10], %[ping]" "\n\t"
           "in %[s11], %[ping]" "\n\t"
           "in %[s12], %[ping]" "\n\t"
           "in %[s13], %[ping]" "\n\t"
           "in %[s14], %[ping]" "\n\t"
           "in %[s15], %[ping]"
           :
           /* output operands */
           [s0] "=r" (SamplesPortG[0]),
           [s1] "=r" (SamplesPortG[1]),
           [s2] "=r" (SamplesPortG[2]),
           [s3] "=r" (SamplesPortG[3]),
           [s4] "=r" (SamplesPortG[4]),
           [s5] "=r" (SamplesPortG[5]),
           [s6] "=r" (SamplesPortG[6]),
           [s7] "=r" (SamplesPortG[7]),
           [s8] "=r" (SamplesPortG[8]),
           [s9] "=r" (SamplesPortG[9]),
           [s10] "=r" (SamplesPortG[10]),
           [s11] "=r" (SamplesPortG[11]),
           [s12] "=r" (SamplesPortG[12]),
           [s13] "=r" (SamplesPortG[13]),
           [s14] "=r" (SamplesPortG[14]),
           [s15] "=r" (SamplesPortG[15])
           :
           /* input operands */
           [ping] "I" (_SFR_IO_ADDR(PING)));
}

uint8_t update_pads(void)
{
    uint8_t scans[9], i;
    uint8_t *pscan, *pstate, ret;
    ret = KEY_NONE;
    sample_port_d();
    sample_port_g();

    /* Map the */
    for (i=0; i<9;i++)
    {
        switch(i)
        {
            case 0:
                scans[0] = (SamplesPortD[NSAMPLE] & _BV(PD7)) ;
                break;
            case 1:        /* detect key events */
                scans[1] = (SamplesPortG[NSAMPLE] & _BV(PG5)) ;
                break;
            case 2:
                scans[2] = (SamplesPortG[NSAMPLE] & _BV(PG2)) ;
                break;
            case 3:
                scans[3] = (SamplesPortD[NSAMPLE] & _BV(PD6)) ;
                break;
            case 4:
                scans[4] = (SamplesPortG[NSAMPLE] & _BV(PG4)) ;
                break;
            case 5:
                scans[5] = (SamplesPortG[NSAMPLE] & _BV(PG1)) ;
                break;
            case 6:
                scans[6] = (SamplesPortD[NSAMPLE] & _BV(PD5)) ;
                break;
            case 7:
                scans[7] = (SamplesPortG[NSAMPLE] & _BV(PG3)) ;
                break;
            case 8:
                scans[8] = (SamplesPortG[NSAMPLE] & _BV(PG0)) ;
                break;
        }

        /* detect key events */
        if ((scans[i] > 0) && (PadState[i] != '_'))
        {
            PadState[i] = '_';
        }

        if ((scans[i] == 0) && (PadState[i] != '#'))
        {
            PadState[i] = '#';
            ret = i;
            __asm ("nop");
            /* verlasse Schleife bei der ersten gedrückten Taste */
            break;
        }
    }

    return ret;
}

ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;
    uint8_t key;

    key = update_pads();

    if (key != KEY_NONE)
    {
        LedState[key] ++;
        if(LedState[key] > 2)
        {
            LedState[key] = 0;
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
    io_init();
    set_sleep_mode(SLEEP_MODE_IDLE);
    while(1)
    {
        sleep_mode();
    }

    return 0;
}
