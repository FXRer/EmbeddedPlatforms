.section .vecs

__reset_vector_ptr:
	rjmp __reset_vector

//vector table for cpu (forward all but INT0)
	rjmp __vector_1
	
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2
	rjmp  . + 0xd00 - 2

//vector table for us
	rjmp  usbWork
	rjmp  packetCanSend
	rjmp  packetSend
	rjmp  packetRx
	rjmp  packetRxRelease
	rjmp  bootloader
	rjmp  getUsbCaps
	rjmp  usbReenumerate
	rjmp  blGetBlVersion

	
.section .text

__reset_vector:

	//init r1 and SREG
	clr  r1
	out  0x3F, r1	//SREG

	//point stack to end of ram
	ldi  r28, lo8(__stack_top)
	ldi  r29, hi8(__stack_top)
	out  0x3E, r29	//SPH
	out  0x3D, r28	//SPL

	//load data start and end, copy data
	ldi  r17, hi8(__data_end_bss_start)
	ldi  r26, lo8(__data_start)
	ldi  r27, hi8(__data_start)
	ldi  r30, lo8(__data_data)
	ldi  r31, hi8(__data_data)

	//copy data
	rjmp __data_copy_loop_check
	
__data_copy_loop:
	lpm  r0, Z+
	st   X+, r0

__data_copy_loop_check:
	cpi  r26, lo8(__data_end_bss_start)
	cpc  r27, r17
	brne __data_copy_loop
	
	//load bss start and end ptrs
	ldi  r18, hi8(__bss_end)
	
	//clear bss
	rjmp __bss_clear_loop_check

__bss_clear_loop:
	st   X+, r1

__bss_clear_loop_check:
	cpi  r26, lo8(__bss_end)
	cpc  r27, r18
	brne __bss_clear_loop


	rjmp main


.global flashErase
flashErase:
	ldi  r18, 0b00000011
	
movw_r24_to_r30_and_write_r18_to_spmcsr_and_then_wait_and_then_reenable_flash_read_and_return:
	movw r30, r24		//Z reg
	out  0x37, r18		//SPMCSR
	spm

1:	//wait for completion
	in  r24, 0x37		//SPMCSR
	sbrc r24, 0
	rjmp 1b
	
	ret

.global flashWrite
flashWrite:

	movw r30, r24	//Z reg
	movw r26, r22	//X reg
	ldi  r19, 32
	
1:	//fill buffer
	ld   r0, X+
	ld   r1, X+
	ldi  r20, 0b00000001
	out	 0x37, r20		//SPMCSR
	spm
	adiw r30, 2			//Z reg
	dec	 r19
	brne 1b

	clr  r1

	//write
	ldi  r18, 0b00000101
	rjmp movw_r24_to_r30_and_write_r18_to_spmcsr_and_then_wait_and_then_reenable_flash_read_and_return
