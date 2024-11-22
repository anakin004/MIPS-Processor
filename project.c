#include "spimcore.h"


/* ALU */
/*
simple alu functinality,
we switch the alucontrol to see which operation we do
the operations are commented below
*/
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch((int)ALUControl)
    {
        case 0: // A+B
            *ALUresult = A+B;
            break;
        
        case 1: // A-B
            *ALUresult = A-B;
            break;
        
        case 2: // A < B
            *ALUresult = ((signed)A < (signed)B) ? 1 : 0;
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
/*
we check if the pc is divisible by 4 ( its aligned ) if its not then we have an error
else we can read the instruction from memory
we shift left by 2 being dividing by 4 and getting the correct location
*/
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{

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
    *op = (instruction >> 26) & 0x3f;

    // last 12 bits but we & with 0000 0000 0000 0000 0000 0000 0001 1111
    // to get the 5 bits we want
    *r1 = (instruction >> 21) & 0x1F;

    // keeping last 5 bits of the shift
    *r2 = (instruction >> 16) & 0x1F;

    // keeping last 5 bits of shift
    *r3 = (instruction >> 11) & 0x1F;

    // keeping last 6 bits after shift
    *funct = instruction & 0x3F;

    // keeping last 16 bits 
    *offset = instruction & 0xFFFF;

    // keep last 26 bits
    *jsec = instruction & 0x03FFFFFF;

}




/* instruction decode */
/*
based on the opcode we will set the control for each function
*/
int instruction_decode(unsigned op,struct_controls *controls){
        switch(op){
        

        // if its an r type funtion
		case 0: 
        	    controls->MemtoReg = 0;
		    controls->ALUOp = 7;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 1;
		    controls->RegDst = 1;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;
		
               //jump
		case 2: 
        	    controls->MemtoReg = 0;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 0;
		    controls->RegDst = 0;
		    controls->Jump = 1;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;
		//beq
		case 4:  
        	    controls->MemtoReg = 2;
		    controls->ALUOp = 1;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 0;
		    controls->RegDst = 2;
		    controls->Jump = 0;
		    controls->Branch = 1;
		    controls->MemRead = 0;
		    break;

     		   // sw
      		case 43:
        	    controls->MemtoReg = 2;
		    controls->ALUOp = 0;
		    controls->MemWrite = 1;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 0;
		    controls->RegDst = 2;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;
        
       		 // lw
       		case 35:
        	    controls->MemtoReg = 1;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 1;
		    break;


		 //stli unsigned
		case 11: 
                    controls->MemtoReg = 0;
		    controls->ALUOp = 3;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;

		//load upp i
		case 15: 
        	    controls->MemtoReg = 0;
		    controls->ALUOp = 6;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;

        	//addi
		case 8:
        	    controls->MemtoReg = 0;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;

		//stli
		case 10:  
        	    controls->MemtoReg = 0;
		    controls->ALUOp = 2;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    break;

		default: // return 1 to halt if invalid
		    return 1;

	    }
    
    return 0;
}

/* Read Register */
/*
not too much commenting needed, 
just simply reading register data,
and putting into data1 and data2
*/
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
    if((offset >> 15)==1)
        *extended_value = offset | 0xFFFF0000;
    else
        *extended_value = offset & 0x0000FFFF;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero){
   	//Check which data we are opperating on by ALU src
	if(ALUSrc == 1){
		data2 = extended_value;
	}

	//AluOP being 7 is R type insturction which requires use of funct
	if(ALUOp == 7){
		//finding the proper ALUOp for each R type instruction
		switch(funct) {

			//Add
			case 32:
					ALUOp = 0;
					break;
			//Sub
			case 34:
					ALUOp = 1;
					break;
			//Set Less Signed
			case 42:
					ALUOp = 2;
					break;
			//Set Less Unsigned
			case 43:
					ALUOp = 3;
					break;
			//And
			case 36:
					ALUOp = 4;
					break;
			//Or
			case 37:
					ALUOp = 5;
					break;
			//Shift Left extended value 16
			case 6:
					ALUOp = 6;
					break;
			//Nor
			case 39:
					ALUOp = 7;
					break;
			//Halt not proper funct
			default:
					return 1;

		}
		//Send to ALU for funct
		ALU(data1,data2,ALUOp,ALUresult,Zero);

	}


	else{
		switch(ALUOp){
		    case 0:
	                // addi, sw, jump, lw
	            case 1:
	                //beq
	            case 2:
	                // slti
	            case 3:
	                // sltiu
	            case 6:
	                //lui
			break;
	            default:
	                return 1;
		}
		//Send to ALU for non funct
		ALU(data1,data2,ALUOp,ALUresult,Zero);
	}

	//return normally
	return 0;
}

/* Read / Write Memory */

/*
ALUresult: The memory address to read/write.
data2: The data to write to memory if we write to memory
memdata is a pointer to store the read memory value
*/

int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

    if (MemWrite)
    //Perform memory write
    {
        if (ALUresult % 4 != 0)
        //checking if the address is word-aligned
        {
            return 1; // Halt due to misaligned address
        }
        Mem[ALUresult >> 2] = data2;
        // write data2 to the memory
    }

    if (MemRead)
    { //preform memory read
        if (ALUresult % 4 != 0 )
        //checking if the address is word-aligned
        {
            return 1; // halt due to misaligned address
        }

        *memdata = Mem[ALUresult >> 2];
        //read from memory into memdata
    }

    return 0;
}



/* Write Register 
r2 and r3 are register indices, and memdata is data read from memory 
ALUresult - the Data from the ALU operation
RegWrite is the Control signal indicating whether a register write occurs
RegDst is control signal to select the destination register  - r-type vs itype

*/
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if(RegWrite == 1){
		 // if we want to write to memory and regdst
         // then we want to write to r2 reg
		if (MemtoReg == 1 && RegDst == 0) {
			Reg[r2] = memdata;
		}
        // other case when RegDst == 1 we want to write to third register
		else if(MemtoReg == 1 && RegDst == 1){
			 Reg[r3] = memdata;
		}

		 // if we dont want to write to memory we are writing to register
		else if (MemtoReg == 0 && RegDst == 0) {
			Reg[r2] = ALUresult;
		}

		else if (MemtoReg == 0 && RegDst == 1){
			Reg[r3] = ALUresult;
		}
	}
}

/* PC update 
some are self documenting, 

inputs ,jsec: The jump target , extended_value - The sign-extended offset
pc - Pointer to the program counter.
*/

void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    //this is the default behavior, it increment PC by 4
    *PC += 4;

    if (Branch && Zero) {
        *PC += (extended_value << 2); // add the branch offset
    }

    //handling the Jump instruction
    if (Jump)
    {
        *PC = (jsec << 2) | (*PC & 0xF0000000); //combining the jump address with upper PC bits
    }
}
