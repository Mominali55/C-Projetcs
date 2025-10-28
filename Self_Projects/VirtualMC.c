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