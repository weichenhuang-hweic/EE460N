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

enum INSTRUCTION_SETS
{
    ADD,
    AND,
    BRn,
    BRz,
    BRp,
    BR,
    BRzp,
    BRnp,
    BRnz,
    BRnzp,
    HALT,
    JMP,
    JSR,
    JSRR,
    LDB,
    LDW,
    LEA,
    NOP,
    NOT,
    RET,
    RTI,
    LSHF,
    RSHFL,
    RSHFA,
    STB,
    STW,
    TRAP,
    XOR,
};

char opcodeMap[28][5] = {
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
    char *lRet, *lPtr;
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
    /* ignore the comments */
    if (*lPtr == ';')
    {
        lPtr++;
        while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n')
        {
            lPtr++;
        }
        return (OK);
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

void firstPass(FILE *infile, int *programCounter, int *symbolTableCnt)
{

    char *pLine = (char *)malloc(255 * sizeof(char));
    char **pLabel = (char **)malloc(1 * sizeof(char *));
    char **pOpcode = (char **)malloc(1 * sizeof(char *));
    char **pArg1 = (char **)malloc(1 * sizeof(char *));
    char **pArg2 = (char **)malloc(1 * sizeof(char *));
    char **pArg3 = (char **)malloc(1 * sizeof(char *));
    char **pArg4 = (char **)malloc(1 * sizeof(char *));

    int programBegin = FALSE;

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
            (*programCounter) = toNum(*pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
            continue;
        }
        (*programCounter) += 2;
        if (programBegin == TRUE && strlen(*pLabel) > 0)
        {
            TableEntry te;
            te.address = (*programCounter);
            for (int i = 0; i < strlen(*pLabel); i++)
            {
                te.label[i] = (*pLabel)[i];
            }
            te.label[strlen(*pLabel)] = '\0';
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

int main(int argc, char *argv[])
{

    // Parsing Command Line Arguments
    char *prgName = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

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
    int *programCounter = (int *)malloc(sizeof(int));
    int *symbolTableCnt = (int *)malloc(sizeof(int));
    firstPass(infile, programCounter, symbolTableCnt);

    // Second Pass
    // Generate Machine Language Program

    // Closing Files
    fclose(infile);
    fclose(outfile);
}