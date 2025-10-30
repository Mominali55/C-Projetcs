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


//Memory Mapped Registers

enum {
    MR_KBSR = 0xFE00, //Keyboard Status Register
    MR_KBDR = 0xFE02  //Keyboard Data Register
};

//Trap Codes
enum {
    TRAP_GETC = 0x20,  //get character from keyboard, not echoed onto the terminal
    TRAP_OUT = 0x21,   //output a character in R0 to the console
    TRAP_IN = 0x22,    //input a character from the keyboard, echoed onto the terminal
    TRAP_PUTSP = 0x23, //output a word string to the console
    TRAP_HALT = 0x25   //halt the program
};

//creating Memory storage
#define MEMORY_SIZE (1 << 16) //2^16 locations
uint16_t memory[MEMORY_SIZE]; //Each location is 16 bits wide

//Register storage
uint16_t reg[R_COUNT]; //Each of size 16 bits

//Input Buffering 
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

//Handle Interrupt
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

//Sign Extend
uint16_t sign_extend(uint16_t x, int bit_count)
{
    if((x >> (bit_count - 1)) & 1) //checking if the sign bit is 1
    {
        x |= (0xFFFF << bit_count); //extend with 1s
    }
    return x;
}

//swap
uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

//Update falgs
void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZERO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] =FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

//Read Image File
void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}
//Read Image
int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

//Memory acess functions
void mem_write (uint16_t address,uint16_t val)
{
    memory[address] =val;
}
uint16_t mem_read (uint16_t address)
{
    if(address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = (uint16_t)getchar();
        }
        else
        {
            memory[MR_KBSR] =0;
        }
    }
    return memory[address];
}

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
        case OP_LDR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F,6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
            }
            break;
        case OP_LEA:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
            }
            break;
        case OP_ST:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]); //new "mem_write" function
            }
            break;
        case OP_STI:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
            }
            break;
        case OP_STR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F,6);
                mem_write(reg[r1] + offset, reg[r0]);
            }
            break;
        //Reasearch/read thsi later
        case OP_TRAP:
            {
                reg[R_R7] = reg[R_PC]; //store return address
                switch (instr & 0xFF) //trap vector is in low order 8 bits
                {
                case TRAP_GETC:
                    {
                        //read a single character from keyboard, not echoed onto the terminal
                        reg[R_R0] = (uint16_t)_getch();
                        update_flages(R_R0);
                    }
                    break;
                case TRAP_OUT:
                    {
                        //output a character in R0 to the console
                        putc((char)reg[R_R0],stdout);
                        fflush(stdout); //New function to force output to console //Print ASCII value of R0
                    } 
                    break;
                case TRAP_IN:
                    {
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c,stdout); //echo back to console
                        fflush(stdout);
                        reg[R_R0] = (uint16_t)c;
                        update_flags(R_R0);
                    }
                    break;
                case TRAP_PUTSP:
                    {
                    /*one char per byte (two bytes per word)
                        here we need to swap back to
                        big endian format*/
                        uint16_t* c = memory + reg[R_R0];
                        while (*c)
                        {
                            char char1 = (*c) & 0xFF;
                            putc(char1,stdout);
                            char char2 = (*c) >> 8;
                            if (char2) putc(char2,stdout);
                            ++c;
                        }
                        fflush(stdout);

                    }
                    break;
                case TRAP_HALT:
                    {
                        printf("HALT\n");
                        fflush(stdout);
                        running = 0;
                    }
                    break;
                }
            }
            break;
        //why not implement the body??
        case OP_RES:
        case OP_RTI:
        default:
            abort();
            break;
        }
    }
    restore_input_buffering();

if (argc < 2)
{
    /*show usage string*/
    printf("lc3 [image-file1] ...\n");
    exit(2);
}
for(int j=1;j<argc;++j)
{
    if(!read_image(argv[j]))
    {
        printf("Failed to load image: %s\n",argv[j]);
        exit(1);
    }
}
