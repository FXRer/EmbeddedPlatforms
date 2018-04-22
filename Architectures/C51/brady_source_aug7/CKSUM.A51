;-------------------------------------------------------------------------
; Copyright (c) 2002 Jim Brady
; Do not use commercially without author's permission
; Last revised August 2002
; CKSUM.A51
;
; This computes the internet checksum on an 8051
;
; Call from C program: UINT cksum(UCHAR xdata * buf, UINT length);
; Where * buf is pointer to first char (must be in xdata space)
; and "length" is number of bytes (not words) to sum over
;-------------------------------------------------------------------------

$NOMOD51
NAME	CKSUM

; pulse test for debug
PULSE6   BIT  080H.6


DPL	   DATA	082H
DPH	   DATA	083H

?PR?_cksum?CKSUM     SEGMENT CODE
PUBLIC	_cksum
RSEG  ?PR?_cksum?CKSUM
USING	0
_cksum:
	; xdata pointer to buffer is in R6 and R7
   ; Set up data pointer
	MOV   DPL, R7
   MOV   DPH, R6

   ; Use R6 and R7 to hold final sum
   ; so set to zero before starting sum loop
   MOV   R6, #00H
   MOV   R7, #00H

   ; Length is in R4 and R5
   ; Save low byte of length for later
   MOV   R2, AR5

   ; Divide length by 2 to get word count
   MOV   A, R4
   CLR   C
   RRC   A
   MOV   R4, A
   MOV   A, R5
   RRC   A
   MOV   R5, A

   ; Adjust hi byte of loop counter if necessary
   MOV   A, R5
   JZ    SUM_LOOP
   INC   R4

SUM_LOOP:
   MOVX  A, @DPTR    ; Load hi byte
   INC   DPTR
   MOV   R3, A       ; Temporarily store hi byte
   MOVX  A, @DPTR    ; Load lo byte
   INC   DPTR
   ADD   A, R7       ; Add low byte
   MOV   R7, A
   MOV   A, R3       ; Retreive hi byte
   ADDC  A, R6       ; Add hi byte
   MOV   R6, A
   JNC   NOCARRY     ; See if we have a carry

   ; We do have carry so add 1 to sum
   MOV   A, #01H
   ADD   A, R7
   MOV   R7, A
   CLR   A
   ADDC  A, R6
   MOV   R6, A 
   

NOCARRY:
   ; Decrement loop counter and repeat if not zero
   DJNZ  R5, SUM_LOOP
   DJNZ  R4, SUM_LOOP


   ; See if length is odd
   MOV   A, R2
   ANL   A, #01H
   JZ    DONE


   ; Length is odd so we need to add one more byte
   ; This byte adds as the high order byte
   MOVX  A, @DPTR    ; Load byte
   ADD   A, R6       ; Add to hi byte
   MOV   R6, A
   JNC   DONE        ; See if we have a carry
      
   ; We do have carry so add 1 to sum
   MOV   A, #01H
   ADD   A, R7
   MOV   R7, A
   CLR   A
   ADDC  A, R6
   MOV   R6, A
   
   ; Final sum is in R6, R7 to return 

DONE:
  	
  	RET  	
   END


