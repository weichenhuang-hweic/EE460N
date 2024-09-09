;This program counts from 10 to 0
	.ORIG x3000   	
START	ADD R1, R1, #-1
	BRZ DONE
	BR START
    JSR JP
    AND R1, R1, #0
JP  ADD R1, R1, #-1
	LDB R4, R2, #-10
	LDW R3,R3,#0
	LEA R5, EEA
	NOP
	RET
	LSHF R2, R3, #3 ; R2 ê LSHF(R3, #3)
	RSHFL R2, R3, #7 ; R2 ê RSHF(R3, #7, 0)
	RSHFA R2, R3, #7 ; R2 ê RSHF(R3, #7, R3[15])
	STB R4, R2, #10 
	STW R4, R2, #10 
DONE	TRAP x25		;The last executable instruction
	HALT
TEN	.FILL x000A		;This is 10 in 2's comp, hexadecimal
EEA .FILL x5000
	.END			;The pseudo-op, delimiting the source program

EEA .FILL x5000
EEA .FILL x5000
EEA .FILL x5000