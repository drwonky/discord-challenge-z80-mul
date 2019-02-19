//============================================================================
// Name        : discord-challenge-z80-mul.cpp
// Author      : Perry Harrington
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

/*
 * Objective:
 *
 * The Z80 processor does not contain multiply or divide instructions, how did
 * programmers handle this?
 *
 * Multiplying numbers that are the power of 2 is simple, you just shift them by
 * N number of bits, but how do you do that if the number isn't a power of 2?
 *
 * The Z80 is an improved version of the 8080 processor created by Intel.  The
 * 8086 is the 16bit successor to the 8bit 8080 processor, the 8086 has multiply
 * and divide instructions.
 *
 * The IMUL instruction (opcode 0x69) will multiply a 16bit register value by an
 * immediate 16bit value, then store it in registers DX:AX, with DX being the
 * overflow of AX.
 *
 * The program should take 2 packed 16bit little endian hex numbers on the input
 * (a single little endian 32bit hex number), then multiply them and print the
 * resulting HEX 32bit value.
 *
 * Input: 34127856
 *
 * Output: 6260060
 *
 * Bonus 1: Handle signed numbers, the X86 processor uses two's compliment to
 * represent signed numbers, ref:
 * https://en.wikipedia.org/wiki/Two%27s_complement
 *
 * Input: cced7856
 *
 * Output: f9d9ffa0
 *
 * Bonus 2: Implement an interpreter which runs the following machine code
 * (shown in NASM/Intel syntax) and prints out the result:
 *
 * 31 d2                	xor    dx,dx			0x30, d=0, s=1 ram, dx, dx
 * b8 cc ed             	mov    ax,0xedcc		0xb8+r ax reg imm16/32 (s=1 implied)
 * 69 d0 78 56          	imul   dx,ax,0x5678		0x68, d=0, s=1 ram, dx, ax, imm16/32
 *
 * The input for Bonus 2 will be the following HEX string:
 *
 * 31d2b8cced69d07856
 *
 * The output should be the 32bit result in DX:AX.
 *
 * Judging based on:
 *
 * a) function
 * b) completeness
 * c) cleanliness
 * d) neat factor
 *
 */

struct x86 {
	enum { ax, cx, dx, bx, sp, bp, si, di };
	enum { al, cl, dl, bl, ah, ch, dh, bh };
	enum { eax, ecx, edx, ebx, esp, ebp, esi, edi };
	enum { ri, osb, fsb, ram };
	enum { b8, b16 };

	enum { xor_rr = 0xC, mov_r_imm16 = 0x17, imul_rr_imm16 = 0x1A };

	struct po {					// primary opcode
		union {
			struct {			// for instructions longer than 1 byte
				uint8_t s:1;	// operand size, 0=8bit reg/mem 1=16/32 operand size prefix required for 16bit in protected mode (0x66)
				uint8_t d:1;	// direction
				uint8_t op:6;	// operand
			};
			struct {			// for 1 byte instructions
				uint8_t r:3;	// register
				uint8_t opr:5;	// operand
			};
		};
	};

	struct modrm {				// describes addressing mode and which registers operand uses for destination and source
		uint8_t rm:3;			// source or destination reg
		uint8_t reg:3;			// dir = 0, reg is source, dir = 1, reg is dest
		uint8_t mod:2;			// address mode
	};

	struct sib {				// scaled index byte mode
		uint8_t base:3;
		uint8_t index:3;
		uint8_t ss:2;
	};

	union {
		po opcode;
		uint8_t	b1;
	};

	union {
		modrm mod;
		uint8_t b2;
	};

	union {
		sib SIB;
		uint8_t b3;
	};

	uint8_t dest;
	uint8_t src;

	uint16_t regs[8];
	uint16_t shadow;

	uint8_t expect;
	bool tertiary;

};

string printbin32(uint32_t n)
{
	string s;
	for (int i=0;i<32;i++) {
		s+=(n&(1<<31) ? "1":"0");
		if ((i&7) == 7 && i<31) s+="_";
		n<<=1;
	}
	return s;
}

string printbin(uint16_t n)
{
	string s;
	for (int i=0;i<16;i++) {
		s+=((n&0x8000) ? "1":"0");
		if ((i&7) == 7 && i<15) s+="_";
		n<<=1;
	}
	return s;
}

int main_cpu() {
	x86 cpu;
	string prog;

	cin >> prog;

	enum { operand, modrm, imm8 } state;
	const char *st_string[]={"_operand_","_modrm_","_imm8_"};
	enum { decode, exec } uop;

	uint8_t byte;
	string h;

	cpu.expect = 1;
	cpu.tertiary = false;

	for (auto c : prog) {
		h+=c;

		if (h.size() == 2) {
			byte = strtol(h.c_str(),NULL,16);
			h.clear();
		} else continue;

		cout << "Decoded "<< std::hex << (int)byte <<" state "<<st_string[state]<<endl;

		switch (state) {
			case operand:
				cpu.b1 = byte;
				cout << "opr "<<std::hex<<(int)cpu.opcode.opr<<" op "<<(int)cpu.opcode.op<<endl;
				switch(cpu.opcode.opr) {
					case x86::mov_r_imm16:
						cout << "0x17"<<endl;
						cpu.dest = cpu.opcode.r;
						cpu.opcode.s = 1;
						cpu.expect = 2;
						state = imm8;
						break;
					default:
						switch(cpu.opcode.op) {
							case x86::xor_rr:
								cout << "0xC"<<endl;
								cpu.expect = 1;
								state = modrm;
								break;
							case x86::imul_rr_imm16:
								cout << "0x1A"<<endl;
								cpu.expect = 3;
								cpu.tertiary = true;
								state = modrm;
								break;
						}

				}
				break;

			case modrm:
				cpu.b2 = byte;

				cout << "CPU dir "<<(int)cpu.opcode.d<<endl;
				if (cpu.opcode.d) {
					cpu.dest = cpu.mod.reg;
					cpu.src = cpu.mod.rm;
				} else {
					cpu.dest = cpu.mod.rm;
					cpu.src = cpu.mod.reg;
				}

				cout << "mod "<<(int)cpu.mod.mod<<" reg "<<(int)cpu.mod.reg<<" rm "<<(int)cpu.mod.rm<<endl;
				cout << "CPU src"<<(int)cpu.src<<" dest "<<(int)cpu.dest<<endl;
				if (cpu.expect) state = imm8;

				break;

			case imm8:
				cout << "immediate"<<endl;
				if (cpu.tertiary) {
					uint16_t temp = (uint16_t)byte << 8;
					cpu.shadow >>= 8;
					cpu.shadow |=temp;
				} else {
					if (cpu.opcode.s) {
						uint16_t temp = (uint16_t)byte << 8;
						cout <<"temp "<<std::hex<<temp<<endl;
						cpu.regs[cpu.dest]>>=8;
						cpu.regs[cpu.dest]|=temp;
					} else {
						cout <<"byte operand "<<endl;
						cpu.regs[cpu.dest]=byte;
					}
				}
				break;
		}

		if (cpu.expect == 0) {
			state = operand;
			uop = exec;
			cpu.tertiary = false;
		}

		cpu.expect--;

		if (uop == exec) {
			cout << "Executing..."<<endl;
			switch (cpu.opcode.op){
				case x86::xor_rr:
					cpu.regs[cpu.dest] = cpu.regs[cpu.dest] ^ cpu.regs[cpu.src];
					break;
				case x86::imul_rr_imm16: {
					bool sign = (cpu.regs[cpu.dest] & 0x8000) ^ (cpu.regs[cpu.src] & 0x8000);	// test if either input value is negative, both negative, or not negative and set sign of result appropriately

					if (cpu.regs[cpu.dest] & 0x8000) cpu.regs[cpu.dest] = ~cpu.regs[cpu.dest] + 1;					// convert from two's complement to unsigned
					if (cpu.shadow & 0x8000) cpu.shadow = ~cpu.shadow + 1;

					uint16_t a=cpu.regs[cpu.dest];
					uint16_t carry=0;
					cpu.regs[cpu.dest]=0;

					while (a && cpu.shadow) {							// fall through if either input is zero or when we're done
						if (a&1) {								// shift + add, test bit about to be shifted out
							cpu.regs[cpu.dest] += cpu.shadow;				// if multiplier is 1, we add shifted multiplicand and carry
							cpu.regs[cpu.src] += carry + ((cpu.shadow&0x8000)>>15);
						}
						carry <<= 1;							// carry the multiplicand bits
						carry += (cpu.shadow&0x8000)>>15;					// since we are doing 16x32 multiply, the multiplicand can't shift past 16 bits, so we have to maintain a carry
						cpu.shadow <<= 1;								// shift part of shift + add
						a >>= 1;								// decimate the multiplier for next round
					}

					if (sign) {
						cpu.regs[cpu.dest] = ~cpu.regs[cpu.dest];
						cpu.regs[cpu.src] = ~cpu.regs[cpu.src];

						if (cpu.regs[cpu.dest] < 0xFFFF ) {
							cpu.regs[cpu.dest]++;
						} else {
							cpu.regs[cpu.src]++;
							cpu.regs[cpu.dest]=0;
						}
					}
				}
				break;
			}
			uop = decode;
		}

		cout << "AX: "<<std::hex<<cpu.regs[x86::ax]<<endl;
		cout << "BX: "<<std::hex<<cpu.regs[x86::bx]<<endl;
		cout << "CX: "<<std::hex<<cpu.regs[x86::cx]<<endl;
		cout << "DX: "<<std::hex<<cpu.regs[x86::dx]<<endl;
		cout << "SP: "<<std::hex<<cpu.regs[x86::sp]<<endl;
		cout << "BP: "<<std::hex<<cpu.regs[x86::bp]<<endl;
		cout << "SI: "<<std::hex<<cpu.regs[x86::si]<<endl;
		cout << "DI: "<<std::hex<<cpu.regs[x86::di]<<endl;

	}

	printf("Output DX:%04X :AX:%04X\n",cpu.regs[x86::dx],cpu.regs[x86::ax]);

	return 0;
}

int main_signed_stripped() {
	uint32_t input;

	cin >> std::hex >> input;	// contains 2 little endian 16bit integers packed in text format

	unsigned short a, b;		// input parameters are signed 16bit integers, we don't want compiler doing anything for us

	cout << "Input: " << std::hex << input<<endl;

	a = ((input >> 8) & 0xff00) | (input >> 24);			// convert text little endian to native little endian
	b = ((input << 8) & 0xff00) | ((input >> 8) & 0xff);

	bool sign = (a & 0x8000) ^ (b & 0x8000);	// test if either input value is negative, both negative, or not negative and set sign of result appropriately

	if (a & 0x8000) a = ~a + 1;					// convert from two's complement to unsigned
	if (b & 0x8000) b = ~b + 1;

	int32_t result=0;							// our 32bit accumulator
	int32_t carry=0;							// our 32bit carry to catch bits that fall out of b

	while (a && b) {							// fall through if either input is zero or when we're done
		if (a&1) {								// shift + add, test bit about to be shifted out
			result += b + carry;				// if multiplier is 1, we add shifted multiplicand and carry
		}
		carry += ( b & 0x8000);					// since we are doing 16x32 multiply, the multiplicand can't shift past 16 bits, so we have to maintain a carry
		b <<= 1;								// shift part of shift + add
		a >>= 1;								// decimate the multiplier for next round
		carry <<= 1;							// carry the multiplicand bits
	}

	if (sign) result = ~result + 1;				// if the result is a negative number, convert to two's complement

	cout << "Output: "<<std::hex<<result<<endl;

	return 0;
}

int main_signed() {
	uint32_t input;

	cin >> std::hex >> input;	// contains 2 little endian 16bit integers packed in text format

	unsigned short a, b;

	cout << "Input: " << std::hex << input<<endl;

	a = ((input >> 8) & 0xff00) | (input >> 24);	// convert text little endian to native little endian
	b = ((input << 8) & 0xff00) | ((input >> 8) & 0xff);

	cout << "Unpacked " << std::hex << a << " " << b << endl;

	cout << "A "<<printbin(a)<<" B "<<printbin(b)<<endl;

	bool sign = (a & 0x8000) ^ (b & 0x8000);

	if (a & 0x8000) a = ~a + 1;
	if (b & 0x8000) b = ~b + 1;

	cout << "A "<<printbin(a)<<" B "<<printbin(b)<<endl;
	cout << "Unpacked " << std::hex << a << " " << b << endl;

	int32_t result=0;
	int32_t carry=0;

	while (a) {
		cout << "A "<<printbin(a)<<" B "<<printbin(b)<<" result "<<printbin(result)<<" "<<result<<endl;
		if (a&1) {
			result += b + carry;
		}
		carry += ( b & 0x8000);
		b <<= 1;
		a >>= 1;
		carry <<= 1;
	}

	cout << "Output: "<<std::hex<<result<<endl;
	cout << "Output: "<<printbin(result)<<endl;
	cout << "Output: "<<std::dec<<result<<endl;
	cout << "Output: "<<printbin(result*-1)<<endl;

	if (sign) result = ~result + 1;

	cout << "Output: "<<printbin(result)<<endl;
	cout << "Output: "<<std::hex<<result<<endl;
	cout << "Output: "<<std::dec<<result<<endl;
	return 0;
}

int main_unsigned() {
	uint32_t input;

	cin >> std::hex >> input;	// contains 2 little endian 16bit integers packed in text format

	uint32_t a, b;

	cout << "Input: " << std::hex << input<<endl;

	a = ((input >> 8) & 0xff00) | (input >> 24);	// convert text little endian to native little endian
	b = ((input << 8) & 0xff00) | ((input >> 8) & 0xff);

	cout << "Unpacked " << std::hex << a << " " << b << endl;

	uint32_t result=0;

	for (int i=0; i<16; i++) {
		cout << "A "<<printbin(a)<<" B "<<printbin(b)<<" result "<<printbin(result)<<" "<<result<<endl;
		if (a&1) {
			result+=b;
		}
		b<<=1;
		a>>=1;
	}

	cout << "Output: "<<std::hex<<result<<endl;
	return 0;
}

int main() {
	return main_cpu();
}
