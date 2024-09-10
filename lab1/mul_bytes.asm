        .ORIG x3000

; Load Unsigned Integer 1 into R0
        LEA R0, UINT1
        LDW R0, R0, #0
        LDB R0, R0, #0

; Load Unsigned Integer 2 into R1
        LEA R1, UINT2
        LDW R1, R1, #0
        LDB R1, R1, #0

; Do Multiplication (Bitwise); R2 for result; R3 for counting 8 times (8-bit)
        AND R2, R2, #0
        AND R3, R3, #0
        ADD R3, R3, #8

MULTY   AND R4, R1, #1
        BRz NBIT
        ADD R2, R2, R0

NBIT    ADD R3, R3, #-1
        LSHF R0, R0, #1
        RSHFL R1, R1, #1
        BRp MULTY

; Store Result (only lower 8-bit) Back to RESULT
        LEA R0, RESULT
        LDW R0, R0, #0
        AND R1, R1, #0
        ADD R1, R1, R2
        LSHF R1, R1, #8
        RSHFL R1, R1, #8
        STB R1, R0, #0

; Check Overflow by R2 > 256 (the upper byte is not zero) and store in R3
        RSHFL R3, R2, #8
        BRz NOOVER
        AND R3, R3, #0
        ADD R3, R3, #1
        BRnzp STOREOV

NOOVER  AND R3, R3, #0

STOREOV LEA R0, OVER
        LDW R0, R0, #0
        STB R3, R0, #0

        HALT

UINT1   .FILL x3100
UINT2   .FILL x3101
RESULT  .FILL x3102
OVER    .FILL x3103

        .END