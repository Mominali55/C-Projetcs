/*
Creating a Virtual machine in C
We are using LC-3b architecture as reference 
-This is a 16-bit architecture
*/

// For windows compatibility
#include <stdio.h>
#include <stdint.h> //for uint8_t, uint16_t, uint32_t (For register,memory,opcodes)
#include <signal.h>  //Interrupt handling, graceful shutdown
/* windows only */
#include <Windows.h> //Console I/O, memory management, timing
#include <conio.h>  // _kbhit 

//creating Memory storage
#define MEMORY_SIZE (1 << 16) //2^16 locations
uint16_t memory[MEMORY_SIZE]; //Each location is 16 bits wide

//Registers
enum {
    R_R0, //optional to define first item as 0
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, //Program Counter
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT]; //Each of size 16 bits

enum {
    OP_BR, /* Branch */
    OP_ADD, /* Add */
    OP_LD, /* Load */
    OP_ST, /* Store */
    OP_JSR, /* Jump to Subroutine */
    OP_AND, /* Bitwise AND */
    OP_LDR, /* Load Register */
    OP_STR, /* Store Register */
    OP_RTI, /* Return from Interrupt */
    OP_NOT, /* Bitwise NOT */
    OP_LDI, /* Load Indirect */
    OP_STI, /* Store Indirect */
    OP_JMP, /* Jump */
    OP_RES, /* Reserved */
    OP_LEA, /* Load Effective Address */
    OP_TRAP /* Trap */
};

// Condition Flags
enum {
    FL_POS = 1 << 0, // P (bit 0: 0b001)
    FL_ZRO = 1 << 1, // Z (bit 1: 0b010)
    FL_NEG = 1 << 2  // N (bit 2: 0b100)
};

//We have Finished settitng up the Hardware components of our VM

int main(int argc, char *argv[]) //Taking command line argumnets
{
    //Setup signal handlers
    signal(SIGINT,handle_interrupt);
    disable_input_buffering();

    //load argumnets

    //Checking for Wrong Format or Insufficient arguments
    if (argc < 2)
    {
        /*show usage string*/
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for(int i=1;i<argc;i++)
    {
        if(!read_image(argv[i]))
        {
            printf("Failed to load image: %s\n",argv[i]);
            exit(1);
        }
    }

    /*Since exactly one condition flag should be set at any given time,set the Z Flag*/
    reg[R_COND] = FL_ZRO; //Using the Conditional flags from "enum" above

    /*set the Pc to starting position*/
    /*0x3000 is the default*/
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START; //using the Program counter from "enum" above

    int running =1;
    while (running)
    {
        /*FETCH*/
        uint16_t instr =mem_read(reg[R_PC]++);  //important to post increment the PC **
        uint16_t op = instr >> 12; //top 4 bits of the instruction

        switch (op)
        {
        case OP_ADD:
            {
                /*destination register(DR)*/
                uint16_t r0 =(instr >> 9) & 0x7;
                /*first operand (SR1)*/
                uint16_t r1 =(instr >> 6) & 0x7;
                /*whether to use immediate mode*/
                uint16_t imm_flag =(instr >> 5) & 0x1;

                if(imm_flag)
                {
                    uint16_t imm5 =sign_extend(instr & 0x1F ,5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    uint16_t r2 =instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                update_flags(r0);
            }
            break;
       case OP_AND:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1f ,5);
                    reg[r0] = reg[r1] & imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7; //wht is the purpose?
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
            }
            break;
        case OP_NOT:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;

            reg[r0] = ~reg[r1]; //why use ~ here?
            update_flags(r0);
        }
        break;

        case OP_BR:
            {
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & reg[R_COND])
                {
                    reg[R_PC] += pc_offset;
                }
            }
            break;
        case OP_JMP:
            {
                uint16_t base_r = (instr >> 6) & 0x7; //#changed..
                reg[R_PC] = reg[base_r];
            }
            break;
        case OP_JSR:
            {
                uint16_t long_flag = (instr >> 11) & 1;
                reg[R_R7] = reg[R_PC];
                if (long_flag)
                {
                    uint16_t pc_offset = sign_extend(instr & 0x7FF,11);
                    reg[R_PC] += pc_offset;
                }
                else
                {
                    uint16_t base_r = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[base_r];
                }
            }
            break;
        case OP_LD:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9); //Pc offset is getting owerwritten here again and again
                reg[r0] = mem_read(reg[R_PC] + pc_offset); //Usually this two are always together?
                update_flags(r0);
            }
            break;
        
        case OP_LDI:
            {
                //Destination Register
                uint16_t r0 = (instr >> 9) & 0x7;
                //PC offset
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
                //add pc_offset to the current PC, look at that memory location to get the final address
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flags(r0);
            }
            break;      
        default:
            break;
        }
    }

}