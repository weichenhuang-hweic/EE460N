    .ORIG   x1200

    ; Push Potential Used Registers to SSP
    ADD R6, R6, #-2
    STW R0, R6, #0
    ADD R6, R6, #-2
    STW R1, R6, #0
    ADD R6, R6, #-2
    STW R2, R6, #0

    ; Load Base Addr to Register 0
    LEA R0, PTSTART
    LDW R0, R0, #0

    ; Load Length to Register 1
    LEA R1, LENGTH
    LDW R1, R1, #0
    
    ; Loop PageTable and Reset Reference Bit
LOOP LDW R2, R0, #0
    AND R2, R2, x1E
    STW R2, R0, #0
    ADD R0, R0, #2
    ADD R1, R1, #-1
    BRp LOOP

    ; Pop Value Back to Registers
    LDW R2, R6, #0
    ADD R6, R6, #2
    LDW R1, R6, #0
    ADD R6, R6, #2
    LDW R0, R6, #0
    ADD R6, R6, #2
    
    RTI

PTSTART .FILL x1000
LENGTH  .FILL x80

    .END