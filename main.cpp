// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <stdio.h>
#include <conio.h>
#include <stdint.h>

unsigned char core[10000];
int len;

/*
Make code to calculate the sum of S:
	 17              35                             -            
     1  2  3  4  5   6  7  8  9  10  11 12  13  14
S = {1, 2, 0, 4, 10, 0, 7, 8, 0, 20, 11, 0, 13, 14, -15,.. , 0, 20000}
n = 10000
--
Si  = 0    :  if i%3 == 0
= 2*i  :  if i%5 == 0
= i    :  if i%3 != 0  and  i%5 != 0
= -i   :  if i%3 == 0  and  i%5 == 0
--
(Result: 36678337)

See make_simple_sumOf_1to10() function if need.
*/

int make_code(unsigned char*, size_t);
int make_simple_sumOf_1to10(unsigned char *);

/*
		+---+---+---+---+---+---+---+---+
		| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
		+---+---+---+---+---+---+---+---+
+-------|   comand  -     value     - OP|
|A_SET  | 0   0   0 |         value     |
|A_MIN  | 0   0   1 |   R1 -> R16   | - |
|A_PLS  | 0   1   0 |   R1 -> R16   | - |
|A_MUL  | 0   1   1 |   R1 -> R16   | - |   +---------------------+
|A_PUS  | 1   0   0 |   R0 -> R16   |0/1|    0: R1->R16;  1: R0
|A_POP  | 1   0   1 |   R0 -> R16   |0/1|    0: R1->R16;  1: R0
|A_GTO  | 1   1   0 |     LABEL     |0/1|    OP ~~ R0 : goto LABEL
|A_LAB  | 1   1   1 |     LABEL     | - |   +---------------------+
+-------+-----------+---------------+---+

The 'run_code' function is implemented to read the content from the 'code' array, with 'l' is the length.
You have a main register (R0), and 16 secondary registers (from R1 to R16).
You must return the value of R0 register at the end of this function.

The meaning of 'code' is explained in the table.
The first 3 bytes define the command to execute:
A_SET : set 'value' to R0 register.
A_MIN : minus directly the value of R0 register for another register (R1 -> R16).
A_PLS : plus directly the value of R0 register for another register (R1 -> R16).
A_MUL : multiply directly the value of R0 register for a secondary register (R1 -> R16).
(the result saved at R0)
A_PUS :  + if OP == 1, push the value of R0 to STACK
+ if OP == 0, push the value of a secondary register (R1 -> R16) to STACK.
(Maximum of STACK is 10000)
A_POP :  + if OP == 1, pop the value in STACK to R0
+ if OP == 0, pop the value in STACK to a secondary register (R1 -> R16)
A_LAB : declare a new LABEL.
A_GTO :  + if OP == 1, R0 != 0 : go to LABEL
+ if OP == 0, R0 == 0 : go to LABEL
+ if not, do nothing.
*/


typedef unsigned char Command_t;
typedef int64_t Register_t;

typedef enum{
	CMD_A_SET = 0,
	CMD_A_MIN = 1,
	CMD_A_PLS = 2,
	CMD_A_MUL = 3,
	CMD_A_PUS = 4,
	CMD_A_POP = 5,
	CMD_A_GTO = 6,
	CMD_A_LAB = 7
}CommandOpCode_t;
 
Register_t __mainRegister;
Register_t __registers[16];
Register_t __stackMem[10000];
Register_t __stackHead = 0;
Register_t __lablelsLUT[16];
Register_t __ip;


void stackPush(Register_t val)
{
	__stackMem[__stackHead] = val;
	__stackHead++;
}

Register_t stackPop(void)
{
	return __stackMem[--__stackHead];
}

void compileCode(Command_t *code, size_t len)
{
	Register_t addr = 0;
	while (addr < len){
		Command_t cmd = code[addr];
		uint8_t opcode = (cmd & 0xE0) >> 5;
		if (opcode == CMD_A_LAB){
			uint8_t labelNo = (cmd & 0x1E) >> 1;
			__lablelsLUT[labelNo] = addr + 1;
		}
		addr++;
	}
}

void executeCommand(Command_t cmd)
{
	uint8_t opcode = (cmd & 0xE0) >> 5;
	uint8_t	value = (cmd & 0x1E) >> 1;
	uint8_t opt = cmd & 0x01;;

	switch (opcode)
	{
	case CMD_A_SET:
		__mainRegister = cmd & 0x1F;
		__ip++;
		break;

	case CMD_A_MIN:
		__mainRegister -= __registers[value];
		__ip++;
		break;
	case CMD_A_PLS:
		__mainRegister += __registers[value];
		__ip++;
		break;
	case CMD_A_MUL:
		__mainRegister *= __registers[value];
		__ip++;
		break;
	case CMD_A_PUS:
		if (opt)
			stackPush(__mainRegister);
		else{
			stackPush(__registers[value]);
		}
		__ip++;
		break;
	case CMD_A_POP:
		if (opt)
			__mainRegister = stackPop();
		else{
			__registers[value] = stackPop();
		}
		__ip++;
		break;
	case CMD_A_GTO:
		opt = cmd & 0x01;
		if ((opt && __mainRegister) || (!opt && !__mainRegister)){
			__ip = __lablelsLUT[value];
		}
		else
			__ip++;
		break;

	/* ignore label */
	default:
		__ip++;
		break;
	}
}



Register_t runCode(uint8_t *code, size_t len)
{
	/* conpile */
	compileCode(code, len);

	/* reset ip */
	__ip = 0;

	/* run */
	while (__ip < len){
		//printf("execute cmd: %d\r\n", __ip);
		executeCommand(code[__ip]);
	}
		
	return __mainRegister;
}


int run_code(unsigned char *, int);

int main()
{
	int result;
	len = make_simple_sumOf_1to10(core);
	result = run_code(core, len);
	if (result == 55)
	{
		printf("Test result 1->10: OK!\n");
	}
	else
	{
		printf("Wrong result! Please check or implement run_code() function!\n");
	}

	len = make_code(core, 10000);
	result = run_code(core, len);
	printf("Result: %d\n", result);
	if (result == 36678337)
	{
		printf("Done!\n");
	}
	else
	{
		printf("Wrong answer!\n");
	}
	_getch();
	return 0;
}

int make_simple_sumOf_1to10(unsigned char * code)
{
	code[0] = 0x0a;
	code[1] = 0x81;
	code[2] = 0xA0;
	code[3] = 0x00;
	code[4] = 0x81;
	code[5] = 0xA2;
	code[6] = 0x01;
	code[7] = 0x81;
	code[8] = 0xA4;
	code[9] = 0xE0;
	code[10] = 0x00;
	code[11] = 0x42;
	code[12] = 0x40;
	code[13] = 0x81;
	code[14] = 0xA2;
	code[15] = 0x00;
	code[16] = 0x40;
	code[17] = 0x24;
	code[18] = 0x81;
	code[19] = 0xA0;
	code[20] = 0xC1;
	code[21] = 0x00;
	code[22] = 0x42;
	return 23;
}
/*
Implement 2 functions below, don't using global variant, don't cheating!! :))
*/

int run_code(unsigned char *code, int l)
{
	return runCode(code, l);
}

int make_code(unsigned char *code, size_t n)
{
	unsigned char _code[] = { 0x81, 0xa0, 0x00, 0x81, 0xa2, 0x81, 0xa4, 0x81, 0xa8, 0x81, 0xaa, 0x01, 0x81, 0xb4, 0x10, 0x81, 0xb6, 0xe0, 0x82, 0xa1, 0x54, 0x81, 0xa2, 0x84, 0xa1, 0x54, 0x81, 0xa4, 0x84, 0xa1, 0x36, 0xc3, 0x01, 0x81, 0xa4, 0xe2, 0x01, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x02, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x03, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc4, 0x04, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x05, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc8, 0x06, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc4, 0x07, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x08, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x09, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc4, 0x0a, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc8, 0x0b, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x0c, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc4, 0x0d, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x0e, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xc6, 0x0f, 0x81, 0xb2, 0x84, 0xa1, 0x32, 0xca, 0xe4, 0x00, 0x81, 0xa8, 0x01, 0xcd, 0xe6, 0x01, 0x81, 0xa8, 0x01, 0xcd, 0xe8, 0x02, 0x81, 0xa8, 0xcd, 0xea, 0x00, 0x34, 0x81, 0xa8, 0xcd, 0xec, 0x88, 0xa1, 0x62, 0x81, 0xa8, 0x8a, 0xa1, 0x48, 0x81, 0xaa, 0x80, 0xa1, 0x22, 0xc1, 0x8a, 0xa1 };
	int len = sizeof(_code);
	memcpy(code, _code, len);

	/* set N by main register */
	__mainRegister = n;
	return len;
}