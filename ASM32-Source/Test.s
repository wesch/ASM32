; ======================================
; Example program ASM32 Bootstrap Mode
; ======================================
;  Global area
; ======================================                  

                    .GLOBAL 
                    .CODE align=0x0000_1000,entry,addr=0x0000_3000
                    .DATA addr=0x0000_4000,align=0x000_2000

                    .ALIGN 4k

S_1:                 .STRING "HALLO"
Word_01:             .WORD 0x0123_4567
S2:                 .STRING "ASSEMBLER-ASM32"
B1:                 .BYTE 0x01
H1:                 .HALF 0x0123
D1:                 .DOUBLE 0x0123_4567_89ab_cdef

AB_C1:
                    ADCH    R1,R2(R3)
                    ADC.L   R1,-47
                    ADC.O   R1,R2,W1

                    ADC.L   R2,-47                                      
                    .END