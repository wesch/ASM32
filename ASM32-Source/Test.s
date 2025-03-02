; =====================================
; Testsuite ASM32 instructions
; =====================================



H:	.EQU	52

M1:	.MODULE		


GREG1:	.REG	R1

V1:		.WORD
V2:		.HALF
V3:		.WORD
V4:		.WORD	
V5:		.BYTE


; =====================================
; Computational instructions
; =====================================

BEG:	ADD		R1,-47
		ADD		GREG1,R2,R3
		ADD		R2,R3(R4)
		ADD		R3,-45(R5)
		ADD		R4,H(R6)
		ADDW	R5,V3
		ADDB	R6,V5
		ADDH	R1,R2(R3)
		ADD.L	R1,-47
		ADD.O	GREG1,R2,R3
		ADD.LO	R1,-47

		ADCW	R5,V3
		ADCB	R6,V5
		ADCH	R1,R2(R3)
		ADC.L	R1,-47
		ADC.O	GREG1,R2,R3
		ADC.LO	R1,-47		
				
		SBCW	R5,V3
		SBCB	R6,V5
		SBCH	R1,R2(R3)
		SBC.L	R1,-47
		SBC.O	GREG1,R2,R3
		SBC.LO	R1,-47

		SUBW	R5,V3
		SUBB	R6,V5
		SUBH	R1,R2(R3)
		SUB.L	R1,-47
		SUB.O	GREG1,R2,R3
		SUB.LO	R1,-47

		AND		R1,-43
		AND		R1,V3
		AND		R1,R2(R3)
		AND 	R3,-45(R5)
		AND 	R3,45(R5)
		ANDW 	R3,45(R5)
		ANDH 	R3,45(R5)		
		AND.N	R1,R2(R3)
		AND.C 	R3,45(R5)	
		
		OR		R1,43
		OR		R1,V3
		OR		R1,R2(R3)
		OR 		R3,45(R5)
		OR 		R3,-45(R5)
		ORW 	R3,45(R5)
		ORH 	R3,45(R5)		
		OR.N	R1,R2(R3)
		OR.C 	R3,45(R5)	
		
		XOR		R1,43
		XOR		R1,V3
		XOR		R1,R2(R3)
		XOR 	R3,45(R5)
		XOR 	R3,45(R5)
		XORW 	R3,45(R5)
		XORH 	R3,45(R5)		
		XOR.N	R1,R2(R3)
					
		CMR		R1,R2,R3
		CMR.EQ	GREG1,R2,R3
		CMR.LT	R1,R2,R3
		CMR.GT	R1,R2,R3
		CMR.EV	R1,R2,R3
		CMR.NE	R1,R2,R3
		CMR.LE	R1,R2,R3
		CMR.GE	R1,R2,R3
		CMR.OD	R1,R2,R3

		EXTR	R1,R2,4,22
		EXTR.A	R3,R4,17

		DSR		R8,R9,R10
		DSR.A	R9,R10,R11,5

		DEP		R1,R2,25,22
		DEP.A	R3,R4,21
		DEP.I	R1,2,25,23
		DEP.AI	R5,4,22
		
		CMP.EQ	R2,23
		CMP.LT	R3,V1
		CMP.NE	R4,R3
		CMP.LE	R5,R2(R4)
		CMP.LE	R5,-43(R6)
		CMP.LE	R5,R3,R4
		
		CMPU.EQ	R2,23
		CMPU.LT	R3,V1
		CMPU.NE	R4,R3
		CMPU.LE	R5,R2(R4)
		CMPU.LE	R5,43(R6)
		CMPU.LE	R5,R3,R4

		LSID	R2,R3
		
		SHLA	R5,R6,R7,2
		SHLA.L	R5,R6,R7,2	
		SHLA.O	R5,R6,R7,2
		
; =====================================
; Control Flow instructions
; =====================================
		
		B		-16
		B		32
		B		BEG
		B		16,R1
		
		BE		12(R1,R2)
		BE		12(R1,R2),R3
		
		BR		(R5)
		BR		(R5),R7
		
		BV		(R5)
		BV		(R5),R7		
		
		BVE		R5(R6)
		BVE		R5(R6),R7
		
		CBR.EQ	R1,R2,36
		CBR.LT	R1,R2,-36		
		CBR.NE	R1,R2,36		
		CBR.LE	R1,R2,36		

		CBRU.EQ	R1,R2,32
		CBRU.LT	R1,R2,32		
		CBRU.NE	R1,R2,-32		
		CBRU.LE	R1,R2,32		
		
		GATE	R5,40
		
; =====================================
; Immediate instructions
; =====================================

		ADDIL	R5,32
		
		LDIL	R6,12
		
		LDO		R5,26(R2)
		LDO		R5,V1
		
; =====================================
; Memory Reference instructions
; =====================================

		LD		R3, R4(S3,R5)
		LDH		R2, H+45(S2,R3)
		LDW		R3, R4(R5)
		LD		R2, 45+H(R3)
		LD.M	R1,H(R3)
		LD.M	R1,23(S1,R3)
		LD.M	R1,-23(S2,R3)
		LD.M	R1,23(S3,R3)
		LD		R5,V1

		LDA		R3, R4(R5)
		LDA		R2, 45(R3)
		LDA.M	R3, R4(R5)
		LDA.M	R2, -45(R3)
		
		LDR		R1, 25(R2)
		LDR		R2, 25(S3,R3)
		LDR		R3,V1
		
		ST.M	R1,23(R3)
		ST		R2,-223(S1,R2)
		STW		R3,R4(S2,R5)
		STH		R3, V2
		
		STA		R5,27(R4)
		STA		R5,R3(R4)
		
		STC		R3, 31(R5)
		STC		R4, 31(S2,R7)
		STC		R6,V2
		
; =====================================
; System Control instructions
; =====================================
		
		BRK	12,334
		
		DIAG	R2,R3,R4,7
		
		ITLB	R4,(S5,R6)
		ITLB.T	R7,(S8,R9)
		
		LDPA	R5,R4(S2,R3)
		
		MR		R5,S1
		MR		R5,C7
		MR		S2,R4
		MR		C6,R2
		
		MST		R5,R6
		MST.S	R4,11
		MST.C	R3,9	 
		
		PCA		R1(R3)
		PCA		R1(S3,R3)
		PCA.T	R1(R3)
		PCA.M	R1(S3,R3)
		PCA.TM	R1(S3,R3)
		
		PRB		R5,(S3,R3)
		PRB		R5,(S3,R3),R4
		PRB.W	R5,(S3,R3),R4
		PRB.IW	R5,(S3,R3),R4
				
		PTLB	R1(R3)
		PTLB	R1(S3,R3)
		PTLB.T	R1(R3)
		PTLB.M	R1(S3,R3)
		PTLB.TM	R1(S3,R3)
	
		RFI
; ----------------------------------------------------------------------			
       .ENDMODULE			
; ----------------------------------------------------------------------			
        .END 			
; ----------------------------------------------------------------------


	
 
