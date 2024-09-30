M1:     .MODULE

HUGO:   .EQU    29



        add R1,HUGO + 7
        add R2, 3 * HUGO
        add R3, (2 + HUGO) * 3
        ADD R4, (HUGO + 1)
        add R5, 21
        add R6, -22
        add R7, HUGO

        
        add r0,r14,R5
        add r1,r15(r2)

SP:     .REG   R11




        add R8, HUGO
        add SP,4(R11)
        add R12, HUGO


XY:     .REG   R3
        add r2,r3(r4)
        add XY,HUGO(SP)

 
        add R2,SP

F1:     .FUNCTION     

HUGO:   .EQU 9 
        add R12, HUGO

        .ENDFUNCTION




       

S:      ADD R2,SP

       
        .ENDMODULE
       
        .END