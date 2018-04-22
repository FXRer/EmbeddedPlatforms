---------------------------------------------------------------------
SOURCE CODE FOR 8051 WEB SERVER
The code is Copyright (c) 2002 Jim Brady
Do not use commercially without author's permission
---------------------------------------------------------------------

The source compiles with Keil C51 / A51 version 5.10,
with no warnings.  I have also compiled it with the Tasking
compiler version 4.1 with some tweaking of the source. 

Compile and link the program using the included makefile. The
source zip also includes net.pjt which is a project file for
CodeWright.  The source code includes pragmas to set compiler
options - if you use an IDE be careful to not change these.

It is designed to run on the Cygnal C8051F005DK evaluation kit
target board, modified and connected to a Cirrus Logic CS8900A
10 Base-T Ethernet controller, as described in Circuit Cellar
article.

The web server has been successfully tested with Netscape 4.7,
Netscape 6.2, and MS IE 5.5

Jim Brady
jimbrady@aol.com
August 5, 2002


