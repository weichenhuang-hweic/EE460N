// Author: Wei-Chen Huang
// EID: wh9442
// Title: Lab 1 - Part I: Write an assembler for the LC-3b Assembly Language
// Date Begin: 2024/09/02

#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

// Syntax of Assembly Code
// LABEL OPCODE OPERANDS ; COMMENTS

// Parsing Assembly Language

#define SIXTEEN_BIT_LIMIT_PLUS_1 65536
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

int readAndParse(
    FILE *pInfile,
    char *pLine,
    char **pLabel,
    char **pOpcode,
    char **pArg1,
    char **pArg2,
    char **pArg3,
    char **pArg4)
{
    // TODO: check parser
    char *lPtr;
    int i;

    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
    {
        return (DONE);
    }

    /* convert entire line to lowercase */
    for (i = 0; i < strlen(pLine); i++)
    {
        pLine[i] = tolower(pLine[i]);
    }

    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    lPtr = pLine;
    /* ignore begin space */
    while (*lPtr == ' ')
    {
        lPtr++;
    }
    /* ignore the comments */
    if (strstr(lPtr, ";") != NULL)
    {
        char *comment = strstr(lPtr, ";");
        *comment = '\0';
    }

    if (!(lPtr = strtok(pLine, "\t\n ,")))
    {
        return (EMPTY_LINE);
    }

    /* found a label */
    if (isOpcode(lPtr) == -1 && lPtr[0] != '.')
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
        {
            return (OK);
        }
    }

    *pOpcode = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
    {
        return (OK);
    }

    *pArg1 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
    {
        return (OK);
    }

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
    {
        return (OK);
    }

    *pArg3 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
    {
        return (OK);
    }

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

int hexToBinaryStrCpy(char *pStr, char hex)
{
    switch (hex)
    {
    case '0':
        strcpy(pStr, "0000");
        break;
    case '1':
        strcpy(pStr, "0001");
        break;
    case '2':
        strcpy(pStr, "0010");
        break;
    case '3':
        strcpy(pStr, "0011");
        break;
    case '4':
        strcpy(pStr, "0100");
        break;
    case '5':
        strcpy(pStr, "0101");
        break;
    case '6':
        strcpy(pStr, "0110");
        break;
    case '7':
        strcpy(pStr, "0111");
        break;
    case '8':
        strcpy(pStr, "1000");
        break;
    case '9':
        strcpy(pStr, "1001");
        break;
    case 'a':
        strcpy(pStr, "1010");
        break;
    case 'b':
        strcpy(pStr, "1011");
        break;
    case 'c':
        strcpy(pStr, "1100");
        break;
    case 'd':
        strcpy(pStr, "1101");
        break;
    case 'e':
        strcpy(pStr, "1110");
        break;
    case 'f':
        strcpy(pStr, "1111");
        break;

    default:
        // throw error
        break;
    }

    return 4;
}

void decToHexStrCpy(char *pStr, int num)
{
    int operand = num;
    if (operand < 0)
    {
        // compute two's compliment
        operand += SIXTEEN_BIT_LIMIT_PLUS_1;
    }
    sprintf(pStr, "%04x", operand);
}

int decToBinaryStrCpy(char *pStr, int num, int digit)
{
    char *hex = (char *)malloc(4 * sizeof(char));
    char *binary = (char *)malloc(16 * sizeof(char));
    decToHexStrCpy(hex, num);
    for (int i = 0; i < 4; i++)
    {
        hexToBinaryStrCpy(binary + i * 4, hex[i]);
    }
    strncpy(pStr, binary + 16 - digit, digit);
    free(hex);
    free(binary);
    return digit;
}

void outputNumToHexFile(FILE *outfile, int num)
{
    fprintf(outfile, "0x%04X\n", num);
}

void outputBinaryToHexFile(FILE *outfile, char *ptr)
{
    int num = 0;
    while (*ptr == '1' || *ptr == '0')
    {
        int cur = *ptr == '1' ? 1 : 0;
        num = (num << 1) | cur;
        ptr++;
    }
    fprintf(outfile, "0x%04X\n", num);
}

unsigned int drInt(char **pArg)
{
    return (*pArg)[1] - '0';
}

unsigned int srInt(char **pArg)
{
    return drInt(pArg);
}

int dr(char *op, char **pArg)
{
    if (*pArg[0] == 'r')
    {
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, ((*pArg)[1] - '0'));
        hexToBinaryStrCpy(binary, hex[3]);
        strncpy(op, binary + 1, 3);
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }

    return 3;
}

int sr1(char *op, char **pArg)
{
    if (*pArg[0] == 'r')
    {
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, (*pArg)[1] - '0');
        hexToBinaryStrCpy(binary, hex[3]);
        strncpy(op, binary + 1, 3);
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }

    return 3;
}

int sr2(char *op, char **pArg)
{
    if (*pArg[0] == 'r')
    {
        strcpy(op, "000");
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, (*pArg)[1] - '0');
        hexToBinaryStrCpy(binary, hex[3]);
        strncpy(op + 3, binary + 1, 3);
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }

    return 6;
}

int imm5(char *op, char **pArg)
{
    if (*pArg[0] == '#' || *pArg[0] == 'x' || *pArg[0] == 'X')
    {
        // TODO: do we need binary?
        // TODO: might need to deal with limit of add operand (15 ~ -16)
        strcpy(op, "1");
        int num = toNum(*pArg);
        if (num > 15 || num < -16)
        {
            // TODO: throw error
            printf("Cleaning up, error code = 3\n");
            // exit(3);
        }
        decToBinaryStrCpy(op + 1, num, 5);
    }
    else
    {
        // TODO: throw error
    }

    return 6;
}

int pcOffset(int pcGap, int digit)
{
    return (pcGap >> 1) & (0xFFFF >> (16 - digit));
}

int calculatePcOffset(char *op, int digit, int pcGap)
{
    // TODO: handle error (if PCGAP > or < limit => throw error)
    char *hex = (char *)malloc(4 * sizeof(char));
    char *binary = (char *)malloc(16 * sizeof(char));
    decToHexStrCpy(hex, pcGap / 2); // LSHT 1  = divide by 2
    for (int i = 0; i < 4; i++)
    {
        hexToBinaryStrCpy(binary + i * 4, hex[i]);
    }
    strncpy(op, binary + 16 - digit, digit);
    free(hex);
    free(binary);
    return digit;
}

// ISA
void ADD(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0001;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (*pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (*pArg3[0] == '#' || *pArg3[0] == 'x' || *pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(*pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void AND(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0101;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (*pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (*pArg3[0] == '#' || *pArg3[0] == 'x' || *pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(*pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void BR(char **pOpcode,
        char **pArg1,
        int PC,
        int symbolTableCnt,
        FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0000;
    unsigned int conditional = 0;

    // Conditional Code
    if (strstr(*pOpcode, "n"))
    {
        conditional += 4;
    }
    if (strstr(*pOpcode, "z"))
    {
        conditional += 2;
    }
    if (strstr(*pOpcode, "p"))
    {
        conditional += 1;
    }
    if (strcmp(*pOpcode, "br") == 0)
    {
        conditional = 7;
    }

    // LABEL
    int jumpPC = -1;
    for (int i = 0; i < symbolTableCnt; i++)
    {
        TableEntry te = symbolTable[i];
        if (strcmp(*pArg1, te.label) == 0)
        {
            jumpPC = te.address;
            break;
        }
    }
    // TODO: handle no label found & Invalid LABEL

    int pcGap = jumpPC - (PC + 2);
    unsigned int pcOffset9 = pcOffset(pcGap, 9);
    op = opcode << 12 | conditional << 9 | (pcOffset9);
    outputNumToHexFile(outFile, op);
}

void JMP(char **pArg1,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1100;
    unsigned int baseR = srInt(pArg1);
    op = opcode << 12 | (0b000) << 9 | baseR << 6 | 0b0;
    outputNumToHexFile(outFile, op);
}

void JSR(char **pOpcode,
         char **pArg1,
         int PC,
         int symbolTableCnt,
         FILE *outFile)
{
    // This includes JSR & JSRR
    unsigned int op = 0;
    unsigned int opcode = 0b0100;

    if (strcmp(*pOpcode, "jsrr") == 0)
    {
        unsigned int baseR = srInt(pArg1);
        op = opcode << 12 | (0b000) << 9 | baseR << 6 | 0b0;
    }
    else
    {
        // LABEL
        int jumpPC = -1;
        for (int i = 0; i < symbolTableCnt; i++)
        {
            TableEntry te = symbolTable[i];
            if (strcmp(*pArg1, te.label) == 0)
            {
                jumpPC = te.address;
                break;
            }
        }
        // TODO: handle no label found & Invalid LABEL

        int pcGap = jumpPC - (PC + 2);
        unsigned int pcOffset11 = pcOffset(pcGap, 11);
        op = opcode << 12 | (0b1) << 11 | pcOffset11;
    }

    outputNumToHexFile(outFile, op);
}

// TODO: check pArg3 limit
void LDB(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0010;
    unsigned int dr = drInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int boffset6 = toNum(*pArg3) & 0x3F;
    op = opcode << 12 | dr << 9 | baseR << 6 | boffset6;
    outputNumToHexFile(outFile, op);
}

// TODO: check pArg3 limit
void LDW(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0110;
    unsigned int dr = drInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int offset6 = toNum(*pArg3) & 0x3F;
    op = opcode << 12 | dr << 9 | baseR << 6 | offset6;
    outputNumToHexFile(outFile, op);
}

void LEA(char **pArg1,
         char **pArg2,
         int PC,
         int symbolTableCnt,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1110;
    unsigned int dr = drInt(pArg1);
    // LABEL
    int jumpPC = -1;
    for (int i = 0; i < symbolTableCnt; i++)
    {
        TableEntry te = symbolTable[i];
        if (strcmp(*pArg2, te.label) == 0)
        {
            jumpPC = te.address;
            break;
        }
    }
    // TODO: handle no label found & Invalid LABEL

    int pcGap = jumpPC - (PC + 2);
    unsigned int pcOffset9 = pcOffset(pcGap, 9);
    op = opcode << 12 | dr << 9 | pcOffset9;

    outputNumToHexFile(outFile, op);
}

void NOP(FILE *outFile)
{
    outputNumToHexFile(outFile, 0x0000);
}

void NOT(char **pArg1,
         char **pArg2,
         FILE *outFile)
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

void SHF(char **pArg1,
         char **pArg2,
         char **pArg3,
         int shiftOption,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1101;
    unsigned int dr = drInt(pArg1);
    unsigned int sr = srInt(pArg2);
    unsigned int option = shiftOption & 0x03;
    // TODO: check number limit
    unsigned int amount4 = toNum(*pArg3) & 0x0F;
    op = opcode << 12 | dr << 9 | sr << 6 | option << 4 | amount4;
    outputNumToHexFile(outFile, op);
}

void STB(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0011;
    unsigned int sr = srInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int boffset6 = toNum(*pArg3) & 0x3F;
    op = opcode << 12 | sr << 9 | baseR << 6 | boffset6;
    outputNumToHexFile(outFile, op);
}

void STW(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b0111;
    unsigned int sr = srInt(pArg1);
    unsigned int baseR = srInt(pArg2);
    // TODO: check number limit
    unsigned int offset6 = toNum(*pArg3) & 0x3F;
    op = opcode << 12 | sr << 9 | baseR << 6 | offset6;
    outputNumToHexFile(outFile, op);
}

void TRAP(char **pArg1,
          FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1111;
    // TODO: check number limit
    unsigned int trapvect8 = toNum(*pArg1) & 0xFF;
    op = opcode << 12 | (0b0000) << 8 | trapvect8;
    outputNumToHexFile(outFile, op);
}

void HALT(FILE *outFile)
{
    char *trapvect = (char *)malloc(5 * sizeof(char));
    strcpy(trapvect, "x25");
    TRAP(&trapvect, outFile);
    free(trapvect);
}

void XOR(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    unsigned int op = 0;
    unsigned int opcode = 0b1001;
    unsigned int dr = drInt(pArg1);
    unsigned int sr1 = srInt(pArg2);
    if (*pArg3[0] == 'r')
    {
        unsigned int sr2 = srInt(pArg3);
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b000) << 3 | sr2;
    }
    else if (*pArg3[0] == '#' || *pArg3[0] == 'x' || *pArg3[0] == 'X')
    {
        unsigned int imme5 = toNum(*pArg3) & 0x001F;
        op = opcode << 12 | dr << 9 | sr1 << 6 | (0b1) << 5 | (imme5);
    }
    else
    {
        // TODO: throw error
    }
    outputNumToHexFile(outFile, op);
}

void PS_FILL(char **pArg1,
             FILE *outFile)
{
    // TODO: check unsigned number
    // TODO: check memory limit
    // TODO: check number limit

    unsigned int num = toNum(*pArg1) & 0xFFFF;
    outputNumToHexFile(outFile, num);
}

void firstPass(FILE *infile, int *symbolTableCnt)
{

    char *pLine = (char *)malloc(255 * sizeof(char));
    char **pLabel = (char **)malloc(1 * sizeof(char *));
    char **pOpcode = (char **)malloc(1 * sizeof(char *));
    char **pArg1 = (char **)malloc(1 * sizeof(char *));
    char **pArg2 = (char **)malloc(1 * sizeof(char *));
    char **pArg3 = (char **)malloc(1 * sizeof(char *));
    char **pArg4 = (char **)malloc(1 * sizeof(char *));

    int programBegin = FALSE;
    int programCounter = -1;

    while (readAndParse(
               infile,
               pLine,
               pLabel,
               pOpcode,
               pArg1,
               pArg2,
               pArg3,
               pArg4) != DONE)
    {
        if (strncmp(".end", *pOpcode, 4) == 0)
        {
            break;
        }
        if (programBegin == FALSE && strncmp(".orig", *pOpcode, 5) == 0)
        {
            programBegin = TRUE;
            programCounter = toNum(*pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
            continue;
        }
        if (programBegin == TRUE && (strlen(*pLabel) > 0 || strlen(*pOpcode) > 0))
        {
            programCounter += 2;
        }
        if (programBegin == TRUE && strlen(*pLabel) > 0)
        {
            TableEntry te;
            te.address = programCounter;
            strcpy(te.label, *pLabel);
            symbolTable[(*symbolTableCnt)++] = te;
        }
    };

    printf("symbol table entry count = %d\n", *symbolTableCnt);

    free(pLine);
    free(pLabel);
    free(pOpcode);
    free(pArg1);
    free(pArg2);
    free(pArg3);
    free(pArg4);
}

void secondPass(FILE *infile, FILE *outFile, int *symbolTableCnt)
{
    char *pLine = (char *)malloc(255 * sizeof(char));
    char **pLabel = (char **)malloc(1 * sizeof(char *));
    char **pOpcode = (char **)malloc(1 * sizeof(char *));
    char **pArg1 = (char **)malloc(1 * sizeof(char *));
    char **pArg2 = (char **)malloc(1 * sizeof(char *));
    char **pArg3 = (char **)malloc(1 * sizeof(char *));
    char **pArg4 = (char **)malloc(1 * sizeof(char *));

    int programBegin = FALSE;
    int programCounter = -1;

    while (readAndParse(
               infile,
               pLine,
               pLabel,
               pOpcode,
               pArg1,
               pArg2,
               pArg3,
               pArg4) != DONE)
    {
        if (programBegin == FALSE && strncmp(".orig", *pOpcode, 5) == 0)
        {
            programBegin = TRUE;
            outputNumToHexFile(outFile, toNum(*pArg1));
            programCounter = toNum(*pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
            continue;
        }
        if (programBegin == TRUE && strlen(*pOpcode) > 0)
        {
            programCounter += 2;
            if (strncmp("add", *pOpcode, 3) == 0)
            {
                ADD(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("and", *pOpcode, 3) == 0)
            {
                AND(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("br", *pOpcode, 2) == 0)
            {
                BR(pOpcode, pArg1, programCounter, *symbolTableCnt, outFile);
            }
            else if (strncmp("halt", *pOpcode, 4) == 0)
            {
                HALT(outFile);
            }
            else if (strncmp("jmp", *pOpcode, 3) == 0)
            {
                JMP(pArg1, outFile);
            }
            else if (strncmp("jsr", *pOpcode, 3) == 0)
            {
                JSR(pOpcode, pArg1, programCounter, *symbolTableCnt, outFile);
            }
            else if (strncmp("ldb", *pOpcode, 3) == 0)
            {
                LDB(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("ldw", *pOpcode, 3) == 0)
            {
                LDW(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("lea", *pOpcode, 3) == 0)
            {
                LEA(pArg1, pArg2, programCounter, *symbolTableCnt, outFile);
            }
            else if (strncmp("nop", *pOpcode, 3) == 0)
            {
                NOP(outFile);
            }
            else if (strncmp("not", *pOpcode, 3) == 0)
            {
                NOT(pArg1, pArg2, outFile);
            }
            else if (strncmp("ret", *pOpcode, 3) == 0)
            {
                RET(outFile);
            }
            else if (strncmp("rti", *pOpcode, 3) == 0)
            {
                RTI(outFile);
            }
            else if (strncmp("lshf", *pOpcode, 4) == 0)
            {
                SHF(pArg1, pArg2, pArg3, 0, outFile);
            }
            else if (strncmp("rshfl", *pOpcode, 5) == 0)
            {
                SHF(pArg1, pArg2, pArg3, 1, outFile);
            }
            else if (strncmp("rshfa", *pOpcode, 5) == 0)
            {
                SHF(pArg1, pArg2, pArg3, 3, outFile);
            }
            else if (strncmp("stb", *pOpcode, 3) == 0)
            {
                STB(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("stw", *pOpcode, 3) == 0)
            {
                STW(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp("trap", *pOpcode, 4) == 0)
            {
                TRAP(pArg1, outFile);
            }
            else if (strncmp("xor", *pOpcode, 3) == 0)
            {
                XOR(pArg1, pArg2, pArg3, outFile);
            }
            else if (strncmp(".fill", *pOpcode, 4) == 0)
            {
                PS_FILL(pArg1, outFile);
            }
            else if (strncmp(".end", *pOpcode, 4) == 0)
            {
                break;
            }
        }
    };

    free(pLine);
    free(pLabel);
    free(pOpcode);
    free(pArg1);
    free(pArg2);
    free(pArg3);
    free(pArg4);
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