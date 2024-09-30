// ****************************************************
//  Constants for ASM32 Assembler
// ****************************************************

#ifndef CONSTANTS_H
#define CONSTANTS_H



#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#define TRUE 1
#define FALSE 0


#define VERSION "A.00.02.006"


/* Addresses: */
#define FIRST_MEMORY_ADDRESS 100

/* Lengths: */
#define MAX_LINE_LENGTH 80 /* Max line length*/
#define MAX_FILE_NAME_LENGTH 255 
#define MAX_WORD_LENGTH 32
#define MAX_ERROR_LENGTH 255
#define MAX_TOKEN_PER_LINE 20


/* Filetypes Suffix: */
#define SOURCE_FILETYPE "asm"
#define DISASM_OUT "dis"
#define LIST_OUT "lst"
#define SIMULATOR_OUT "sim"

/* Program errors: */
#define FILE_OPEN_ERROR "ERROR: File open failure:"
#define FILE_CLOSE_ERROR "ERROR: File closing failure:"
#define EXIT_MESSAGE "The program will now exit."
#define NO_ERROR_CODE 0

/* Flags for source */
#define FLG_COMMAND 1
#define FLG_EMPTY 2
#define FLG_COMMENT 3
#define FLG_LABEL 4

#endif /* CONSTANTS_H */






// -------------------------  enum token types  --------------------*

enum {
    NONE,
    IDENTIFIER,
    NUM,
    COMMA,
    COLON,
    DOT,
    LPAREN,
    RPAREN,
    COMMENT,
    DIRECTIVE,
    OPCODE,
    LABEL,
    MINUS,
    PLUS,
    MUL,
    DIV,
    EOL
};

// -------------------------  enum AST node types  --------------------*

typedef enum {
    NODE_PROGRAM,
    NODE_INSTRUCTION,
    NODE_DIRECTIVE,
    NODE_OPERATION,
    NODE_OPERAND,
    NODE_MODE,
    NODE_DATA_DECLARATION,
    NODE_EXPRESSION,
    NODE_CONSTANT,
    NODE_LABEL
} NodeType;



// -------------------------  enum ScopeType Symboltable --------------------*

typedef enum {
    GLOBAL = 0,     // Globaler Scope
    PROGRAM = 1,    // Programm- oder Datei-Scope
    MODULE = 2,     // Modul-Scope
    FUNCTION = 3,   // Funktions-Scope (lokal)
    BLOCK = 4,      // Block Scope
    DIRECT = 5,
} ScopeType;



// -------------------------  enum opCodes --------------------*

enum {
    ADD,
    ADC,
    ADDIL,
    AND,
    B,
    BE,
    BR,
    BRK,
    BV,
    BVE,
    CBR,
    CBRU,
    CMP,
    CMPU,
    CMR,
    DEP,
    DIAG,
    DSR,
    EXTR,
    GATE,
    ITLB,
    LD,
    LDA,
    LDIL,
    LDO,
    LDPA,
    LDR,
    LSID,
    MR,
    MST,
    OR,
    PCA,
    PRB,
    PTLB,
    RFI,
    SBC,
    SHLA,
    ST,
    STA,
    STC,
    SUB,
    XOR
};


// --------------------------------------------------------------------------------
// Table of directives
// --------------------------------------------------------------------------------

const struct directInfo {
    char directive[12];
} dirCodeTab[] = {

    { "ALIGN" },
    { "BYTE" },
    { "CODE" },
    { "DATA" },
    { "END" },
    { "EQU" },
    { "HALF" },
    { "REG" },
    { "STRING" },
    { "WORD" },
    { "GLOBAL" },
    { "PROGRAM" },
    { "MODULE" },
    { "ENDMODULE" },
    { "FUNCTION" },
    { "ENDFUNCTION" },
    { "BLOCK" },
    { "ENDBLOCK" },

};


// --------------------------------------------------------------------------------
// Table of OpCode mnemonics
// 
// bin_instr: 32 bit instruction which will be filled druing analysis of instruction
// 
// instr type; enum opCode to overcome opCode variants with B,H,W
// 
// --------------------------------------------------------------------------------


const struct opCodeInfo {
    char        mnemonic[8];
    uint32_t     binInstr;
    int         instrType;
} opCodeTab[] = {

    { "ADD",     0x40000000 , ADD },
    { "ADDB",    0x40030000 , ADD },
    { "ADDH",    0x40010000 , ADD },
    { "ADDW",    0x40000000 , ADD },

    { "ADC",     0x44000000 , ADC },
    { "ADCB",    0x44030000 , ADC },
    { "ADCH",    0x44010000 , ADC },
    { "ADCW",    0x44000000 , ADC },

    { "AND",     0x50000000 , AND },
    { "ANDB",    0x50030000 , AND },
    { "ANDH",    0x50010000 , AND },
    { "ANDW",    0x50000000 , AND },

    { "CMP",     0x5C000000 , CMP },
    { "CMPB",    0x5C030000 , CMP },
    { "CMPH",    0x5C010000 , CMP },
    { "CMPW",    0x5C000000 , CMP },

    { "CMPU",    0x60000000 , CMPU },
    { "CMPUB",   0x60030000 , CMPU },
    { "CMPUH",   0x60010000 , CMPU },
    { "CMPUW",   0x60000000 , CMPU },

    { "OR",      0x54000000 , OR },
    { "ORB",     0x54030000 , OR },
    { "ORH",     0x54010000 , OR },
    { "ORW",     0x54000000 , OR },


    { "SBC",     0x4C000000 , SBC },
    { "SBCB",    0x4C030000 , SBC },
    { "SBCH",    0x4C010000 , SBC },
    { "SBCW",    0x4C000000 , SBC },

    { "SUB",     0x44000000 , SUB },
    { "SUBB",    0x44030000 , SUB },
    { "SUBH",    0x44010000 , SUB },
    { "SUBW",    0x44000000 , SUB },

    { "XOR",     0x58000000 , XOR },
    { "XORB",    0x58030000 , XOR },
    { "XORH",    0x58010000 , XOR },
    { "XORW",    0x58000000 , XOR },

    { "ADDIL",   0x08000000 , ADDIL },

    { "LDIL",    0x04000000 , LDIL },

    { "LD",      0xC0000000 , LD },
    { "LDB",     0xC0030000 , LD },
    { "LDH",     0xC0010000 , LD },
    { "LDW",     0xC0000000 , LD },

    { "LDR",     0xD0000000 , LDR },

    { "LDA",     0x68000000 , LDA },
    { "LDAB",    0x68030000 , LDA },
    { "LDAH",    0x68010000 , LDA },
    { "LDAW",    0x68000000 , LDA },

    { "LDO",     0x0C000000 , LDO },

    { "ST",      0xC4000000 , ST },
    { "STB",     0xC4030000 , ST },
    { "STH",     0xC4010000 , ST },
    { "STW",     0xC4000000 , ST },
};
