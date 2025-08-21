; ======================================
; Example program ASM32
; ======================================

; KSO:		.GLOBAL		adr=0x1000_0000

;			.ENDGLOBAL

IVA:		.MODULE		
W1:			.WORD		
W2:			.WORD
H1:			.HALF

			.IMPORT	UTIL

MAIN:		.FUNCTION	
			ADCH	R1,R2(R3)
			ADC.L	R1,-47
			ADC.O	R1,R2,W1
			B	FUNC1, RL
			.ENDFUNCTION

INIT:		.FUNCTION
			.ENDFUNCTION
			.ENDMODULE

UTIL:		.MODULE

			.EXPORT	FUNC1
FUNC1:		.FUNCTION

STATUS:		.WORD		

			LD	RL, -8(SP)
			BV	(RL) 
			.ENDFUNCTION

			.ENDMODULE
			.ENDPROGRAM