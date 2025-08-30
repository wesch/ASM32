// ****************************************************
//  Konstanten ASM32 Assembler
// ****************************************************

#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @file
/// \brief enthält die Defines und enums für den Assembler


#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <inttypes.h>

#define TRUE 1
#define FALSE 0


#define VERSION "A.00.1.05"


/* Adressen */
#define FIRST_MEMORY_ADDRESS 100

/* Längen */
#define MAX_LINE_LENGTH 255 /* Max line length*/
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






/// \brief Token Typen

typedef enum {
    NONE,
    T_IDENTIFIER,
    T_NUM,
    T_COMMA,
    T_COLON,
    T_DOT,
    T_LPAREN,
    T_QUOT,
    T_RPAREN,
    T_L22,
    T_R10,
    T_COMMENT,
    T_DIRECTIVE,
    T_OPCODE,
    T_LABEL,
    T_MINUS,
    T_UNDERSCORE,
    T_PLUS,
    T_MUL,
    T_DIV,
    T_NEG,
    T_MOD,
    T_OR,
    T_AND,
    T_XOR,
    T_EOL
} TokenType ;

/// \brief AST node types

typedef enum {
    NODE_PROGRAM,
    NODE_INSTRUCTION,
    NODE_DIRECTIVE,
    NODE_OPERATION,
    NODE_OPERAND,
    NODE_OPTION,
    NODE_MODE,
    NODE_DATA_DECLARATION,
    NODE_EXPRESSION,
    NODE_CONSTANT,
    NODE_LABEL
} AST_NodeType;

typedef enum {
    SRC_GLOBAL,
    SRC_PROGRAM,
    SRC_SOURCE,
    SRC_BIN,
    SRC_ERROR,
    SRC_INFO
} SRC_NodeType;

/// \brief AST operandType 
typedef enum {  
    OT_NOTHING,
    OT_REGISTER,
    OT_MEMGLOB,
    OT_MEMLOC,
    OT_LABEL,
    OT_VALUE
} AST_OperandType;

/// \brief ScopeType Symboltable 

typedef enum {
    SCOPE_PROGRAM = 1,    // Programm- oder Datei-Scope
    SCOPE_MODULE = 2,     // Modul-Scope
    SCOPE_FUNCTION = 3,   // Funktions-Scope (lokal)
    SCOPE_DIRECT = 4,
} SYM_ScopeType;

/// \brief Variablen Type für Symboltabelle
/// </summary>

typedef enum {
    V_VALUE,
    V_MEMGLOBAL,
    V_MEMLOCAL,
    V_LABEL,
} SYM_VarType;

/// \brief bininstr in SRCnode
/// 
typedef enum {
    B_NOBIN,
    B_BIN,
    B_BINCHILD,
    
} BIN_Type;


typedef enum {
    P_UNDEFINED,
    P_STANDALONE,
    P_MODULE,
};

/// \brief opCodes 

typedef enum {
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
    DS,
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
} Opcodes;

/// \brief    Directives 

typedef enum {
    D_ALIGN,
    D_BEGIN,
    D_BYTE,
    D_CODE,
    D_DATA,
    D_EQU,
    D_HALF,
    D_IMPORT,
    D_EXPORT,
    D_REG,
    D_STRING,
    D_WORD,
    D_DOUBLE,
    D_STR,
    D_GLOBAL,
    D_PROGRAM,
    D_MODULE,
    D_ENDMODULE,
    D_FUNCTION,
    D_ENDFUNCTION


} Directives;

/// \brief Table of directives

const struct directInfo { 
    char directive[12];
    int  directNum;
} dirCodeTab[] = {

    { "ALIGN" ,         D_ALIGN },
    { "BEGIN" ,         D_BEGIN },
    { "BYTE",           D_BYTE },
    { "CODE",           D_CODE },
    { "DATA" ,          D_DATA },
    { "EQU" ,           D_EQU },
    { "HALF" ,          D_HALF },
    { "EXPORT" ,        D_EXPORT},
    { "IMPORT" ,        D_IMPORT},
    { "REG" ,           D_REG },
    { "STRING" ,        D_STRING },
    { "WORD" ,          D_WORD },
    { "DOUBLE",         D_DOUBLE },
    { "GLOBAL" ,        D_GLOBAL },
    { "PROGRAM" ,       D_PROGRAM },
    { "MODULE" ,        D_MODULE },
    { "ENDMODULE" ,     D_ENDMODULE },
    { "FUNCTION" ,      D_FUNCTION },
    { "ENDFUNCTION" ,   D_ENDFUNCTION }

} ;




/// \brief Table of other reserved words

const struct reservedInfo {
    char resWord[12];
} resWordTab[] = {

    { "IF" },
    { "ELSE" },
    { "R0" },
    { "R1" },
    { "R2" },
    { "R3" },
    { "R4" },
    { "R5" },
    { "R6" },
    { "R7" },
    { "R8" },
    { "R9" },
    { "R10" },
    { "R11" },
    { "R12" },
    { "R13" },
    { "R14" },
    { "R15" },
    { "R16" },

};


/// \brief Table of OpCode mnemonics
/// bin_instr: 32 bit instruction which will be filled druing analysis of instruction
/// instr type; enum opCode to overcome opCode variants with B,H,W


const struct opCodeInfo {
    char        mnemonic[8];
    uint32_t    binInstr;
    int         instrType;
} opCodeTab[] = {

    { "ADD",     0x40020000 , ADD },
    { "ADDB",    0x40000000 , ADD },
    { "ADDH",    0x40010000 , ADD },
    { "ADDW",    0x40020000 , ADD },

    { "ADC",     0x44020000 , ADC },
    { "ADCB",    0x44000000 , ADC },
    { "ADCH",    0x44010000 , ADC },
    { "ADCW",    0x44020000 , ADC },
        
    { "ADDIL",   0x08000000 , ADDIL },

    { "AND",     0x50020000 , AND },
    { "ANDB",    0x50000000 , AND },
    { "ANDH",    0x50010000 , AND },
    { "ANDW",    0x50020000 , AND },

    { "B",       0x80000000 , B },
    { "BE",      0x90000000 , BE },
    { "BR",      0x88000000 , BR },
    { "BRK",     0x00000000 , BRK },
    { "BV",      0x8c000000 , BV },
    { "BVE",     0x94000000 , BVE },

    { "CBR",      0x98000000 , CBR },
    { "CBRU",     0x9C000000 , CBRU },

    { "CMP",     0x5C020000 , CMP },
    { "CMPB",    0x5C000000 , CMP },
    { "CMPH",    0x5C010000 , CMP },
    { "CMPW",    0x5C020000 , CMP },

    { "CMPU",    0x60020000 , CMPU },
    { "CMPUB",   0x60000000 , CMPU },
    { "CMPUH",   0x60010000 , CMPU },
    { "CMPUW",   0x60020000 , CMPU },

    { "CMR",      0x24000000 , CMR },
    { "DEP",      0x18000000 , DEP },
    { "DIAG",     0xF8000000 , DIAG },
    { "DS",       0x30000000 , DS },

    { "DSR",      0x1C000000 , DSR },
    { "EXTR",     0x14000000 , EXTR },
    { "GATE",     0x84000000 , GATE },
    { "ITLB",     0xEC000000 , ITLB },

    { "LD",      0xC0020000 , LD },
    { "LDB",     0xC0000000 , LD },
    { "LDH",     0xC0010000 , LD },
    { "LDW",     0xC0020000 , LD },

    { "LDIL",    0x04000000 , LDIL },
    { "LDO",     0x0C000000 , LDO },
    { "LDPA",    0xE4000000 , LDPA },
    { "LDR",     0xD0000000 , LDR },
    { "LDA",     0xC8020000 , LDA },

    { "LSID",    0x10000000 , LSID },
    { "MR",      0x28000000 , MR },
    { "MST",     0x2C000000 , MST },

    { "OR",      0x54020000 , OR },
    { "ORB",     0x54000000 , OR },
    { "ORH",     0x54010000 , OR },
    { "ORW",     0x54020000 , OR },

    { "PCA",     0xF4000000 , PCA },
    { "PRB",     0xE8000000 , PRB },
    { "PTLB",    0xF0000000 , PTLB },
    { "RFI",     0xFC000000 , RFI },

    { "SBC",     0x4C020000 , SBC },
    { "SBCB",    0x4C000000 , SBC },
    { "SBCH",    0x4C010000 , SBC },
    { "SBCW",    0x4C020000 , SBC },

    { "SHLA",    0x20000000 , SHLA },

    { "ST",      0xC4020000 , ST },
    { "STB",     0xC4000000 , ST },
    { "STH",     0xC4010000 , ST },
    { "STW",     0xC4020000 , ST },

    { "STA",     0xCC000000 , STA },
    { "STC",     0xD4000000 , STC },

    { "SUB",     0x48020000 , SUB },
    { "SUBB",    0x48000000 , SUB },
    { "SUBH",    0x48010000 , SUB },
    { "SUBW",    0x48020000 , SUB },

    { "XOR",     0x58020000 , XOR },
    { "XORB",    0x58000000 , XOR },
    { "XORH",    0x58010000 , XOR },
    { "XORW",    0x58020000 , XOR },

};


