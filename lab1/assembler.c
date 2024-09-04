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
        strcpy(pStr, "0000");
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
        operand += SIXTEEN_BIT_LIMIT_PLUS_1;
    }
    sprintf(pStr, "%04x", operand);
}

void outputNumToHexFile(FILE *outfile, int num)
{
    fprintf(outfile, "0x%04x\n", num);
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
    fprintf(outfile, "0x%04x\n", num);
}

// ISA
void add(char **pArg1,
         char **pArg2,
         char **pArg3,
         FILE *outFile)
{
    char *op = (char *)malloc(17 * sizeof(char));
    int cnt = 0;
    strcpy(op, "0001");
    cnt += 4;

    // DR
    if (*pArg1[0] == 'r')
    {
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, ((*pArg1)[1] - '0'));
        hexToBinaryStrCpy(binary, hex[3]);
        strcpy(op + cnt, binary + 1);
        cnt += 3;
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }

    // SR1
    if (*pArg2[0] == 'r')
    {
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, (*pArg2)[1] - '0');
        hexToBinaryStrCpy(binary, hex[3]);
        strncpy(op + cnt, binary + 1, 3);
        cnt += 3;
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }

    if (*pArg3[0] == 'r')
    {
        // SR2
        strcpy(op + cnt, "000");
        cnt += 3;
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(4 * sizeof(char));
        decToHexStrCpy(hex, (*pArg3)[1] - '0');
        hexToBinaryStrCpy(binary, hex[3]);
        strncpy(op + cnt, binary + 1, 3);
        cnt += 3;
        free(hex);
        free(binary);
    }
    else if (*pArg3[0] == '#' || *pArg3[0] == 'x' || *pArg3[0] == 'X')
    {
        // TODO: might need to deal with limit of add operand (15 ~ -16)
        // imm5
        strcpy(op + cnt, "1");
        cnt += 1;
        char *hex = (char *)malloc(4 * sizeof(char));
        char *binary = (char *)malloc(16 * sizeof(char));
        decToHexStrCpy(hex, toNum(*pArg3));
        for (int i = 0; i < 4; i++)
        {
            hexToBinaryStrCpy(binary + i * 4, hex[i]);
        }
        strncpy(op + cnt, binary + 11, 5);
        cnt += 5;
        free(hex);
        free(binary);
    }
    else
    {
        // TODO: throw error
    }
    outputBinaryToHexFile(outFile, op);
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
        if (programBegin == FALSE && strncmp(".orig", *pOpcode, 5) == 0)
        {
            programBegin = TRUE;
            programCounter = toNum(*pArg1) - 2; /* .orig pseudo program counter is the value minus 1 instruction spac*/
            continue;
        }
        programCounter += 2;
        if (programBegin == TRUE && strlen(*pLabel) > 0)
        {
            TableEntry te;
            te.address = programCounter;
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

        if (programBegin == TRUE)
        {
            // ADD
            if (strncmp("add", *pOpcode, 3) == 0)
            {
                add(pArg1, pArg2, pArg3, outFile);
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