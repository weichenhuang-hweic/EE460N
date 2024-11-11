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

void set_interrupts();
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

// lab4
int LATCH_SP;
int LATCH_SPMUX;

// lab 5
int LATCH_MDRVALUEMUX;

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
    COND2,
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
    DRMUX1,
    DRMUX0,
    SR1MUX1,
    SR1MUX0,
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
    /* MODIFY: you have to add all your new control signals */
    LD_PSR,
    LD_USP,
    LD_SSP,
    LD_VECTOR,
    GatePSR,
    GateUSP,
    GateSSP,
    GateSP,
    GateVector,
    GateOldPc,
    SPMUX,
    ResetInt,
    ResetEXC,
    LD_VA,
    GateVA,
    LD_TEMPPSR,
    GateTEMPPSR,
    GatePTBR,
    MDRINMUX1,
    MDRINMUX0,
    MDRVALUEMUX,
    LD_SAVESTATE,
    TRANS,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x) { return (x[IRD]); }
int GetCOND(int *x) { return ((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
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
int GetDRMUX(int *x) { return ((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x) { return ((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x) { return (x[ADDR1MUX]); }
int GetADDR2MUX(int *x) { return ((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x) { return (x[MARMUX]); }
int GetALUK(int *x) { return ((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x) { return (x[MIO_EN]); }
int GetR_W(int *x) { return (x[R_W]); }
int GetDATA_SIZE(int *x) { return (x[DATA_SIZE]); }
int GetLSHF1(int *x) { return (x[LSHF1]); }
/* MODIFY: Lab 4 */
int GetLD_PSR(int *x) { return (x[LD_PSR]); }
int GetLD_USP(int *x) { return (x[LD_USP]); }
int GetLD_SSP(int *x) { return (x[LD_SSP]); }
int GetLD_VECTOR(int *x) { return (x[LD_VECTOR]); }
int GetGate_PSR(int *x) { return (x[GatePSR]); }
int GetGate_USP(int *x) { return (x[GateUSP]); }
int GetGate_SSP(int *x) { return (x[GateSSP]); }
int GetGate_SP(int *x) { return (x[GateSP]); }
int GetGate_VECTOR(int *x) { return (x[GateVector]); }
int GetGate_OldPc(int *x) { return (x[GateOldPc]); }
int GetSPMUX(int *x) { return (x[SPMUX]); }
int GetResetInt(int *x) { return (x[ResetInt]); }
int GetResetEXC(int *x) { return (x[ResetEXC]); }
/* MODIFY: Lab 5 */
int GetLD_VA(int *x) { return (x[LD_VA]); }
int GetGate_VA(int *x) { return (x[GateVA]); }
int GetLD_TEMPPSR(int *x) { return (x[LD_TEMPPSR]); }
int GetGate_TEMPPSR(int *x) { return (x[GateTEMPPSR]); }
int GetGate_PTBR(int *x) { return (x[GatePTBR]); }
int GetMDRINMUX(int *x) { return ((x[MDRINMUX1] << 1) + x[MDRINMUX0]); }
int GetMDRVALUEMUX(int *x) { return (x[MDRVALUEMUX]); }
int GetLD_SAVESTATE(int *x) { return (x[LD_SAVESTATE]); }
int GetTRANS(int *x) { return (x[TRANS]); }

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

#define WORDS_IN_MEM 0x2000 /* 32 frames */
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

    /* For lab 4 */
    int INT;
    int INTV; /* Interrupt vector register */
    int EXC;
    int EXCV; /* Exception vector register */
    int EXCOND;
    int SSP;    /* System stack pointer */
    int USP;    /* User stack pointer */
    int PSR_XV; /* We omit PSD[14:3]*/
    int VECTOR;

    /* For lab 5 */
    int VA;   /* Temporary VA register */
    int PTBR; /* Page Table Base Register */
    int SAVE_STATE;
    int TEMPPSR;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

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
    // Lab 4: Manually Set Interrupt
    if (CYCLE_COUNT == 300) {
        set_interrupts();
    }

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

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
void load_program(char *program_filename, int is_virtual_base) {
    FILE *prog;
    int ii, word, program_base, pte, virtual_pc;

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

    if (is_virtual_base) {
        if (CURRENT_LATCHES.PTBR == 0) {
            printf("Error: Page table base not loaded %s\n", program_filename);
            exit(-1);
        }

        /* convert virtual_base to physical_base */
        virtual_pc = program_base << 1;
        pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
              MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

        printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
        if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
            program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
            printf("physical base of program: %x\n\n", program_base);
            program_base = program_base >> 1;
        } else {
            printf("attempting to load a program into an invalid (non-resident) page\n\n");
            exit(-1);
        }
    } else {
        /* is page table */
        CURRENT_LATCHES.PTBR = program_base << 1;
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
        ;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
        CURRENT_LATCHES.PC = virtual_pc;

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
void initialize(char *argv[], int num_prog_files) {
    int i;
    init_control_store(argv[1]);

    init_memory();
    load_program(argv[2], 0);
    for (i = 0; i < num_prog_files; i++) {
        load_program(argv[i + 3], 1);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int) * CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000;  /* Initial value of system stack pointer */
    CURRENT_LATCHES.USP = 0xFE00;  /* Initial value of user stack pointer */
    CURRENT_LATCHES.PSR_XV = 1;    /* Initial value of PSR[15] */
    CURRENT_LATCHES.PTBR = 0x1000; /* Initial value of page table base register */

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
    if (argc < 4) {
        printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 3);

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

void set_interrupts() {
    NEXT_LATCHES.INT = 1;
    NEXT_LATCHES.INTV = 0x01;
}

void eval_micro_sequencer() {
    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */

    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int IRD_BIT = GetIRD(micro_instruction);
    int TRANS_BIT = GetTRANS(micro_instruction);
    int LD_SAVESTATE = GetLD_SAVESTATE(micro_instruction);
    int COND_BIT = GetCOND(micro_instruction);
    int J_BIT = GetJ(micro_instruction);

    if (TRANS_BIT == 1 && LD_SAVESTATE == 0) {
        int READ_WRITE = GetR_W(micro_instruction);
        if (READ_WRITE) {
            NEXT_LATCHES.STATE_NUMBER = 0b110111;
        } else {
            NEXT_LATCHES.STATE_NUMBER = 0b111001;
        }
        NEXT_LATCHES.SAVE_STATE = J_BIT;
    } else if (TRANS_BIT == 0 && LD_SAVESTATE == 1) {
        NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.SAVE_STATE;
    } else if (IRD_BIT) {
        int IR = CURRENT_LATCHES.IR;
        NEXT_LATCHES.STATE_NUMBER = 0b000000 + ((IR & 0xF000) >> 12); // 0,0,IR[15:12]
    } else {
        NEXT_LATCHES.STATE_NUMBER =
            (J_BIT & 0b010000) +
            ((J_BIT & 0b100000) | (((COND_BIT & 0b100) >> 2) & ((COND_BIT & 0b010) >> 1) & ~(COND_BIT & 0b001) & ((CURRENT_LATCHES.PSR_XV))) << 4) +
            ((J_BIT & 0b001000) | (((COND_BIT & 0b100) >> 2) & (~(COND_BIT & 0b010) >> 1) & (COND_BIT & 0b001) & (CURRENT_LATCHES.INT)) << 3) +
            ((J_BIT & 0b000100) | ((~(COND_BIT & 0b100) >> 2) & ((COND_BIT & 0b010) >> 1) & ~(COND_BIT & 0b001) & CURRENT_LATCHES.BEN) << 2) +
            ((J_BIT & 0b000010) | ((~(COND_BIT & 0b100) >> 2) & (~(COND_BIT & 0b010) >> 1) & (COND_BIT & 0b001) & CURRENT_LATCHES.READY) << 1) +
            ((J_BIT & 0b000001) | ((~(COND_BIT & 0b100) >> 2) & ((COND_BIT & 0b010) >> 1) & (COND_BIT & 0b001) & ((CURRENT_LATCHES.IR & 0x0800) >> 11)));
    }

    // Unknown State Exception
    if (NEXT_LATCHES.STATE_NUMBER == 0b001010 || NEXT_LATCHES.STATE_NUMBER == 0b001011) {
        NEXT_LATCHES.EXC = 1;
        NEXT_LATCHES.EXCV = 0x05;
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
            NEXT_LATCHES.READY = 0;
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
                        MEMORY[CURRENT_LATCHES.MAR >> 1][1] = Low16bits((CURRENT_LATCHES.MDR & 0xff00) >> 8);
                    } else {
                        MEMORY[CURRENT_LATCHES.MAR >> 1][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00ff);
                    }
                }
            } else {
                // read
                if (LD_MDR) {
                    if (DATA_SIZE) {
                        LATCH_MDR = Low16bits((MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8) + MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
                    } else {
                        if (CURRENT_LATCHES.MAR & 0x0001) {
                            LATCH_MDR = Low16bits(MEMORY[CURRENT_LATCHES.MAR >> 1][1]);
                        } else {
                            LATCH_MDR = Low16bits(MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
                        }
                    }
                }
            }
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

    // SR1MUX
    int SR1MUX = GetSR1MUX(micro_instruction);
    if (SR1MUX >= 2) {
        LATCH_SR1MUX = CURRENT_LATCHES.REGS[6];
    } else if (SR1MUX == 1) {
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
    int ALUK = GetALUK(micro_instruction);
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
    int L_R_SHF = (IR & 0x0010) >> 4;
    int R_A_L_SHF = (IR & 0x0020) >> 5;
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
        LATCH_ADDR1MUX = LATCH_SR1MUX;
    } else {
        LATCH_ADDR1MUX = CURRENT_LATCHES.PC;
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

    // Omit PCMUX Here, put in latch_datapath_values()

    // MARMUX
    int MARMUX = GetMARMUX(micro_instruction);
    if (MARMUX) {
        LATCH_MARMUX = LATCH_ADDER;
    } else {
        LATCH_MARMUX = (0x0000 | (IR & 0x00ff)) << 1;
    }

    // SP
    LATCH_SP = Low16bits(LATCH_SR1MUX);

    // SP_MUX
    int SP_MUX = GetSPMUX(micro_instruction);
    if (SP_MUX) {
        LATCH_SPMUX = LATCH_SP + 2;
    } else {
        LATCH_SPMUX = LATCH_SP - 2;
    }

    // MDRVALUEMUX
    int MDRVALUEMUX = GetMDRVALUEMUX(micro_instruction);
    if (MDRVALUEMUX) {
        LATCH_MDRVALUEMUX = (0x0000 | (CURRENT_LATCHES.MDR & 0x3E00) | (CURRENT_LATCHES.VA & 0x01FF));
    } else {
        int DATA_SIZE = GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
        if (DATA_SIZE) {
            LATCH_MDRVALUEMUX = CURRENT_LATCHES.MDR;
        } else {
            if (CURRENT_LATCHES.MAR & 0x0001) {
                LATCH_MDRVALUEMUX = Low16bits(SEXT((CURRENT_LATCHES.MDR & 0xff00) >> 8, 8));
            } else {
                LATCH_MDRVALUEMUX = Low16bits(SEXT(CURRENT_LATCHES.MDR & 0x00ff, 8));
            }
        }
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
    int Gate_PSR = GetGate_PSR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_USP = GetGate_USP(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_SSP = GetGate_SSP(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_SP = GetGate_SP(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_VECTOR = GetGate_VECTOR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int Gate_Old_PC = GetGate_OldPc(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int GateVA = GetGate_VA(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int GateTEMPPSR = GetGate_TEMPPSR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    int GatePTBR = GetGate_PTBR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);

    if (Gate_PC + Gate_MDR + Gate_ALU + Gate_MARMUX + Gate_SHF + Gate_PSR + Gate_USP + Gate_SSP + Gate_SP + Gate_VECTOR + Gate_Old_PC > 1) {
        exit(1);
    } else {
        if (Gate_PC) {
            BUS = CURRENT_LATCHES.PC;
        } else if (Gate_MDR) {
            BUS = LATCH_MDRVALUEMUX;
        } else if (Gate_ALU) {
            BUS = LATCH_ALUMUX;
        } else if (Gate_SHF) {
            BUS = LATCH_SHFMUX;
        } else if (Gate_MARMUX) {
            BUS = LATCH_MARMUX;
        } else if (Gate_PSR) {
            BUS = Low16bits((CURRENT_LATCHES.PSR_XV << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P);
        } else if (Gate_USP) {
            BUS = CURRENT_LATCHES.USP;
        } else if (Gate_SSP) {
            BUS = CURRENT_LATCHES.SSP;
        } else if (Gate_SP) {
            BUS = LATCH_SPMUX;
        } else if (Gate_VECTOR) {
            BUS = Low16bits(Low16bits(CURRENT_LATCHES.VECTOR << 1) + 0x0200);
        } else if (Gate_Old_PC) {
            BUS = CURRENT_LATCHES.PC - 2;
        } else if (GateVA) {
            BUS = CURRENT_LATCHES.VA;
        } else if (GateTEMPPSR) {
            BUS = CURRENT_LATCHES.TEMPPSR;
        } else if (GatePTBR) {
            BUS = CURRENT_LATCHES.PTBR + Low16bits(((CURRENT_LATCHES.VA & 0xFE00) >> 9) << 1);
        } else {
            BUS = 0;
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

    int *micro_instruction = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int IR = CURRENT_LATCHES.IR;
    int LD_MDR = GetLD_MDR(micro_instruction);
    int LD_MAR = GetLD_MAR(micro_instruction);
    int LD_IR = GetLD_IR(micro_instruction);
    int LD_BEN = GetLD_BEN(micro_instruction);
    int LD_REG = GetLD_REG(micro_instruction);
    int LD_CC = GetLD_CC(micro_instruction);
    int LD_PC = GetLD_PC(micro_instruction);
    int LD_PSR = GetLD_PSR(micro_instruction);
    int LD_USP = GetLD_USP(micro_instruction);
    int LD_SSP = GetLD_SSP(micro_instruction);
    int LD_VECTOR = GetLD_VECTOR(micro_instruction);
    int RESET_INT = GetResetInt(micro_instruction);
    int RESET_EXC = GetResetEXC(micro_instruction);
    int LD_VA = GetLD_VA(micro_instruction);
    int LD_TEMPPSR = GetLD_TEMPPSR(micro_instruction);

    if (LD_MDR) {
        int MIO_EN = GetMIO_EN(micro_instruction);
        if (MIO_EN) {
            NEXT_LATCHES.MDR = LATCH_MDR;
        } else {
            int MDRINMUX = GetMDRINMUX(micro_instruction);
            int DATA_SIZE = GetDATA_SIZE(micro_instruction);
            int BUS_VALUE = -1;

            if (MDRINMUX == 0) {
                BUS_VALUE = BUS;
            } else if (MDRINMUX == 1) {
                BUS_VALUE = Low16bits(BUS | 0x0001);
            } else if (MDRINMUX == 2) {
                BUS_VALUE = Low16bits(BUS | 0x0003);
            } else {
                exit(1);
            }

            if (DATA_SIZE) {
                NEXT_LATCHES.MDR = BUS_VALUE;
            } else {
                if (CURRENT_LATCHES.MAR & 0x0001) {
                    NEXT_LATCHES.MDR = ((BUS_VALUE & 0x00ff) << 8) | (BUS_VALUE & 0x00ff);
                } else {
                    NEXT_LATCHES.MDR = BUS_VALUE;
                }
            }

            if (MDRINMUX != 0 && ((BUS_VALUE & 0x0008) >> 3) == 0 && CURRENT_LATCHES.PSR_XV == 1 && ((CURRENT_LATCHES.IR & 0xF000) >> 12) != 0b001111) {
                // protection exception
                NEXT_LATCHES.EXC = 1;
                NEXT_LATCHES.EXCV = 0x04;
                NEXT_LATCHES.STATE_NUMBER = 0b001010;
            } else if (MDRINMUX != 0 && ((BUS_VALUE & 0x0004) >> 2) == 0 && CURRENT_LATCHES.PSR_XV == 1 && ((CURRENT_LATCHES.IR & 0xF000) >> 12) != 0b001111) {
                // page fault exception
                NEXT_LATCHES.EXC = 1;
                NEXT_LATCHES.EXCV = 0x02;
                NEXT_LATCHES.STATE_NUMBER = 0b001010;
            }
        }
    }

    if (LD_MAR) {
        NEXT_LATCHES.MAR = BUS;
    }

    if (LD_IR) {
        NEXT_LATCHES.IR = BUS;
    }

    if (LD_BEN) {
        NEXT_LATCHES.BEN = (((IR & 0x0800) >> 11) & CURRENT_LATCHES.N) | (((IR & 0x0400) >> 10) & CURRENT_LATCHES.Z) | (((IR & 0x0200) >> 9) & CURRENT_LATCHES.P);
    }

    if (LD_REG) {
        int DRMUX = GetDRMUX(micro_instruction);
        int DR = DRMUX >= 2 ? 6 : (DRMUX == 1 ? 7 : ((IR & 0x0E00) >> 9));
        NEXT_LATCHES.REGS[DR] = BUS;
    }

    if (LD_CC || LD_PSR) {
        if (LD_PSR) {
            NEXT_LATCHES.N = (BUS & 0x0004) >> 2;
            NEXT_LATCHES.Z = (BUS & 0x0002) >> 1;
            NEXT_LATCHES.P = BUS & 0x0001;
        } else {
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
    }

    if (LD_PC) {
        int PCMUX = GetPCMUX(micro_instruction);
        if (PCMUX == 0) {
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
        } else if (PCMUX == 1) {
            NEXT_LATCHES.PC = BUS;
        } else if (PCMUX == 2) {
            NEXT_LATCHES.PC = LATCH_ADDER;
        } else {
            exit(1);
        }
    }

    if (LD_PSR) {
        if (LD_USP) {
            NEXT_LATCHES.PSR_XV = 0;
        } else {
            NEXT_LATCHES.PSR_XV = (BUS & 0x8000) >> 15;
        }
    }

    if (LD_USP) {
        NEXT_LATCHES.USP = LATCH_SP;
    }

    if (LD_SSP) {
        NEXT_LATCHES.SSP = LATCH_SP;
    }

    if (LD_VECTOR) {
        if (CURRENT_LATCHES.EXC) {
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.EXCV;
        } else {
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.INTV;
        }
    }

    if (RESET_INT) {
        NEXT_LATCHES.INT = 0;
    }

    if (RESET_EXC) {
        NEXT_LATCHES.EXC = 0;
    }

    if (LD_VA) {
        int DATA_SIZE = GetDATA_SIZE(micro_instruction);
        // unaligned memory access exception
        if ((BUS & 0x0001) && (DATA_SIZE)) {
            NEXT_LATCHES.EXC = 1;
            NEXT_LATCHES.EXCV = 0x03;
            NEXT_LATCHES.STATE_NUMBER = 0b001010;
        } else {
            NEXT_LATCHES.VA = BUS;
        }
    }

    if (LD_TEMPPSR) {
        NEXT_LATCHES.TEMPPSR = BUS;
    }
}
