#include "WProgram.h"

rx_frame_t rxbuf;

int main(void)
{
	init();
	setup();
    
	for (;;)
		loop();
        
	return 0;
}
