H:	.EQU	52+5

M1:	.MODULE		; Test


HELLO:	.REG	R1

V1:		.WORD
V2:		.HALF

		PRB		R1,(R2)
		PRB.WI	R2,(S1,R3)
		PRB.WI	R2,(S1,R3),R4

		LSID	R7,R8

		ITLB	R4,(R5,R6)
		ITLB.T	R7,(R8,R9)

		DIAG	R2,R3,R4,23

		RFI

		LDPA	R3,R4(R5)
		LDPA	R4,R5(S1,R6)

		SHLA	R1,R2,R3,2
		SHLA.LO	R2,R3,R4,3


		STC		R3, 31(R5)
		STC		R4, 31(s2,R7)
		STC		R6,V2

		LDR		R1, 25(R2)
		LDR		R2, 25(S3,R3)
		LDR		R3,V1

		LDO		R2,25(R5)
		LDO		R3,V1


		EXTR	R1,R2,4,22
		EXTR.A	R3,R4,17


		DSR			R8,R9,R10
		DSR.A		R9,R10,R11,5

		ADD			R1,R2,R3

		DEP			R1,R2,25,32
		DEP.A		R3,R4,21
		DEP.I		R1,2,25,23
		DEP.AI		R5,4,22

; =================================
		CMR			R1,R2,R3
		CMR.EQ		HELLO,R2,R3
		CMR.LT		R1,R2,R3
		CMR.NE		R1,R2,R3
		CMR.LE		R1,R2,R3
		CMR.GT		R1,R2,R3
		CMR.GE		R1,R2,R3
		CMR.HI		R1,R2,R3
		CMR.HE		R1,R2,R3

		
		LD			R3, R4(S3,R5)
		LD			R2, H+45(S2,R3)

		LD			R3, R4(R5)
		LD			R2, 45+H(R3)

		LDA			R3, R4(R5)
		LDA			R2, 45(R3)


	
		LD.M		R1,1(R3)
		LD			R2,V1
		STH			R3, V2
		ADD			R3,45(r5)
		ADD			R3,H(r6)
		B			H
		LD.M		R1,H(R3)
		LD.M		R1,23(S1,R3)
		LD.M		R1,23(S2,R3)
		LD.M		R1,23(S5,R3)

		ST.M			R1,23(R3)

		ST			R2,223(S1,r2)




; ----------------------------------------------------------------------			
       .ENDMODULE			
; ----------------------------------------------------------------------			
        .END 			
; ----------------------------------------------------------------------			
 