; ======================================
; Example program ASM32 Bootstrap Mode
; ======================================
;  Global area
; ======================================                  

                    .GLOBAL 
CODE1:              .CODE addr=0x0000_0020,align=0x0000_0010,entry
                    

                    ADD R1,R2,R3
                    ADD R4,R5



                    ADCH    R1,R2(R3)

DATA1:              .DATA addr=0x0000_0030,align=0x0000_0010

BUFF:               .BUFFER size=5,init=0x99

StartReg:           .REG    R1
Foo_Bar:            .EQU    47

S_1:                .STRING "HALLO"
Word_01:            .WORD 0x0123_4567

DATA2:              .DATA addr=0x0003_0000,align=0x0001_0000

B1:                 .BYTE 0x01
                    .ALIGN 16
H1:                 .HALF 0x0123
D1:                 .DOUBLE 0x0123_4567_89ab_cdef
AB_C1:
                    ADCH    R1,R2(R3)
                    ADD     R1,Foo_Bar
                    ADC.O   StartReg,R2
X:                  ADD     R1,R2

                    ADC.L   R2,-47                 
                    B       AB_C1
                    B       X
                    
CODE2:              .CODE addr=0x0001_0000,align=0x0001_0000,
                    
                    ADCH    R3,R4(R5)
                    ADD     R1,R2
                    ADC.L   R2,-47   
                    ADD     R1,R2
                    .END
