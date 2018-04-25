#include <stdint.h>
/* === macros ============================================================== */
#define COL_PADS    (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define OFF (0)
#define GREEN (1)
#define RED (2)

#define DISPLAY_RED (0)
#define DISPLAY_GREEN (3)

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    /* Function prototypes from leds.c */
    void display_leds(uint8_t row);
    void io_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
