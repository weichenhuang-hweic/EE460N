        .ORIG x3000
        LEA R3, MEM ; x3000
        LDW R3, R3, #0 
        LEA R0, FIRST ; 
        LDB R0, R0, #0 ; 
        LEA R6, BYTEMAX;
        LDW R6, R6, #0 ;
        AND R0, R0, R6
        LEA R1, SECOND ; 
        LDB R1, R1, #0 ; 
        AND R1, R1, R6
        LEA R6, WORD ; 
        LDW R6, R6, #0 ; 
        ADD R6, R6, R0 ; 
        LEA R4 L2 ; 
        JMP R4 ; 
        ADD R6, R6, R1 ; 

L2      AND R2, R2, #0 ; Clear R2 (to store result) ;
        JSR S1 ; 
L1      AND R4, R1, #1 ; Take the least significant bit of R1 ; 
        BRz SKIP ; Do not add to temporary result if bit is 0 ; 
        ADD R2, R2, R0 ; Add to temporary result ; 

SKIP    LSHF R0, R0, #1 ; Left shift R0 by 1 bit ; 
        RSHFA R1, R1, #1 ; Right shift R1 by 1 bit ;
        BRnp L1 ; if R1 is not zero (yet), continue loop ;

        STB R2, R3, #2 ; Mem[0x3102] = Mem[0x3100] * Mem[0x3101] ;

        ; check overflow
        AND R5, R5, #0 ; Clear R5 (to store overflow byte) ; 
        LEA R4, BYTEMAX ; x302a ; PROBLEM
        LDW R4, R4, #0 ; Now R4 contains BYTE_MAX (0xFF, maximum value for an unsigned byte) ;
        NOT R2, R2 ;
        ADD R2, R2, #1 ; Take two's complement of R2 ; 
        ADD R4, R4, R2 ; BYTE_MAX - R2 ; 
        BRp SETBYTE ; if (BYTE_MAX - R2 > 0), no overflow ; 
        ADD R5, R5, #1 ; Set overflow byte ; 
        
SETBYTE STB R5, R3, #3 ; Mem[0x3103] = did_overflow_byte(Mem[0x3100] * Mem[0x3101]) ; 
        HALT ; x303a
        
S1      STW R4, R3, #0 ;
        RET ; 

MEM	.FILL x3100 ; 
BYTEMAX .FILL x00FF ;
FIRST   .FILL xFF ;
SECOND  .FILL #5 ;
WORD    .FILL #4096 ;
        .END