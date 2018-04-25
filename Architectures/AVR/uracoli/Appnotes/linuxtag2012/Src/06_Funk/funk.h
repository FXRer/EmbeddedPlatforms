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

#define PAYLD_SIZE        (116)
#define CRC_SIZE          (sizeof(crc_t))
#define TRX_FRAME_SIZE    (sizeof(xxo_frame_t))
#define PAYLD_START       (sizeof(prot_header_t))
#define CHANNEL 17
#define SHORTADDR 0x4214
#define PANID 0xF00F
#define FRAME_CTRL_FIELD (0x8841)

/* === types =============================================================== */
typedef struct
{
    /* 802.15.4 MAC Header */
    uint16_t fcf;   /**< Frame control field*/
    uint8_t  seq;   /**< Sequence number */
    uint16_t panid; /**< 16 bit PAN ID */
    uint16_t dst;   /**< 16 bit destination address */
    uint16_t src;   /**< 16 bit source address */

    /* Payload */
    uint8_t key;

    /* 802.15.4 MAC Footer */
    uint16_t crc_t;

} xxo_frame_t;

typedef uint16_t crc_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    /* Function prototypes from leds.c */
    uint8_t update_pads(uint8_t row);
    void display_leds(uint8_t row);
    void io_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
