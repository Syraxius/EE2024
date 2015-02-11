 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align 2
 	.global	distrBF
 	.thumb_func

@ EE2024 Assignment 1, Sem 2, AY 2014/15
@ (c) CK Tham, ECE NUS, 2015

@R0 = T
@R1 = n_res
@R2 = Dij
@R3 = Dj
@R4 is storing N
@R5 is T counting index
@R6 is N counting index
@R7 is passed array
@R8 is return value from ELEMENT
@R9 is accumulator (from ELEMENT)
@R10 is best Dij
@R11 is best j

distrBF:
	PUSH {R5, R6, R7, R8, R9, R10, R11, R14}
	LDR R4, [R1]
	MOV R5, #0

	loopT:
		MOV R6, #0
		MOV R10, #0x7FFFFFFF

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
