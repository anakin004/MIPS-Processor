#include "spimcore.h"


/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch(ALUControl)
    {
        case 0: // A+B
            *ALUresult = A+B;
            break;
        
        case 1: // A-B
            *ALUresult = A-B;
            break;
        
        case 2: // A < B
            *ALUresult = (A<B) ? 1 : 0;
            break;
        
        case 3: // A < B (both unsigned)
            *ALUresult = (A<B) ? 1 : 0;
            break;
        
        case 4: // A and B
            *ALUresult = A & B;
            break;

        case 5: // A or B
            *ALUresult = A | B;
            break;

        case 6: // shift B by 16
            *ALUresult = B << 16;
            break;
        
        case 7: // not/inverse A
            *ALUresult = ~A;
            break;
        
        default:
            break;
    }
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    *instruction = Mem[PC >> 2];

    if( *instruction % 4 != 0 )
        return 1;

    return PC + 4;
}


/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    /*
    for refrece:

    unsigned op,	// instruction [31-26]
	r1,	// instruction [25-21]
	r2,	// instruction [20-16]
	r3,	// instruction [15-11]
	funct,	// instruction [5-0]
	offset,	// instruction [15-0]
	jsec;	// instruction [25-0]
    */

    // last 6 bits
    *op = instruction >> 25;

    // last 12 bits but we & with 0000 0001 1111
    // to get the 5 bits we want
    *r1 = instruction >> 20 & 0x01F;

}



/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{

}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // 0x indicates hexidecimal
    // 8000 is 1000 0000 0000 0000,

    // we want to check the 16th bit to see if its negative
    // if it is offset & 0x8000 will be 1000 0000 0000 0000 
    // else if will be 0000 0000 0000 0000

    // if offset is negative we want to extend 1's else we extend 0's
    if(offset & 0x8000)
        *extended_value = offset | 0xFFFF0000;
    else
        *extended_value = offset & 0x0000FFFF;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

}

