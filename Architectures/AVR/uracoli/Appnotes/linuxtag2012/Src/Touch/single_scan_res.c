#include <avr/io.h>
#include <util/delay.h>

#define COL_PADS    (_BV(PD0) | _BV(PD1) | _BV(PD2))
#define COL_LEDS    (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define ROW_IO_MASK (_BV(PB3) | _BV(PB4) | _BV(PB5))

#define NB_LEDS (9)
#define NB_ROWS (3)

#define OFF (0)
#define RED (1)
#define GREEN (2)

static uint8_t RowPattern[2*NB_ROWS] = {
							/* row mask for red display */
							0x08, 0x10, 0x20,
							/* row mask for green display */
							0x30, 0x28, 0x18 };
							
static uint8_t LedState[NB_LEDS] = {OFF, OFF, OFF,
									OFF, OFF, OFF,
									OFF, OFF, OFF};

static uint8_t RegDDRB[9] = {
								(8+1), /*led 0,0*/
								(8+2), /*led 0,1*/
								(8+4), /*led 0,2*/
								(16+1), /*led 1,0*/
								(16+2), /*led 1,1*/
								(16+4), /*led 1,2*/
								(32+1), /*led 2,0*/
								(32+2), /*led 2,1*/
								(32+4), /*led 2,2*/
							};

static uint8_t RegGreen[9] = {
							0x8,0x10,0x20,
							0x8,0x10,0x20,
							0x8,0x10,0x20,
							};

static uint8_t RegRed[9] = {
							0x1,0x2,0x4,
							0x1,0x2,0x4,
							0x1,0x2,0x4,
							};

							
static uint8_t LedMemory[NB_ROWS*2] = { /* set all LEDs off */									
										0x07, 0x07, 0x07,
										0x00, 0x00, 0x00
										};
static uint8_t LastPadState[NB_ROWS];

void io_init(void)
{
	DDRB  = COL_LEDS | ROW_IO_MASK;    
	PORTB = 0;
	DDRD = 0;
}

void set_led(uint8_t led, uint8_t color)
{
	uint8_t col = 0, row = 0;
	LedState[led] = color;
	while(col > 2)
	{
		row ++;
		col -=3;
	}
	
	
	
	LedState[led] = color;

}

int main(void)
{
	uint8_t row_cnt, col, color, led_cnt =0;
	uint8_t curr_pad_state, pad_event, i, key_event;
	io_init();
	row_cnt = 0;
	col = 0;
	color = OFF;
	curr_pad_state = 0;
	uint8_t ledcnt = 0;
	set_led(0, RED);
	set_led(4, GREEN);
	set_led(8, OFF);

	DDRB = 0x3f;
	while(1)
	{
		PORTB &= ~ROW_IO_MASK;
		PORTB = RowPattern[row_cnt] | (LedMemory[row_cnt] &7);						
        //DDRB = (DDRB & ~ROW_IO_MASK) | ~(RowPattern[row_cnt] & 7);
		row_cnt ++;
		switch(LedState[led_cnt])
		{
			case GREEN:
				DDRB = RegDDRB[led_cnt];
				PORTB = RegGreen[led_cnt];
				break;
			case RED:
				DDRB = RegDDRB[led_cnt];
				PORTB = RegRed[led_cnt];
				break;
			case OFF:
				DDRB = RegDDRB[led_cnt];
				PORTB = 0;
				break;
		}
		led_cnt ++;
		if(led_cnt>8)
		{
			led_cnt = 0;
		}
		_delay_us(250);
	}
	return 1;
}
