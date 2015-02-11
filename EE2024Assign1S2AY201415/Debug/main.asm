   1              		.syntax unified
   2              		.cpu cortex-m3
   3              		.fpu softvfp
   4              		.eabi_attribute 20, 1
   5              		.eabi_attribute 21, 1
   6              		.eabi_attribute 23, 3
   7              		.eabi_attribute 24, 1
   8              		.eabi_attribute 25, 1
   9              		.eabi_attribute 26, 1
  10              		.eabi_attribute 30, 6
  11              		.eabi_attribute 34, 1
  12              		.eabi_attribute 18, 4
  13              		.thumb
  14              		.file	"main.c"
  15              		.text
  16              	.Ltext0:
  17              		.cfi_sections	.debug_frame
  18              		.section	.rodata
  19              		.align	2
  20              	.LC2:
  21 0000 25642025 		.ascii	"%d %d\012\000"
  21      640A00
  22 0007 00       		.align	2
  23              	.LC0:
  24 0008 09000000 		.word	9
  25 000c 05000000 		.word	5
  26 0010 07000000 		.word	7
  27 0014 08000000 		.word	8
  28 0018 07000000 		.word	7
  29 001c 09000000 		.word	9
  30 0020 06000000 		.word	6
  31 0024 09000000 		.word	9
  32 0028 0A000000 		.word	10
  33 002c 08000000 		.word	8
  34 0030 08000000 		.word	8
  35 0034 07000000 		.word	7
  36              		.align	2
  37              	.LC1:
  38 0038 11000000 		.word	17
  39 003c 17000000 		.word	23
  40 0040 0E000000 		.word	14
  41 0044 0F000000 		.word	15
  42 0048 12000000 		.word	18
  43 004c 13000000 		.word	19
  44 0050 12000000 		.word	18
  45 0054 0E000000 		.word	14
  46 0058 13000000 		.word	19
  47 005c 16000000 		.word	22
  48 0060 11000000 		.word	17
  49 0064 10000000 		.word	16
  50              		.section	.text.main,"ax",%progbits
  51              		.align	2
  52              		.global	main
  53              		.thumb
  54              		.thumb_func
  56              	main:
  57              	.LFB0:
  58              		.file 1 "../src/main.c"
   1:../src/main.c **** #include "stdio.h"
   2:../src/main.c **** 
   3:../src/main.c **** extern void distrBF(int T_arg, int* n_res_arr, int* dij_arr, int* Dj_arr);
   4:../src/main.c **** 
   5:../src/main.c **** // EE2024 Assignment 1, Sem 2, AY 2014/15
   6:../src/main.c **** // (c) CK Tham, ECE NUS, 2015
   7:../src/main.c **** 
   8:../src/main.c **** #define T 3
   9:../src/main.c **** #define N 4
  10:../src/main.c **** 
  11:../src/main.c **** int main(void)
  12:../src/main.c **** {
  59              		.loc 1 12 0
  60              		.cfi_startproc
  61              		@ args = 0, pretend = 0, frame = 128
  62              		@ frame_needed = 1, uses_anonymous_args = 0
  63 0000 B0B5     		push	{r4, r5, r7, lr}
  64              	.LCFI0:
  65              		.cfi_def_cfa_offset 16
  66              		.cfi_offset 14, -4
  67              		.cfi_offset 7, -8
  68              		.cfi_offset 5, -12
  69              		.cfi_offset 4, -16
  70 0002 A0B0     		sub	sp, sp, #128
  71              	.LCFI1:
  72              		.cfi_def_cfa_offset 144
  73 0004 00AF     		add	r7, sp, #0
  74              	.LCFI2:
  75              		.cfi_def_cfa_register 7
  13:../src/main.c **** 	// Variable definitions
  14:../src/main.c **** 	int i;
  15:../src/main.c **** 
  16:../src/main.c **** 	// Note: different initialisation list is needed for different T, N
  17:../src/main.c **** 	int dij[T][N]={{9,5,7,8},
  76              		.loc 1 17 0
  77 0006 40F20003 		movw	r3, #:lower16:.LC0
  78 000a C0F20003 		movt	r3, #:upper16:.LC0
  79 000e 07F14C04 		add	r4, r7, #76
  80 0012 1D46     		mov	r5, r3
  81 0014 0FCD     		ldmia	r5!, {r0, r1, r2, r3}
  82 0016 0FC4     		stmia	r4!, {r0, r1, r2, r3}
  83 0018 0FCD     		ldmia	r5!, {r0, r1, r2, r3}
  84 001a 0FC4     		stmia	r4!, {r0, r1, r2, r3}
  85 001c 95E80F00 		ldmia	r5, {r0, r1, r2, r3}
  86 0020 84E80F00 		stmia	r4, {r0, r1, r2, r3}
  18:../src/main.c **** 	               {7,9,6,9},
  19:../src/main.c **** 	               {10,8,8,7}};
  20:../src/main.c **** 
  21:../src/main.c **** 	int Dj[T][N]={{17,23,14,15},
  87              		.loc 1 21 0
  88 0024 40F20003 		movw	r3, #:lower16:.LC1
  89 0028 C0F20003 		movt	r3, #:upper16:.LC1
  90 002c 07F11C04 		add	r4, r7, #28
  91 0030 1D46     		mov	r5, r3
  92 0032 0FCD     		ldmia	r5!, {r0, r1, r2, r3}
  93 0034 0FC4     		stmia	r4!, {r0, r1, r2, r3}
  94 0036 0FCD     		ldmia	r5!, {r0, r1, r2, r3}
  95 0038 0FC4     		stmia	r4!, {r0, r1, r2, r3}
  96 003a 95E80F00 		ldmia	r5, {r0, r1, r2, r3}
  97 003e 84E80F00 		stmia	r4, {r0, r1, r2, r3}
  22:../src/main.c **** 	              {18,19,18,14},
  23:../src/main.c **** 	              {19,22,17,16}};
  24:../src/main.c **** 
  25:../src/main.c **** 	int n_res[T][2]={{N,0},
  98              		.loc 1 25 0
  99 0042 4FF00403 		mov	r3, #4
 100 0046 7B60     		str	r3, [r7, #4]
 101 0048 4FF00003 		mov	r3, #0
 102 004c BB60     		str	r3, [r7, #8]
 103 004e 4FF00003 		mov	r3, #0
 104 0052 FB60     		str	r3, [r7, #12]
 105 0054 4FF00003 		mov	r3, #0
 106 0058 3B61     		str	r3, [r7, #16]
 107 005a 4FF00003 		mov	r3, #0
 108 005e 7B61     		str	r3, [r7, #20]
 109 0060 4FF00003 		mov	r3, #0
 110 0064 BB61     		str	r3, [r7, #24]
  26:../src/main.c **** 	                 {0,0},
  27:../src/main.c **** 	                 {0,0}};
  28:../src/main.c **** 
  29:../src/main.c **** 	// Call assembly language function
  30:../src/main.c **** 	distrBF(T,(int*)n_res,(int*)dij,(int*)Dj);
 111              		.loc 1 30 0
 112 0066 07F10401 		add	r1, r7, #4
 113 006a 07F14C02 		add	r2, r7, #76
 114 006e 07F11C03 		add	r3, r7, #28
 115 0072 4FF00300 		mov	r0, #3
 116 0076 FFF7FEFF 		bl	distrBF
  31:../src/main.c **** 
  32:../src/main.c **** 	// Print out results on console
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
 117              		.loc 1 33 0
 118 007a 4FF00003 		mov	r3, #0
 119 007e FB67     		str	r3, [r7, #124]
 120 0080 1AE0     		b	.L2
 121              	.L3:
  34:../src/main.c **** 		printf("%d %d\n", n_res[i][0], n_res[i][1]);
 122              		.loc 1 34 0 discriminator 2
 123 0082 40F20003 		movw	r3, #:lower16:.LC2
 124 0086 C0F20003 		movt	r3, #:upper16:.LC2
 125 008a FA6F     		ldr	r2, [r7, #124]
 126 008c 4FEAC202 		lsl	r2, r2, #3
 127 0090 07F18000 		add	r0, r7, #128
 128 0094 8218     		adds	r2, r0, r2
 129 0096 52F87C1C 		ldr	r1, [r2, #-124]
 130 009a FA6F     		ldr	r2, [r7, #124]
 131 009c 4FEAC202 		lsl	r2, r2, #3
 132 00a0 07F18000 		add	r0, r7, #128
 133 00a4 8218     		adds	r2, r0, r2
 134 00a6 52F8782C 		ldr	r2, [r2, #-120]
 135 00aa 1846     		mov	r0, r3
 136 00ac FFF7FEFF 		bl	printf
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
 137              		.loc 1 33 0 discriminator 2
 138 00b0 FB6F     		ldr	r3, [r7, #124]
 139 00b2 03F10103 		add	r3, r3, #1
 140 00b6 FB67     		str	r3, [r7, #124]
 141              	.L2:
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
 142              		.loc 1 33 0 is_stmt 0 discriminator 1
 143 00b8 FB6F     		ldr	r3, [r7, #124]
 144 00ba 022B     		cmp	r3, #2
 145 00bc E1DD     		ble	.L3
 146              	.L4:
  35:../src/main.c **** 
  36:../src/main.c **** 	// Enter an infinite loop, just incrementing a counter
  37:../src/main.c **** 	volatile static int loop = 0;
  38:../src/main.c **** 	while (1) {
  39:../src/main.c **** 		loop++;
 147              		.loc 1 39 0 is_stmt 1 discriminator 1
 148 00be 40F20003 		movw	r3, #:lower16:loop.3821
 149 00c2 C0F20003 		movt	r3, #:upper16:loop.3821
 150 00c6 1B68     		ldr	r3, [r3, #0]
 151 00c8 03F10102 		add	r2, r3, #1
 152 00cc 40F20003 		movw	r3, #:lower16:loop.3821
 153 00d0 C0F20003 		movt	r3, #:upper16:loop.3821
 154 00d4 1A60     		str	r2, [r3, #0]
  40:../src/main.c **** 	}
 155              		.loc 1 40 0 discriminator 1
 156 00d6 F2E7     		b	.L4
 157              		.cfi_endproc
 158              	.LFE0:
 160              		.bss
 161              		.align	2
 162              	loop.3821:
 163 0000 00000000 		.space	4
 164              		.text
 165              	.Letext0:
DEFINED SYMBOLS
                            *ABS*:00000000 main.c
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:19     .rodata:00000000 $d
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:20     .rodata:00000000 .LC2
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:23     .rodata:00000008 .LC0
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:37     .rodata:00000038 .LC1
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:51     .text.main:00000000 $t
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:56     .text.main:00000000 main
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:162    .bss:00000000 loop.3821
C:\Users\Kelvin\AppData\Local\Temp\ccozJmCO.s:161    .bss:00000000 $d
                     .debug_frame:00000010 $d

UNDEFINED SYMBOLS
distrBF
printf
