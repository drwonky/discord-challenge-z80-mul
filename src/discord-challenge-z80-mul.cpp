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
 * Bonus 1: Handle signed numbers, the x86 processor uses two's compliment to
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
 * 31 d2                	xor    dx,dx
 * b8 cc ed             	mov    ax,0xedcc
 * 69 d0 78 56          	imul   dx,ax,0x5678
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

string printbin(uint32_t n)
{
	string s;
	for (int i=0;i<32;i++) {
		s+=(n&(1<<31) ? "1":"0");
		if ((i&7) == 7 && i<31) s+="_";
		n<<=1;
	}
	return s;
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
	return main_signed_stripped();
}
