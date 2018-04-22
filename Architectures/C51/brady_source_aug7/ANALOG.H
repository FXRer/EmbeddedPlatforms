//-----------------------------------------------------------------------------
// ANALOG.H
//
//-----------------------------------------------------------------------------

void init_adc(void);
void read_analog_inputs(void);

// Analog input MUX Selects
#define MUX_CPU_TEMP				0x08
#define MUX_AIR_TEMP				0x00
#define MUX_CPU_VOLTS			0x01



