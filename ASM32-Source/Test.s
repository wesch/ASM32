; ======================================
; Example program ASM32 Bootstrap Mode
; ======================================
;  Global area
; ======================================                  

                    .GLOBAL 
                    .CODE entry,addr=0x0000_5000
                    .DATA addr=0x0000_0100,align=0x000_0100
; BUFF:               .BUFFER size=500,init=0x88
                   

StartReg:           .REG    R1
Foo_Bar:            .EQU    47

; S_1:                .STRING "HAL LO"
Word_01:            .WORD 0x0123_4567
; S2:                 .STRING "ASSEMBLER   ASM32"

B1:                 .BYTE 0x01
                    .ALIGN 16
H1:                 .HALF 0x0123
D1:                 .DOUBLE 0x0123_4567_89ab_cdef

AB_C1:
                    ADCH    R1,R2(R3)
                    ADC.L   R1,Foo_Bar
                    ADC.O   StartReg,R2,W1

                    ADC.L   R2,-47                                      
                    .END
