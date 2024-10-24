        .ORIG   x3000

        ; LDW Test protection exception R5 = 5
        ;LEA    R0, ProtHigh
        ;LDW    R0, R0, #0
        ;LDW    R1, R0, #0

        ; STW Test protection exception R5 = 5
        ;LEA    R0, ProtHigh
        ;LDW    R0, R0, #0
        ;STW    R1, R0, #0

        ; LDB Test protection exception R5 = 5
        ;LEA    R0, ProtHigh
        ;LDW    R0, R0, #0
        ;LDB    R1, R0, #0

        ; STB Test protection exception R5 = 5
        ;LEA    R0, ProtHigh
        ;LDW    R0, R0, #0
        ;STB    R1, R0, #0

        ; JMP Test protection exception R5 = 5
        ;LEA    R0, ProtHigh
        ;LDW    R0, R0, #0
        ;JMP    R0

        ; LDW Test unaligned exception R5 = 4
        ;LEA    R0, OddAddress
        ;LDW    R0, R0, #0
        ;LDW    R1, R0, #0

        ; STW Test unaligned exception R5 = 4
        ;LEA    R0, OddAddress
        ;LDW    R0, R0, #0
        ;STW    R1, R0, #0

        ; LDB Test Odd adress R5 = 0
        ;LEA    R0, OddAddress
        ;LDW    R0, R0, #0
        ;LDB    R1, R0, #0

        ; JMP Test unaligned exception R5 = 4
        ;LEA    R0, OddAddress
        ;LDW    R0, R0, #0
        ;JMP    R0

        ; LDW Test protection unaligned exception R5 = 5
        ;LEA    R0, ProtOdd
        ;LDW    R0, R0, #0
        ;LDW    R1, R0, #0

        ; Test Unknown Opcode 10 R5 = 3
        ;.FILL   xA000

        ; Test Unknown Opcode 11 R5 = 3
        ;.FILL   xB000

        HALT


 ProtHigh    .FILL  x2FFF
 OddAddress  .FILL  x3033
 ProtOdd     .FILL  x0023
        .END