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
            *ALUresult = ((int)A < (int)B) ? 1 : 0;
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

    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // checking if address is divisible by 4 for alignment
    if ( PC % 4 != 0 )
        return 1;

    *instruction = Mem[PC >> 2];

    return 0;
}



/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    /*
    refrence spincore.c for instruction layout
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
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc,
                   unsigned *ALUresult, char *Zero) {
                    
    data2 = ALUSrc == 1 ? extended_value : data2;

    if (ALUOp == 7) {
        switch (funct) {
            case 32:
                // adding
                ALUOp = 0;
                break;
            case 34:
                // sub
                ALUOp = 0;
                break;
            case 36:
                // and
                ALUOp = 4;
                break;
            case 37:
                // or
                ALUOp = 5;
                break;
            case 42:
                // stl signed
                ALUOp = 2;
                break;
            case 43:
                // stl unsigned
                ALUOp = 3;
                break;
            default:
                return 1;
        }

        // calling alu to execute alu
        ALU(data1, data2, ALUOp, ALUresult, Zero);
        return 0;
    } else {
        switch (ALUOp) {
            case 0:
                // addi, storeword, jump, load word
            case 1:
                // beq
            case 2:
                // stl signed
            case 3:
                // stl unsigned
            case 6:
                // load upper immediate
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            default:
                return 1;
        }
    }
}

/* Read / Write Memory */

/*
Function Behavior

Inputs:
    ALUresult: The memory address to read/write.
    data2: The data to write to memory (used if MemWrite is asserted).
    MemWrite: Indicates if a memory write should occur.
    MemRead: Indicates if a memory read should occur.
    Mem: The memory array.
    memdata: Pointer to store the read memory value (used if MemRead is asserted).

Checks:
    Ensure ALUresult is word-aligned (ALUresult % 4 == 0).
    Ensure the address is within bounds (0 to the size of memory).

Operations:
    If MemWrite is asserted, write data2 to the memory at ALUresult.
    If MemRead is asserted, read memory at ALUresult into memdata.

Return Value:
    Return 1 if a halt condition occurs (e.g., misaligned address).
    Return 0 otherwise.
*/
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
/*Function Behavior

Inputs:
    r2, r3: Register indices to determine the destination register.
    memdata: Data read from memory (if MemtoReg is asserted).
    ALUresult: Data from the ALU operation.
    RegWrite: Control signal indicating whether a register write occurs.
    RegDst: Control signal to select the destination register (R-type vs I-type).
    MemtoReg: Control signal to select the data source (memory vs ALU result).
    Reg: The register array.

Operation:
    Check if RegWrite is enabled.
    Determine the destination register:
        If RegDst == 1: Use r3 (R-type instruction).
        If RegDst == 0: Use r2 (I-type instruction).
    Determine the data to write:
        If MemtoReg == 1: Use memdata (data from memory).
        If MemtoReg == 0: Use ALUresult (data from the ALU).
    Write the data to the determined destination register.

Return Value:
    This function doesn't return a value; it directly modifies the register array.*/
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

        //Ensuring the destination register is within valid bounds
        if (destination >= 0 && destination < 32)
        {
            //Determine the data to write
            if (MemtoReg)
            {
                data = memdata; //The Data from memory
            } else
            {
                data = ALUresult; //The data from ALU
            }

            //Writeing the data to the register file
            Reg[destination] = data;
        } else
        {             //Handling invalid destination register (optional logging)
            printf("Error: Attempt to write to an invalid register (%u).\n", destination);
        }
    }
}

/* PC update */
/*Function Behavior
Inputs:
    jsec: The jump target (26-bit address).
    extended_value: The sign-extended offset for branching.
    Branch: Control signal for branch instructions.
    Jump: Control signal for jump instructions.
    Zero: ALU zero flag (used for conditional branching).
    PC: Pointer to the program counter.

Operation:
    If Jump is 1, set PC to the jump address.
    If Branch is 1 and Zero is 1, add the extended_value (offset) to the current PC + 4.
     Otherwise, increment PC by 4 (default behavior).*/ 
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    //This is the default behavior, it increment PC by 4
    *PC += 4;

    //Handling the Jump instruction
    if (Jump)
    {
        *PC = (jsec << 2) | (*PC & 0xF0000000); //Combining the jump address with upper PC bits
    }

    // Handle Branch instruction
    if (Branch && Zero) {
        *PC += (extended_value << 2); // Add the branch offset (already sign-extended and shifted)
    }
}

