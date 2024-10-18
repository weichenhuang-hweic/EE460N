    .ORIG   x1200

    ; Push Potential Used Registers to SSP
    ADD R6, R6, #-2
    STW R0, R6, #0
    ADD R6, R6, #-2
    STW R1, R6, #0

    LEA R0, MEM
    LDW R0, R0, #0
    LDW R1, R0, #0
    ADD R1, R1, #1
    STW R1, R0, #0

    ; Pop Value Back to Registers
    LDW R0, R6, #0
    ADD R6, R6, #2
    LDW R1, R6, #0
    ADD R6, R6, #2
    
    RTI

MEM .FILL x4000

    .END