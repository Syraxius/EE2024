 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align 2
 	.global	distrBF
 	.thumb_func

@ EE2024 Assignment 1, Sem 2, AY 2014/15
@ (c) CK Tham, ECE NUS, 2015

@R0		T (passed from main program)
@R1		n_res (passed from main program)
@R2		dij (passed from main program)
@R3		Dj (passed from main program)
@R4		N
@R5		t counter
@R6		j counter
@R7		Array address passed to ELEMENT
@R8		Return value from ELEMENT
@R9		Current Di
@R10	Best (Minimum) Di (for current t)
@R11	Best j (for current t)

distrBF:
	PUSH {R5, R6, R7, R8, R9, R10, R11, R14}
	LDR R4, [R1]
	MOV R5, #0

	loopT:
		MOV R10, #0x7FFFFFFF
		MOV R6, #0

		loopN:
			MOV R9, #0

			MOV R7, R2
			BL ELEMENT
			ADD R9, R8

			MOV R7, R3
			BL ELEMENT
			ADD R9, R8

			CMP R9, R10
			ITT LT
			MOVLT R10, R9
			MOVLT R11, R6

			ADD R6, #1
			CMP R6, R4
			BLT loopN

		ADD R11, #1
		STR R10, [R1], #4
		STR R11, [R1], #4

		ADD R5, #1
		CMP R5, R0
		BLT loopT

	POP {R5, R6, R7, R8, R9, R10, R11, R14}

	BX LR

@ Subroutine ELEMENT
ELEMENT:
	MUL R8, R4, R5
	ADD R8, R6
	LSL R8, #2
	LDR R8, [R7, R8]

	BX LR

	NOP
	.end
