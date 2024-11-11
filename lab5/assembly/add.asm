        .ORIG   x3000

        ; Sum first 20 bytes from Mem[0xC000]
        LEA    R0, SUMS
        LDW    R0, R0, #0
        AND    R1, R1, #0
        AND    R2, R2, #0
        ADD    R2, R2, #10
        ADD    R2, R2, #10

Loop    LDB    R3, R0, #0
        ADD    R1, R1, R3
        ADD    R0, R0, #1
        ADD    R2, R2, #-1
        BRp  Loop

        STW    R1, R0, #0

        ; Test Protection
        JMP    R1

        ; Test Page Fault
        ; LEA    R0, PAGEFAULT
        ; LDW    R0, R0, #0
        ; LDW    R1, R0, #0

        ; Test unaligned access exception
        ; ADD    R0, R0, #3
        ; STW    R1, R0, #0

        ; Test Unknown Opcode
        ; .FILL   xA000

        HALT

SUMS    .FILL   xC000
PAGEFAULT .FILL X4000
        .END