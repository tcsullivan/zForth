.section ".resetvec", "a"
	.word	_start

.section ".text"
_start:
	mov	#__stack, R1

	mov	#__bssstart, R12
	clr.w	R13
	mov	#__bsssize, R14
	call	#memset

	mov	#__datastart, R12
	mov	#__romdatastart, R13

	;;  memmove and memcpy do not currently work when src == dst
	cmp	R12, R13
	jeq	1f

	mov	#__romdatacopysize, R14

	call	#memmove
1:

	mov	#__preinit_array_start, R4
	mov	#__preinit_array_end, R5
	mov	#__preinit_array_size, R6
	call	#__crt0_run_array

	mov	#__init_array_start, R4
	mov	#__init_array_end, R5
	mov	#__init_array_size, R6
	call	#__crt0_run_array

	clr.w	R12		; Set argc == 0
	call	#main

__crt0_run_array:
	cmp	R4, R5
	jeq	_msp430_run_done
	mov	@R4, R7
	add	R6, R4
	call	R7
	br	#__crt0_run_array
_msp430_run_done:
        ret

