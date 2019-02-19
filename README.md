# discord-challenge-z80-mul
3 challenges to multiply numbers without mul operator, resulting in 3 opcode virtual machine for i286
 Objective:

 The Z80 processor does not contain multiply or divide instructions, how did
 programmers handle this?

 Multiplying numbers that are the power of 2 is simple, you just shift them by
 N number of bits, but how do you do that if the number isn't a power of 2?

 The Z80 is an improved version of the 8080 processor created by Intel.  The
 8086 is the 16bit successor to the 8bit 8080 processor, the 8086 has multiply
 and divide instructions.

 The IMUL instruction (opcode 0x69) will multiply a 16bit register value by an
 immediate 16bit value, then store it in registers DX:AX, with DX being the
 overflow of AX.

 The program should take 2 packed 16bit little endian hex numbers on the input
 (a single little endian 32bit hex number), then multiply them and print the
 resulting HEX 32bit value.

 Input: 34127856

 Output: 6260060

 Bonus 1: Handle signed numbers, the X86 processor uses two's compliment to
 represent signed numbers, ref:
 https://en.wikipedia.org/wiki/Two%27s_complement

 Input: cced7856

 Output: f9d9ffa0

 Bonus 2: Implement an interpreter which runs the following machine code
 (shown in NASM/Intel syntax) and prints out the result:

 31 d2                	xor    dx,dx			0x30, d=0, s=1 ram, dx, dx
 b8 cc ed             	mov    ax,0xedcc		0xb8+r ax reg imm16/32 (s=1 implied)
 69 d0 78 56          	imul   dx,ax,0x5678		0x68, d=0, s=1 ram, dx, ax, imm16/32

 The input for Bonus 2 will be the following HEX string:

 31d2b8cced69d07856

 The output should be the 32bit result in DX:AX.

 Judging based on:

 a) function
 b) completeness
 c) cleanliness
 d) neat factor
