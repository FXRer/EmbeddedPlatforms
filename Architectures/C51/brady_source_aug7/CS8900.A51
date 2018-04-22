;-------------------------------------------------------------------------
; Copyright (c) 2002 Jim Brady
; Do not use commercially without author's permission
; Last revised August 2002
; CS8900.A51
;
; This module is hardware interface to the Cirrus Logic CS8900A.
; It contains two read functions: one optimized for reading a whole
; frame from the device and one for just reading a single word. It
; also contains two write functions: one for a whole frame and one
; for a single word. Reads and writes done in 16 bit chunks.
;
; Byte order considerations: Ethernet frame data is byte swapped 
; inside the CS8900A because network order is big-endian and many
; CPUs such as x86 are little-endian.  But the CS8900A control and
; status data is not byte swapped. Thus the byte order is different
; between the write_port() and write_frame() functions below, for
; example.  This causes no loss of speed, it is just a little
; confusing.
;
; C8051F port 2 connects to CS8900A SD[0:7]
; C8051F port 3 connects to CS8900A SD[8:15]
; C8051F port 1 bits 0,1,2 set the CS8900A I/O address while the 
; higher port 0 bits are used for handshaking

; Important CS8900A specs: Read access time = 135 nsec maximum
; write strobe pulsewidth = 110 nsec minimum.  The Cygnal 8051
; is fast, so NOPs are added to get this timing.
;-------------------------------------------------------------------------

$NOMOD51

NAME	CS8900

IE	      DATA	0A8H
P1	      DATA	090H
P2	      DATA	0A0H
P3	      DATA	0B0H
DPL	   DATA	082H
DPH	   DATA	083H
PRT2CF	DATA	0A6H
PRT3CF	DATA	0A7H
EA	      BIT	0A8H.7
IOREAD	BIT	090H.3
IOWRITE	BIT	090H.4

?PR?_write_port?CS8900     SEGMENT CODE
?PR?_read_port?CS8900      SEGMENT CODE
?PR?_write_frame?CS8900   SEGMENT CODE
?PR?_read_frame?CS8900    SEGMENT CODE

PUBLIC	_read_frame
PUBLIC	_write_frame
PUBLIC	_read_port
PUBLIC	_write_port



;-------------------------------------------------------------------------
; Writes a single value to the Crystal CS8900A
; Call from C as: void write_port(UINT value, UCHAR addr)
; UINT value is in R6, R7
; UCHAR addr is in R5
;-------------------------------------------------------------------------
RSEG  ?PR?_write_port?CS8900
USING	0
_write_port:
   CLR  	EA    ; disable interrupts
 
   ; Set P2 and P3 to push pull output
	MOV  	PRT2CF,#0FFH
	MOV  	PRT3CF,#0FFH
 
   ; Take chip select low and set port address
	MOV  	A,R5
	ORL  	A,#018H
	MOV  	P1,A
 
   ; Output value to P2 and P3
   MOV   P2, R7   ; LSB
   MOV   P3, R6   ; MSB
             
   ; Take WR strobe low
	CLR  	IOWRITE
 
   ; Need at least 110 nsec, each NOP is 45 nsec @ 22.1184MHz
   NOP  	
	NOP  	
	NOP  	
      
   ; Set WR strobe and chip select lines high
	MOV  	A,R5
	ORL  	A,#038H
	MOV  	P1,A
 
   ; Set P2 and P3 back to open drain
	CLR  	A
	MOV  	PRT2CF,A
	MOV  	PRT3CF,A
	MOV  	P2,#0FFH
	MOV  	P3,#0FFH
 
	SETB 	EA    ; Enable interrupts
	RET  	


 
;------------------------------------------------------------------------
; Read one word from the CS8900A
; Call from C as: UINT read_port(UCHAR addr);
; UCHAR addr is in R7
;------------------------------------------------------------------------
	RSEG  ?PR?_read_port?CS8900
	USING	0
_read_port:
	CLR  	EA   ; disable interrupts
	MOV   R5, AR7

   ; Set P2 and P3 to open drain, high state
	CLR  	A
	MOV  	PRT2CF,A
	MOV  	PRT3CF,A
	MOV  	P2,#0FFH
	MOV  	P3,#0FFH
       	   
   ; Take chip select low and set port address
	MOV  	A,R5
	ORL  	A,#018H
	MOV  	P1,A
 
   ; take RD strobe low
	CLR  	IOREAD
 
   ; Need at least 135 nsec, each NOP is 45 nsec @ 22.1184MHz
   NOP  	
	NOP  	
	NOP  	

   ; Read the 16 bit data on P2, P3
   MOV   R7, P2   ; LSB
   MOV   R6, P3   ; MSB
      	   	   		
   ; Take RD strobe and chip select lines high
	MOV  	A,R5
	ORL  	A,#038H
	MOV  	P1,A

   SETB 	EA    ; enable interrupts
	RET  	

 
 
 
;------------------------------------------------------------------------
; Write an outgoing frame to the CS8900A
; Call from C as: void write_frame(UINT xdata * buffer, UINT length);
; xdata * buffer is in R6, R7
; UINT length is in R4, R5 - in bytes not words
; UINT length must be an even number of bytes
;------------------------------------------------------------------------
	RSEG  ?PR?_write_frame?CS8900
	USING	0
_write_frame:
   CLR   EA  ; disable interrupts
   
; Protect against length = 0
   MOV   A, R4
   ORL   A, R5
   JZ    SKIP_WRITE

; Divide length by 2 to get word count
   MOV   A, R4
   CLR   C
   RRC   A
   MOV   R4, A
   MOV   A, R5
   RRC   A
   MOV   R5, A

; Set P2 and P3 to push pull output
	MOV  	PRT2CF,#0FFH
	MOV  	PRT3CF,#0FFH
 
   ; Set up data pointer
	MOV   DPL, R7
   MOV   DPH, R6

   ; Increment hi byte of loop counter if necessary
   MOV   A, R5
   JZ    BEGIN_WRITE
   INC R4
   
   
BEGIN_WRITE:     
   ; Take chip select low and set port address
	; Port address is 0
	MOV  	P1, #018H

	; Output the 16 bit value on P2 and P3
	; Hi/lo bytes reversed compared to single-byte functions above
	MOVX 	A,@DPTR
	MOV  	P2,A    ; MSB
   INC   DPTR
	MOVX 	A,@DPTR
	MOV   P3, A   ; LSB
	
	; Go to next address
   INC  	DPTR

   ; Take WR strobe low
   CLR  	IOWRITE
 
   ; Need at least 110 nsec, each NOP is 45 nsec @ 22.1184MHz
   NOP  	
	NOP  	
   NOP  	
       
   ; Set WR strobe and chip select lines high
   MOV  	P1, #038H
 
   ; Decrement and loop again if counter not zero
   DJNZ  R5, BEGIN_WRITE
   DJNZ  R4, BEGIN_WRITE
   
   ; Set P2 and P3 back to open drain
	CLR  	A
	MOV  	PRT2CF,A
	MOV  	PRT3CF,A
	MOV  	P2,#0FFH
	MOV  	P3,#0FFH

SKIP_WRITE:
   SETB 	EA    ; enable interrupts
	RET  	

 
 
 
;------------------------------------------------------------------------
; Read an incoming frame from the CS8900A
; Call from C as: void read_frame(UINT xdata * buffer, UINT length);
; xdata * buffer is in R6, R7
; UINT length is in R4, R5 - in bytes, not words
; UINT length must be an even number of bytes
;------------------------------------------------------------------------
	RSEG  ?PR?_read_frame?CS8900
	USING	0
_read_frame:
	CLR	EA    ; Disable interrupts
   
   ; Protect against length = 0
   MOV   A, R4
   ORL   A, R5
   JZ    SKIP_READ

   ; Divide length by 2 to get word count
   MOV   A, R4
   CLR   C
   RRC   A
   MOV   R4, A
   MOV   A, R5
   RRC   A
   MOV   R5, A
         		
   ; Set P2 and P3 to open drain, high state
	CLR  	A
	MOV  	PRT2CF,A
	MOV  	PRT3CF,A
   MOV  	P2,#0FFH
	MOV  	P3,#0FFH
   
   ; Set up data pointer
	MOV  	DPL, R7
	MOV  	DPH, R6

   ; Increment hi byte of loop counter if necessary
   MOV   A, R5
   JZ    BEGIN_READ
   INC R4
    	   
BEGIN_READ:
   ; Take chip select low and set port address
   ; Port address is 0
   MOV  	P1, #018H
 
   ; Take RD strobe low
   CLR   IOREAD
 		
   ; Need at least 135 nsec, each NOP is 45 nsec @ 22.1184MHz
   NOP  	
	NOP  	
	NOP  	
      
   ; Read in the 16 bit value from CS8900A
   ; Hi/lo bytes reversed compared to single-byte functions above
	MOV  	A, P2
	MOVX	@DPTR, A     ; MSB
	INC 	DPTR
	MOV	A, P3
	MOVX	@DPTR, A     ; LSB

   ; Advance pointer to next buffer location
	INC 	DPTR
	
   ; Take RD strobe and chip select lines high
	MOV  	P1, #038H
 
   ; Decrement and loop again if counter not zero	
	DJNZ  R5, BEGIN_READ
   DJNZ  R4, BEGIN_READ

SKIP_READ:
   SETB 	EA   ; Enable interrutps
	RET  	

	END
