;test file for mmn14

     .entry K

;MAIN:	mov r5[r2],LENGTH
		add r2,STR
LOOP:	jmp END
		prn #-5
		sub r1,r4
		inc K


		mov r7[r6],r3
		bne LOOP
END:	stop
STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:		.data 22

