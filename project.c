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

    // last 12 bits but we & with 0000 0000 0000 0000 0000 0000 0001 1111
    // to get the 5 bits we want
    *r1 = instruction >> 20 & 0x01F;

    // keeping last 5 bits of the shift
    *r2 = (instruction >> 16) & 0x1F;

    // keeping last 5 bits of shift
    *r3 = (instruction >> 11) & 0x1F;

    // keeping last 6 bits after shift
    *funct = instruction & 0x3F;

    // keeping last 16 bits 
    *offset = instruction & 0xFFFF;

    // keep last 26 bits
    *jsec = instruction & 0x3FFFFFF;

}




/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // init all control signals to zero by default

    controls->ALUSrc = 0;
    controls->RegDst = 0;
    controls->RegWrite = 0;
    controls->MemRead = 0;
    controls->MemWrite = 0;
    controls->MemtoReg = 0;
    controls->Branch = 0;
    controls->Jump = 0;
    controls->ALUOp = 0;

    switch( op) {

        // r type (add,sub,stl,and,or)
        case 0:
            controls->ALUSrc = 0;
            controls->RegDst = 1;
            controls->RegWrite = 1;
            controls->MemtoReg = 0;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->Jump = 0;
            controls->ALUOp = 2;
            break;

        // load word
        case 35:
            controls->ALUSrc = 1;
            controls->RegDst = 0;
            controls->RegWrite = 1;
            controls->MemtoReg = 0;
            controls->MemRead = 1;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->Jump = 0;
            controls->ALUOp = 0;
            break;

        // store word
        case 43:
            controls->ALUSrc = 1;
            controls->RegDst = 'X';
            controls->RegWrite = 0;
            controls->MemtoReg = 'X';
            controls->MemRead = 0;
            controls->MemWrite = 1;
            controls->Branch = 0;
            controls->Jump = 0;
            controls->ALUOp = 0;
            break;
        
        // beq
        case 4:
            controls->ALUSrc = 0;
            controls->RegDst = 'X';
            controls->RegWrite = 0;
            controls->MemtoReg = 'X';
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 1;
            controls->Jump = 0;
            controls->ALUOp = 1;
            break;

        // jump

        case 2:
            controls->ALUSrc = 0;
            controls->RegDst = 'X';
            controls->RegWrite = 0;
            controls->MemtoReg = 'X';
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->Jump = 1;
            controls->ALUOp = 0;
            break;
        
        default:
            return 1;
    }

}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
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
    unsigned ALUControl;
    unsigned operand2;

    if (ALUSrc == 1)
    {
    operand2 = extended_value;
    } else{
    operand2 = data2;
    }

    if (ALUOp == 7)
    { // R-type instruction
        ALUControl = funct & 0x3F; // Extract lower 6 bits of function
    } else{
        ALUControl = ALUOp; // Direct ALUOp for non-R-type instructions
    }
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

    if (ALUresult % 4 != 0)
    //Checking if the address is word-aligned
    {
        return 1; // Halt due to misaligned address
    }

    if (ALUresult / 4 >= 16384)
    //Checking if the address is within memory bounds
    { //64KB memory, each word is 4 bytes
        return 1;//Halt due to out-of-bounds access
    }

    if (MemWrite)
    //Perform memory write
    {
        Mem[ALUresult / 4] = data2;
        // Write data2 to the memory
    }

    if (MemRead)
    { //Perform memory read
        *memdata = Mem[ALUresult / 4];
        //Read from memory into memdata
    }

    return 0;
}


/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    
    if (RegWrite)
    //Checking if RegWrite is enabled
    {
        unsigned destination; //The destination register
        unsigned data;        //The data to write

        if (RegDst)
        //Determine the destination register
        {
            destination = r3; //R-type instruction
        } else{
            destination = r2; //I-type instruction
        }

        if (MemtoReg)
        //Determine the data to write
        {
            data = memdata; //Data from memory
        } else{
            data = ALUresult; //Data from ALU
        }

        //Write the data to the register file
        Reg[destination] = data;
    }
}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

}

