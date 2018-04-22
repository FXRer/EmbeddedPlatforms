;in: u32 r22..r25  u32 r18..r21    out -> r22..r25   clobber: r17,r30,r31,r0  clobbers & clears: r1

.globl m0_do_mul		;424 - 519 cycles + ret
m0_do_mul:

	clr r0
	ldi r17, 32
	clr r30
	clr r31
	
m0_do_mul_loop:
	
	lsr  r21
	ror  r20
	ror  r19
	ror  r18
	brcc m0_do_mul_no_add
	
	add  r0,  r22
	adc  r1,  r23
	adc  r30, r24
	adc  r31, r25

m0_do_mul_no_add: 

	lsl  r22
	rol  r23
	rol  r24
	rol  r25
	dec  r17
	brne m0_do_mul_loop
	
	movw r22, r0
	movw r24, r30
	clr  r1
	ret
	