;-------------------------------------------------------------------------
; Copyright (c) 2002 Jim Brady
; Do not use commercially without author's permission
; Last revised August 2002
; START.A51
;
; This is for the small memory model
; This clears 256 bytes of RAM and 2048 bytes xdata
; then sets the reentrant stack pointer and regular stack pointer
;
;-------------------------------------------------------------------------

NAME	?C_STARTUP

?C_C51STARTUP	SEGMENT   CODE
?STACK		   SEGMENT   IDATA

RSEG	?STACK
DS	1
EXTRN CODE (?C_START)
PUBLIC	?C_STARTUP

; Set the initial jump vector
CSEG	AT	0
?C_STARTUP:	LJMP	CODESTART

RSEG	?C_C51STARTUP

CODESTART:

; Clear 256 bytes of RAM
; Can access both 128 byte spaces using mov indirect 
   MOV	R0, #0FFH
   CLR	A
LOOP1:	MOV	@R0, A
   DJNZ	R0, LOOP1

; Clear 2048 bytes of xdata memory
   MOV	DPTR, #00H     ; Start address
   MOV	R7,#00H        ; Lo byte of length
   MOV	R6,#08H        ; Hi byte of length
   CLR	A
LOOP2:	MOVX	@DPTR,A
   INC	DPTR
   DJNZ	R7, LOOP2
   DJNZ	R6, LOOP2

; Initialize the stack pointer for reentrant functions for small
; memory model. It is at the top of RAM and grows downward
   EXTRN DATA (?C_IBP)
   MOV	?C_IBP, #0FFH

; Initialize regular 8051 stack pointer for small memory model
; It is at the top of used RAM and grows upward
   MOV	SP, #?STACK-1

; Jump to main()
   LJMP	?C_START

END
