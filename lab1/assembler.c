/*
    Name 1: Wei-Chen Huang
    Name 2: Yue An
    UTEID 1: wh9442
    UTEID 2:ya6734
*/
// Title: Lab 1 - Part I: Write an assembler for the LC-3b Assembly Language
// Date Begin: 2024/09/02

#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */

// Syntax of Assembly Code
// LABEL OPCODE OPERANDS ; COMMENTS
// Parsing Assembly Language

#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct
{
    int address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */
#define MAX_LINE_LENGTH 255
enum BOOLEAN
{
    FALSE,
    TRUE,
};

enum PARSING_STATUS
{
    DONE,
    OK,
    EMPTY_LINE,
};

char opcodeMap[28][6] = {
    "add",
    "and",
    "brn",
    "brz",
    "brp",
    "br",
    "brzp",
    "brnp",
    "brnz",
    "brnzp",
    "halt",
    "jmp",
    "jsr",
    "jsrr",
    "ldb",
    "ldw",
    "lea",
    "nop",
    "not",
    "ret",
    "rti",
    "lshf",
    "rshfl",
    "rshfa",
    "stb",
    "stw",
    "trap",
    "xor",
};

int isOpcode(char *lPtr)
{
    if (lPtr[0] == '.')
    {
        // Pseudo Operation
        return -1;
    }
    else
    {
        for (int i = 0; i < 28; i++)
        {
            if (strncmp(lPtr, opcodeMap[i], strlen(opcodeMap[i])) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4)
{
    char *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
            return (OK);
    }

    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg4 = lPtr;

    return (OK);
}

// Convert a String To a Number
int toNum(char *pStr)
{
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;

    /* decimal */
    if (*pStr == '#')
    {
        pStr++;

        /* dec is negative */
        if (*pStr == '-')
        {
            lNeg = 1;
            pStr++;
        }

        t_ptr = pStr;
        t_length = strlen(t_ptr);

        for (k = 0; k < t_length; k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }

        lNum = atoi(pStr);
        if (lNeg)
        {
            lNum = -lNum;
        }

        return lNum;
    }
    /* hex */
    else if (*pStr == 'x')
    {
        pStr++;

        /* hex is negative */
        if (*pStr == '-')
        {
            lNeg = 1;
            pStr++;
        }

        t_ptr = pStr;
        t_length = strlen(t_ptr);

        for (k = 0; k < t_length; k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }

        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;

        if (lNeg)
        {
            lNum = -lNum;
        }

        return lNum;
    }
    else
    {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4); /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

void outputNumToHexFile(FILE *outfile, int num)
{
    fprintf(outfile, "0x%04X\n", num);
}

unsigned int drInt(char *pArg)
{
    return pArg[1] - '0';
}

unsigned int srInt(char *pArg)
{
    return drInt(pArg);
}

int pcOffset(char *pLabel, int curPC, int digit, int symbolTableCnt)
{
    int jumpPC = -1;
    for (int i = 0; i < symbolTableCnt; i++)
    {
        TableEntry te = symbolTable[i];
        if (strcmp(pLabel, te.label) == 0)
        {
            jumpPC = te.address;
            break;
        }
    }
    // TODO: handle no label found & Invalid LABEL

    int pcGap = jumpPC - (curPC + 2);
    return (pcGap >> 1) & (0xFFFF >> (16 - digit));
}

// ISA
void ADD(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0001;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (pArg3[0] == '#' || pArg3[0] == 'x' || pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void AND(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0101;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (pArg3[0] == '#' || pArg3[0] == 'x' || pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void BR(char *pOpcode, char *pArg1, int PC, int symbolTableCnt, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0000;
    unsigned int conditional = 0;

    // Conditional Code
    if (strstr(pOpcode, "n"))
    {
        conditional += 4;
    }
    if (strstr(pOpcode, "z"))
    {
        conditional += 2;
    }
    if (strstr(pOpcode, "p"))
    {
        conditional += 1;
    }
    if (strcmp(pOpcode, "br") == 0)
    {
        conditional = 7;
    }

    unsigned int pcOffset9 = pcOffset(pArg1, PC, 9, symbolTableCnt);

    op = opcode << 12 | conditional << 9 | (pcOffset9);
    outputNumToHexFile(outFile, op);
}

void JMP(char *pArg1, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1100;
    unsigned int baseR = srInt(pArg1);
    op = opcode << 12 | (0b000) << 9 | baseR << 6 | 0b0;
    outputNumToHexFile(outFile, op);
}

void JSR(char *pOpcode, char *pArg1, int PC, int symbolTableCnt, FILE *outFile)
{
    // This includes JSR & JSRR
    unsigned int op = 0;
    unsigned int opcode = 0b0100;

    if (strcmp(pOpcode, "jsrr") == 0)
    {
        unsigned int baseR = srInt(pArg1);
        op = opcode << 12 | (0b000) << 9 | baseR << 6 | 0b0;
    }
    else
    {
        unsigned int pcOffset11 = pcOffset(pArg1, PC, 11, symbolTableCnt);
        op = opcode << 12 | (0b1) << 11 | pcOffset11;
    }

    outputNumToHexFile(outFile, op);
}

// TODO: check pArg3 limit
void LDB(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0010;
    unsigned int dr = drInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int boffset6 = toNum(pArg3) & 0x3F;
    op = opcode << 12 | dr << 9 | baseR << 6 | boffset6;
    outputNumToHexFile(outFile, op);
}

// TODO: check pArg3 limit
void LDW(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0110;
    unsigned int dr = drInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int offset6 = toNum(pArg3) & 0x3F;
    op = opcode << 12 | dr << 9 | baseR << 6 | offset6;
    outputNumToHexFile(outFile, op);
}

void LEA(char *pArg1, char *pArg2, int PC, int symbolTableCnt, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1110;
    unsigned int dr = drInt(pArg1);
    unsigned int pcOffset9 = pcOffset(pArg2, PC, 9, symbolTableCnt);
    op = opcode << 12 | dr << 9 | pcOffset9;

    outputNumToHexFile(outFile, op);
}

void NOP(FILE *outFile)
{
    outputNumToHexFile(outFile, 0x0000);
}

void NOT(char *pArg1, char *pArg2, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1001;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    op = opcode << 12 | dr << 9 | sr1 << 6 | (0b111111);
    outputNumToHexFile(outFile, op);
}

void RET(FILE *outFile)
{
    outputNumToHexFile(outFile, 0xC1C0);
}

void RTI(FILE *outFile)
{
    outputNumToHexFile(outFile, 0x8000);
}

void SHF(char *pArg1, char *pArg2, char *pArg3, int shiftOption, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1101;
    unsigned int dr = drInt(pArg1);
    unsigned int sr = srInt(pArg2);
    unsigned int option = shiftOption & 0x03;
    // TODO: check number limit
    unsigned int amount4 = toNum(pArg3) & 0x0F;
    op = opcode << 12 | dr << 9 | sr << 6 | option << 4 | amount4;
    outputNumToHexFile(outFile, op);
}

void STB(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0011;
    unsigned int sr = srInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int boffset6 = toNum(pArg3) & 0x3F;
    op = opcode << 12 | sr << 9 | baseR << 6 | boffset6;
    outputNumToHexFile(outFile, op);
}

void STW(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0111;
    unsigned int sr = srInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int offset6 = toNum(pArg3) & 0x3F;
    op = opcode << 12 | sr << 9 | baseR << 6 | offset6;
    outputNumToHexFile(outFile, op);
}

void TRAP(char *pArg1, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1111;
    // TODO: check number limit
    unsigned int trapvect8 = toNum(pArg1) & 0xFF;
    op = opcode << 12 | (0b0000) << 8 | trapvect8;
    outputNumToHexFile(outFile, op);
}

void HALT(FILE *outFile)
{
    char trapvect[5] = "x25";
    TRAP(trapvect, outFile);
}

void XOR(char *pArg1, char *pArg2, char *pArg3, FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1001;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (pArg3[0] == '#' || pArg3[0] == 'x' || pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void PS_FILL(char *pArg1, FILE *outFile)
{
    // TODO: check unsigned number
    // TODO: check memory limit
    // TODO: check number limit

    unsigned int num = toNum(pArg1) & 0xFFFF;
    outputNumToHexFile(outFile, num);
}

void firstPass(FILE *infile, int *symbolTableCnt)
{
    char pLine[MAX_LINE_LENGTH + 1], *pLabel, *pOpcode, *pArg1, *pArg2, *pArg3, *pArg4;
    int programBegin = FALSE;
    int programCounter = -1;
    int lRet;

    do
    {
        lRet = readAndParse(infile, pLine, &pLabel, &pOpcode, &pArg1, &pArg2, &pArg3, &pArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
            if (strncmp(".end", pOpcode, 4) == 0)
            {
                break;
            }
            if (programBegin == FALSE && strncmp(".orig", pOpcode, 5) == 0)
            {
                programBegin = TRUE;
                programCounter = toNum(pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
                continue;
            }
            if (programBegin == TRUE && (strlen(pLabel) > 0 || strlen(pOpcode) > 0))
            {
                programCounter += 2;
            }
            if (programBegin == TRUE && strlen(pLabel) > 0)
            {
                TableEntry te;
                te.address = programCounter;
                strcpy(te.label, pLabel);
                symbolTable[(*symbolTableCnt)++] = te;
            }
        }
    } while (lRet != DONE);
}

void secondPass(FILE *infile, FILE *outFile, int *symbolTableCnt)
{
    char pLine[MAX_LINE_LENGTH + 1], *pLabel, *pOpcode, *pArg1, *pArg2, *pArg3, *pArg4;
    int programBegin = FALSE;
    int programCounter = -1;
    int lRet;

    do
    {
        lRet = readAndParse(infile, pLine, &pLabel, &pOpcode, &pArg1, &pArg2, &pArg3, &pArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
            if (programBegin == FALSE && strncmp(".orig", pOpcode, 5) == 0)
            {
                programBegin = TRUE;
                outputNumToHexFile(outFile, toNum(pArg1));
                programCounter = toNum(pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
                continue;
            }
            if (programBegin == TRUE && strlen(pOpcode) > 0)
            {
                programCounter += 2;
                if (strncmp("add", pOpcode, 3) == 0)
                    ADD(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("and", pOpcode, 3) == 0)
                    AND(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("br", pOpcode, 2) == 0)
                    BR(pOpcode, pArg1, programCounter, *symbolTableCnt, outFile);
                else if (strncmp("halt", pOpcode, 4) == 0)
                    HALT(outFile);
                else if (strncmp("jmp", pOpcode, 3) == 0)
                    JMP(pArg1, outFile);
                else if (strncmp("jsr", pOpcode, 3) == 0)
                    JSR(pOpcode, pArg1, programCounter, *symbolTableCnt, outFile);
                else if (strncmp("ldb", pOpcode, 3) == 0)
                    LDB(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("ldw", pOpcode, 3) == 0)
                    LDW(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("lea", pOpcode, 3) == 0)
                    LEA(pArg1, pArg2, programCounter, *symbolTableCnt, outFile);
                else if (strncmp("nop", pOpcode, 3) == 0)
                    NOP(outFile);
                else if (strncmp("not", pOpcode, 3) == 0)
                    NOT(pArg1, pArg2, outFile);
                else if (strncmp("ret", pOpcode, 3) == 0)
                    RET(outFile);
                else if (strncmp("rti", pOpcode, 3) == 0)
                    RTI(outFile);
                else if (strncmp("lshf", pOpcode, 4) == 0)
                    SHF(pArg1, pArg2, pArg3, 0, outFile);
                else if (strncmp("rshfl", pOpcode, 5) == 0)
                    SHF(pArg1, pArg2, pArg3, 1, outFile);
                else if (strncmp("rshfa", pOpcode, 5) == 0)
                    SHF(pArg1, pArg2, pArg3, 3, outFile);
                else if (strncmp("stb", pOpcode, 3) == 0)
                    STB(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("stw", pOpcode, 3) == 0)
                    STW(pArg1, pArg2, pArg3, outFile);
                else if (strncmp("trap", pOpcode, 4) == 0)
                    TRAP(pArg1, outFile);
                else if (strncmp("xor", pOpcode, 3) == 0)
                    XOR(pArg1, pArg2, pArg3, outFile);
                else if (strncmp(".fill", pOpcode, 4) == 0)
                    PS_FILL(pArg1, outFile);
                else if (strncmp(".end", pOpcode, 4) == 0)
                    break;
                else
                    break; // TODO: throw error
            }
        }
    } while (lRet != DONE);
}

int main(int argc, char *argv[])
{
    // Opening Files
    FILE *infile = NULL;
    FILE *outfile = NULL;

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile)
    {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile)
    {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    /* Do stuff with files */

    // First Pass
    // 1. Find out where is the start location
    // 2. Construct the symbol table
    int *symbolTableCnt = (int *)malloc(sizeof(int));
    firstPass(infile, symbolTableCnt);

    // rewind infile to read from start
    rewind(infile);

    // Second Pass
    // Generate Machine Language Program
    secondPass(infile, outfile, symbolTableCnt);

    // Closing Files
    fclose(infile);
    fclose(outfile);
}