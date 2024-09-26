/*
    Name 1: Wei-Chen Huang
    UTEID 1: wh9442
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

// Global Variable
int MEMORY_COUNT;
int LATCH_SR1MUX;
int LATCH_SR2MUX;
int LATCH_ALUMUX;
int LATCH_SHFMUX;
int LATCH_ADDR1MUX;
int LATCH_ADDR2MUX;
int LATCH_LSHF;
int LATCH_ADDER;
int LATCH_PCMUX;
int LATCH_MARMUX;
int LATCH_MDR;

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1,
    COND0,
    J5,
    J4,
    J3,
    J2,
    J1,
    J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1,
    PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1,
    ADDR2MUX0,
    MARMUX,
    ALUK1,
    ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x) { return (x[IRD]); }
int GetCOND(int *x) { return ((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x) { return ((x[J5] << 5) + (x[J4] << 4) +
                           (x[J3] << 3) + (x[J2] << 2) +
                           (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x) { return (x[LD_MAR]); }
int GetLD_MDR(int *x) { return (x[LD_MDR]); }
int GetLD_IR(int *x) { return (x[LD_IR]); }
int GetLD_BEN(int *x) { return (x[LD_BEN]); }
int GetLD_REG(int *x) { return (x[LD_REG]); }
int GetLD_CC(int *x) { return (x[LD_CC]); }
int GetLD_PC(int *x) { return (x[LD_PC]); }
int GetGATE_PC(int *x) { return (x[GATE_PC]); }
int GetGATE_MDR(int *x) { return (x[GATE_MDR]); }
int GetGATE_ALU(int *x) { return (x[GATE_ALU]); }
int GetGATE_MARMUX(int *x) { return (x[GATE_MARMUX]); }
int GetGATE_SHF(int *x) { return (x[GATE_SHF]); }
int GetPCMUX(int *x) { return ((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x) { return (x[DRMUX]); }
int GetSR1MUX(int *x) { return (x[SR1MUX]); }
int GetADDR1MUX(int *x) { return (x[ADDR1MUX]); }
int GetADDR2MUX(int *x) { return ((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x) { return (x[MARMUX]); }
int GetALUK(int *x) { return ((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x) { return (x[MIO_EN]); }
int GetR_W(int *x) { return (x[R_W]); }
int GetDATA_SIZE(int *x) { return (x[DATA_SIZE]); }
int GetLSHF1(int *x) { return (x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM 0x08000
#define MEM_CYCLES 5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT; /* run bit */
int BUS;     /* value of the bus */

typedef struct System_Latches_Struct {

    int PC,  /* program counter */
        MDR, /* memory data register */
        MAR, /* memory address register */
        IR,  /* instruction register */
        N,   /* n condition bit */
        Z,   /* z condition bit */
        P,   /* p condition bit */
        BEN; /* ben register */

    int READY; /* ready bit */
               /* The ready bit is also latched as you dont want the memory system to assert it
                  at a bad point in the cycle*/

    int REGS[LC_3b_REGS]; /* register file. */

    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

    int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    eval_micro_sequencer();
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();

    CURRENT_LATCHES = NEXT_LATCHES;

    CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }

    /* Read a line for each row in the control store. */
    for (i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                   i, ucode_filename);
            exit(-1);
        }

        /* Put in bits one at a time. */
        index = 0;

        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                       ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                       ucode_filename, i, j);
                exit(-1);
            }

            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0 : 1;
            index++;
        }

        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for (i = 0; i < num_prog_files; i++) {
        load_program(program_filename);
        while (*program_filename++ != '\0')
            ;
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int) * CONTROL_STORE_BITS);

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
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

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

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

int SEXT(int imme, int digit) {
    if ((imme >> (digit - 1)) & 0b1) {
        return Low16bits(imme | (0xFFFF << digit));
    }
    return Low16bits(imme);
}

void eval_micro_sequencer() {

    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */

    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int IRD_BIT = GetIRD(micro_instruction);
    int COND_BIT = GetCOND(micro_instruction);
    int J_BIT = GetJ(micro_instruction);

    if (IRD_BIT) {
        int IR = CURRENT_LATCHES.IR;
        NEXT_LATCHES.STATE_NUMBER = 0b000000 + IR & 0xF000 >> 12; // 0,0,IR[15:12]
    } else {
        NEXT_LATCHES.STATE_NUMBER =
            (J_BIT & 0b111000) +
            ((J_BIT & 0b000100) | ((COND_BIT & 0b10) & ~(COND_BIT & 0b01) & CURRENT_LATCHES.BEN)) +
            ((J_BIT & 0b000010) | (~(COND_BIT & 0b10) & (COND_BIT & 0b01) & CURRENT_LATCHES.READY)) +
            ((J_BIT & 0b000001) | ((COND_BIT & 0b10) & (COND_BIT & 0b01) & (CURRENT_LATCHES.IR & 0x0800)));
    }
}

void cycle_memory() {

    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */

    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int MIO_EN = GetMIO_EN(micro_instruction);
    int COND_BIT = GetCOND(micro_instruction);

    if (COND_BIT & 0b01 && MIO_EN) {
        if (CURRENT_LATCHES.READY == 0) {
            MEMORY_COUNT++;
            if (MEMORY_COUNT == MEM_CYCLES - 1) {
                NEXT_LATCHES.READY = 1;
                MEMORY_COUNT = 0;
            }
        } else {
            int R_W = GetR_W(micro_instruction);
            int DATA_SIZE = GetDATA_SIZE(micro_instruction);
            int LD_MDR = GetLD_MDR(micro_instruction);

            if (R_W) {
                // write
                if (DATA_SIZE) {
                    MEMORY[CURRENT_LATCHES.MAR >> 1][1] = Low16bits((CURRENT_LATCHES.MDR & 0xff00) >> 8);
                    MEMORY[CURRENT_LATCHES.MAR >> 1][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00ff);
                } else {
                    if (CURRENT_LATCHES.MAR & 0x0001) {
                        MEMORY[CURRENT_LATCHES.MAR >> 1][1] = NEXT_LATCHES.MDR & 0x00ff;
                    } else {
                        MEMORY[CURRENT_LATCHES.MAR >> 1][0] = NEXT_LATCHES.MDR & 0x00ff;
                    }
                }
            } else {
                // read
                if (LD_MDR) {
                    // should we consider KBDR, KBSR, DSDR, DSR?
                    if (DATA_SIZE) {
                        NEXT_LATCHES.MDR = Low16bits((MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8) + MEMORY[CURRENT_LATCHES.MAR][0]);
                    } else {
                        if (CURRENT_LATCHES.MAR & 0x0001) {
                            NEXT_LATCHES.MDR = Low16bits(MEMORY[CURRENT_LATCHES.MAR >> 1][1]);
                        } else {
                            NEXT_LATCHES.MDR = Low16bits(MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
                        }
                    }
                }
            }

            LATCH_MDR = NEXT_LATCHES.MDR;
        }
    }
}

void eval_bus_drivers() {

    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *       Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int IR = CURRENT_LATCHES.IR;

    // CONTROL
    int STEEERBIT = (IR & 0x0020) >> 5;
    int ALUK = (IR & 0xC000) >> 14;

    // SR1MUX
    int SR1MUX = GetSR1MUX(micro_instruction);
    if (SR1MUX) {
        LATCH_SR1MUX = CURRENT_LATCHES.REGS[(IR & 0x01C0) >> 6];
    } else {
        LATCH_SR1MUX = CURRENT_LATCHES.REGS[(IR & 0x0E00) >> 9];
    }

    // SR2MUX
    if (STEEERBIT) {
        LATCH_SR2MUX = SEXT(IR & 0x001F, 5);
    } else {
        LATCH_SR2MUX = CURRENT_LATCHES.REGS[IR & 0x0007];
    }

    // ALUMUX
    if (ALUK == 0) {
        // ADD
        LATCH_ALUMUX = Low16bits(LATCH_SR1MUX + LATCH_SR2MUX);
    } else if (ALUK == 1) {
        // AND
        LATCH_ALUMUX = Low16bits(LATCH_SR1MUX & LATCH_SR2MUX);
    } else if (ALUK == 2) {
        // XOR
        LATCH_ALUMUX = Low16bits(LATCH_SR1MUX ^ LATCH_SR2MUX);
    } else if (ALUK == 3) {
        // PASS_A
        LATCH_ALUMUX = Low16bits(LATCH_SR1MUX);
    } else {
        exit(1);
    }

    // SHFMUX
    int L_R_SHF = IR & 0x0010;
    int R_A_L_SHF = IR & 0x0020;
    int amount4 = IR & 0x000f;
    if (L_R_SHF) {
        if (R_A_L_SHF) {
            // RSHFA
            LATCH_SHFMUX = LATCH_SR1MUX >> amount4;
            if (LATCH_SR1MUX & 0x8000) {
                LATCH_SHFMUX = Low16bits(LATCH_SHFMUX | 0xffff << (16 - amount4));
            } else {
                LATCH_SHFMUX = Low16bits(LATCH_SHFMUX);
            }
        } else {
            // RSHFL
            LATCH_SHFMUX = Low16bits(LATCH_SR1MUX >> amount4);
        }
    } else {
        // LSHF
        LATCH_SHFMUX = Low16bits(LATCH_SR1MUX << amount4);
    }

    // ADDR1MUX
    int ADDR1MUX = GetADDR1MUX(micro_instruction);
    if (ADDR1MUX) {
        LATCH_ADDR1MUX = CURRENT_LATCHES.PC;
    } else {
        LATCH_ADDR1MUX = LATCH_SR1MUX;
    }

    // ADDR2MUX
    int ADDR2MUX = GetADDR2MUX(micro_instruction);
    if (ADDR2MUX == 0) {
        LATCH_ADDR2MUX = 0;
    } else if (ADDR2MUX == 1) {
        LATCH_ADDR2MUX = Low16bits(SEXT(IR & 0x003f, 6));
    } else if (ADDR2MUX == 2) {
        LATCH_ADDR2MUX = Low16bits(SEXT(IR & 0x01ff, 9));
    } else if (ADDR2MUX == 3) {
        LATCH_ADDR2MUX = Low16bits(SEXT(IR & 0x07ff, 11));
    } else {
        exit(1);
    }

    // LSHF
    int LSHF = GetLSHF1(micro_instruction);
    if (LSHF) {
        LATCH_LSHF = Low16bits(LATCH_ADDR2MUX << 1);
    } else {
        LATCH_LSHF = LATCH_ADDR2MUX;
    }

    // ADDER
    LATCH_ADDER = Low16bits(LATCH_LSHF + LATCH_ADDR1MUX);

    // PCMUX
    int PCMUX = GetPCMUX(micro_instruction);
    if (PCMUX == 0) {
        LATCH_PCMUX = CURRENT_LATCHES.PC + 2;
    } else if (PCMUX == 1) {
        LATCH_PCMUX = BUS;
    } else if (PCMUX == 2) {
        LATCH_PCMUX = LATCH_ADDER;
    } else {
        exit(1);
    }

    // MARMUX
    int MARMUX = GetMARMUX(micro_instruction);
    if (MARMUX) {
        LATCH_MARMUX = LATCH_ADDER;
    } else {
        LATCH_MARMUX = (0x0000 | (IR & 0x00ff)) << 1;
    }
}

void drive_bus() {

    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */

    int Gate_PC = GetGATE_PC(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_MDR = GetGATE_MDR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_ALU = GetGATE_ALU(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_MARMUX = GetGATE_MARMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_SHF = GetGATE_SHF(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);

    if (Gate_PC + Gate_MDR + Gate_ALU + Gate_MARMUX + Gate_SHF > 1) {
        exit(1);
    } else {
        if (Gate_PC) {
            BUS = LATCH_PCMUX;
        } else if (Gate_MDR) {
            BUS = LATCH_MDR;
        } else if (Gate_ALU) {
            BUS = LATCH_ALUMUX;
        } else if (Gate_SHF) {
            BUS = LATCH_SHFMUX;
        } else if (Gate_MARMUX) {
            BUS = LATCH_MARMUX;
        } else {
            exit(1);
        }
    }
}

void latch_datapath_values() {

    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come
     * after drive_bus.
     */

    // omit LD_MDR, we finished this in cycle_memory()
    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int IR = CURRENT_LATCHES.IR;
    int LD_MAR = GetLD_MAR(micro_instruction);
    int LD_IR = GetLD_IR(micro_instruction);
    int LD_BEN = GetLD_BEN(micro_instruction);
    int LD_REG = GetLD_REG(micro_instruction);
    int LD_CC = GetLD_CC(micro_instruction);
    int LD_PC = GetLD_PC(micro_instruction);

    if (LD_MAR) {
        NEXT_LATCHES.MAR = BUS;
    }

    if (LD_IR) {
        NEXT_LATCHES.IR = BUS;
    }

    if (LD_BEN) {
        NEXT_LATCHES.BEN = (IR & 0x0800 & CURRENT_LATCHES.N) | (IR & 0x0400 & CURRENT_LATCHES.Z) | (IR & 0x0200 & CURRENT_LATCHES.P);
    }

    if (LD_REG) {
        int DRMUX = GetDRMUX(micro_instruction);
        int DR = DRMUX ? 7 : ((IR & 0x0E00) >> 9);
        NEXT_LATCHES.REGS[DR] = BUS;
    }

    if (LD_CC) {
        if (BUS == 0) {
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.P = 0;
        } else if (BUS & 0x8000) {
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.P = 0;
        } else {
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.P = 1;
        }
    }

    if (LD_PC) {
        NEXT_LATCHES.PC = LATCH_PCMUX;
    }
}