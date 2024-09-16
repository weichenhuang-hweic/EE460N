/*
    Name 1: Wei-Chen Huang
    UTEID 1: wh9442
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

// Aid Functions
int DR(int OP);
int SR1(int OP);
int SR2(int OP);
int BASER(int OP);
int IMME5(int OP);
int SEXT(int imme, int digit);
int PCOFFSET(int pcOffset, int digit);
int BOFFSET(int bOffset, int digit);
void SETCC(int value);
int GETCC();
void UPDATEPC(int PCNext);

// Instruction Functions
void ADD(int OP);
void AND(int OP);
void BR(int OP);
void JMPRET(int OP);
void JSRJSRR(int OP);
void LDB(int OP);

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE 1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM 0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT; /* run bit */

typedef struct System_Latches_Struct {

    int PC,               /* program counter */
        N,                /* n condition bit */
        Z,                /* z condition bit */
        P;                /* p condition bit */
    int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE *dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE *dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE *dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch (buffer[0]) {
    case 'G':
    case 'g':
        go();
        break;

    case 'M':
    case 'm':
        scanf("%i %i", &start, &stop);
        mdump(dumpsim_file, start, stop);
        break;

    case '?':
        help();
        break;
    case 'Q':
    case 'q':
        printf("Bye.\n");
        exit(0);

    case 'R':
    case 'r':
        if (buffer[1] == 'd' || buffer[1] == 'D')
            rdump(dumpsim_file);
        else {
            scanf("%d", &cycles);
            run(cycles);
        }
        break;

    default:
        printf("Invalid Command\n");
        break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i = 0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE *prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0)
        CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for (i = 0; i < num_prog_files; i++) {
        load_program(program_filename);
        while (*program_filename++ != '\0')
            ;
    }
    CURRENT_LATCHES.Z = 1;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE *dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

void process_instruction() {
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */

    int PC = CURRENT_LATCHES.PC;
    int lowerByte = MEMORY[PC >> 1][0];
    int upperByte = MEMORY[PC >> 1][1];
    int OP = upperByte << 8 | lowerByte;

    if (~((OP >> 12) & (0b0001))) {
        ADD(OP);
    } else if (~((OP >> 12) & (0b0101))) {
        AND(OP);
    } else if (~((OP >> 12) & (0b0000))) {
        BR(OP);
    } else if (~((OP >> 12) & (0b1100))) {
        JMPRET(OP);
    } else if (~((OP >> 12) & (0b0100))) {
        JSRJSRR(OP);
    } else if (~((OP >> 12) & (0b0010))) {
        LDB(OP);
    }
}

int DR(int OP) {
    return (OP & 0x0E00) >> 9;
}

int SR1(int OP) {
    return (OP & 0x01C0) >> 6;
}

int SR2(int OP) {
    return (OP & 0x0007);
}

int BASER(int OP) {
    return (OP & 0x01C0) >> 6;
}

int IMME5(int OP) {
    return (OP & 0x001F);
}

int SEXT(int imme, int digit) {
    if (imme >> (digit - 1)) {
        return Low16bits(imme | (0xFFFF << digit));
    }
    return Low16bits(imme);
}

int PCOFFSET(int pcOffset, int digit) {
    int nextPC = CURRENT_LATCHES.PC + 2;
    return Low16bits(nextPC + (SEXT(pcOffset, digit) << 1));
}

int BOFFSET(int bOffset, int digit) {
    return Low16bits(SEXT(bOffset, digit));
}

void SETCC(int value) {
    if (value & 0x8000) {
        NEXT_LATCHES.N = 1;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
    } else if (value ^ 0) {
        // Positives
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 1;
    } else {
        // Zero
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 1;
        NEXT_LATCHES.P = 0;
    }
}

int GETCC() {
    int condition = 0;
    if (CURRENT_LATCHES.N) {
        condition += 4;
    }
    if (CURRENT_LATCHES.Z) {
        condition += 2;
    }
    if (CURRENT_LATCHES.P) {
        condition += 1;
    }
    return Low16bits(condition);
}

void UPDATEPC(int PCNext) {
    NEXT_LATCHES.PC = PCNext;
}

void ADD(int OP) {
    int dr = DR(OP);
    int sr1 = SR1(OP);

    if (OP & 0x0020) {
        int imme5 = SEXT(IMME5(OP), 5);
        CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + imme5);
    } else {
        int sr2 = SR2(OP);
        CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]);
    }

    SETCC(CURRENT_LATCHES.REGS[dr]);
    UPDATEPC(CURRENT_LATCHES.PC + 2);
}

void AND(int OP) {
    int dr = DR(OP);
    int sr1 = SR1(OP);

    if (OP & 0x0020) {
        int imme5 = SEXT(IMME5(OP), 5);
        CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & imme5);
    } else {
        int sr2 = SR2(OP);
        CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]);
    }

    SETCC(CURRENT_LATCHES.REGS[dr]);
    UPDATEPC(CURRENT_LATCHES.PC + 2);
}

void BR(int OP) {
    int condition = (OP & 0x0E00) >> 9;
    int cc = GETCC();
    int pcOffset9 = (OP & 0x01FF);

    if ((condition ^ 0b0) || (condition & cc)) {
        UPDATEPC(PCOFFSET(pcOffset9, 9));
    } else {
        UPDATEPC(CURRENT_LATCHES.PC + 2);
    }
}

void JMPRET(int OP) {
    int baseR = BASER(OP);
    UPDATEPC(CURRENT_LATCHES.REGS[baseR]);
}

void JSRJSRR(int OP) {
    CURRENT_LATCHES.REGS[7] = CURRENT_LATCHES.PC;

    if (OP & 0x0800) {
        int pcOffset11 = (OP & 0x07FF);
        UPDATEPC(PCOFFSET(pcOffset11, 11));
    } else {
        int baseR = BASER(OP);
        UPDATEPC(CURRENT_LATCHES.REGS[baseR]);
    }
}

void LDB(int OP) {
    int dr = DR(OP);
    int baseR = BASER(OP);
    int bOffset6 = BOFFSET((OP & 0x003F), 6);

    int address = baseR + bOffset6;

    // TODO: check lower byte or high byte
    int lowerByte = MEMORY[address >> 1][0];
    int upperByte = MEMORY[address >> 1][1];

    if (OP & 0x0001) {
        CURRENT_LATCHES.REGS[dr] = SEXT(upperByte, 8);
    } else {
        CURRENT_LATCHES.REGS[dr] = SEXT(lowerByte, 8);
    }

    UPDATEPC(CURRENT_LATCHES.PC + 2);
}
