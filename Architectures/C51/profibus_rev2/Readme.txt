-------------------------------------------------------------------------------
Profibus test code revision 2
Jun 2006 
------------------------------------------------------------------------------- 

This code is provided with the intent that it will be used
for educational purposes only.

This code is designed to be built with the Keil v7 C51 compiler
and run on a target board with an 11.0592 MHz  Atmel AT89C52
that is  connected to a Siemens SPC3 Profibus ASIC.  The code
implements a simple RS232 to Profibus converter.  Refer to the
schematic. The SPC3 is mapped at address 0xF800. 

The code may be built either with the Makefile provided, or
with the Keil uVision project file. 

In this sample code, the SPC3 is configured for 16 bytes of
input and 16 bytes of output, and the sample GSD file
matches this.

The Atmel AT89C52 UART is setup for 19.2K baud, N, 8, 1.  
Connect a 19.2K baud RS232 terminal to the serial port.  Upon
startup it will display a banner.  After that you can hit the 
return key to see the most recent 16 bytes of data from the
Profibus master, or enter a single value between 0 and FF
(interpreted as hex) and the SPC3 will send that value to the
Master for all of its 16 bytes.

There is also an 82C55 connected to the 8051 that provides 2 ports
of input and 1 port of output.  It is mapped at 0xF600.   Three
switches connected to the 2 input ports set the Profibus address.
See schematic.  Each switch is 0-9.  No baud rate switch is needed
because the SPC3 auto negotiates baud rate.

The 82C55 output port controls a bi-color LED on the board, and 
also provides a hardware reset line to the SPC3. 

This code was tested with a Profibus DP master consisting of a
PC with an SST 5136-PFB-ISA board, running the SST Profibus
configuration program ver 1.04, and the SST DP Monitor program
ver 1.21.  See www.woodhead.com for SST products.

The Profibus DP cable was terminated on each end and tested
at 19.2K, 500K, 1.5M, and 12M bits/second.

The GSD file that was used for this is provided in the zip file.

The source code for the 22V10 address decoder pal on the Profibus
board is the file Prof_pal.pds.  It uses the description language
for PALASM, which is now public domain and available on the web.


-----------------------------------------------------------------
Jim Brady 
jimbrady@aol.com

