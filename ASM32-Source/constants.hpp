// ****************************************************
//  Constants for ASM32 Assembler
// ****************************************************

#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @file
/// \brief Defines constants, enumerations, and lookup tables for the ASM32 assembler.
/// \details
/// This header provides symbolic constants, token and node type enumerations, 
/// directive and opcode tables, and utility definitions used throughout 
/// the ASM32 assembler. It ensures consistency and centralizes core definitions.

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

#define VERSION "A.00.1.12"


// -----------------------------------------------------------------------------
// Length and buffer size definitions
// -----------------------------------------------------------------------------

/// \brief Maximum lengths for various inputs and buffers.
#define MAX_LINE_LENGTH 255      ///< Maximum length of a source line.
#define MAX_FILE_NAME_LENGTH 255 ///< Maximum length of a file name.
#define MAX_WORD_LENGTH 32       ///< Maximum length of an identifier or symbol.
#define MAX_ERROR_LENGTH 255     ///< Maximum length of an error message.
#define MAX_TOKEN_PER_LINE 20    ///< Maximum number of tokens per source line.


// -----------------------------------------------------------------------------
// File extensions
// -----------------------------------------------------------------------------

/// \brief File type suffixes used by the assembler.
#define SOURCE_FILETYPE "asm" ///< Source file extension.
#define DISASM_OUT "dis"      ///< Disassembly output file extension.
#define LIST_OUT "lst"        ///< Listing output file extension.
#define SIMULATOR_OUT "sim"   ///< Simulator output file extension.


// -----------------------------------------------------------------------------
// Error messages
// -----------------------------------------------------------------------------

/// \brief Program error messages and codes.
#define FILE_OPEN_ERROR "ERROR: File open failure:"
#define FILE_CLOSE_ERROR "ERROR: File closing failure:"
#define EXIT_MESSAGE "The program will now exit."
#define NO_ERROR_CODE 0


// -----------------------------------------------------------------------------
// Source line flags
// -----------------------------------------------------------------------------

/// \brief Flags that classify source code lines.
#define FLG_COMMAND 1 ///< Line contains an assembler command.
#define FLG_EMPTY 2   ///< Line is empty or whitespace only.
#define FLG_COMMENT 3 ///< Line contains a comment.
#define FLG_LABEL 4   ///< Line defines a label.

#endif /* CONSTANTS_H */


// -----------------------------------------------------------------------------
// Token types
// -----------------------------------------------------------------------------

/// \brief Token types recognized by the lexical analyzer.
/// \details
/// Tokens represent the smallest meaningful units in source code, 
/// such as identifiers, numbers, operators, or punctuation.
typedef enum {
    NONE,           ///< No token.
    T_IDENTIFIER,   ///< Identifier (e.g., variable, symbol, or label).
    T_NUM,          ///< Numeric constant.
    T_COMMA,        ///< Comma ','.
    T_COLON,        ///< Colon ':'.
    T_DOT,          ///< Dot '.'.
    T_LPAREN,       ///< Left parenthesis '('.
    T_QUOT,         ///< Quotation mark '"'.
    T_RPAREN,       ///< Right parenthesis ')'.
    T_L22,          ///< Reserved token (L22).
    T_R10,          ///< Reserved token (R10).
    T_COMMENT,      ///< Comment sequence.
    T_DIRECTIVE,    ///< Assembler directive (e.g., DATA, CODE).
    T_OPCODE,       ///< Instruction opcode mnemonic.
    T_LABEL,        ///< Label definition.
    T_MINUS,        ///< Minus operator '-'.
    T_UNDERSCORE,   ///< Underscore '_'.
    T_PLUS,         ///< Plus operator '+'.
    T_MUL,          ///< Multiplication operator '*'.
    T_DIV,          ///< Division operator '/'.
    T_NEG,          ///< Negation operator.
    T_MOD,          ///< Modulo operator '%'.
    T_OR,           ///< Bitwise OR '|'.
    T_AND,          ///< Bitwise AND '&'.
    T_XOR,          ///< Bitwise XOR '^'.
    T_EOL           ///< End of line.
} TokenType;


// -----------------------------------------------------------------------------
// AST node types
// -----------------------------------------------------------------------------

/// \brief Types of nodes used in the Abstract Syntax Tree (AST).
typedef enum {
    NODE_PROGRAM,          ///< Root node representing a complete program.
    NODE_INSTRUCTION,      ///< An instruction node.
    NODE_DIRECTIVE,        ///< An assembler directive node.
    NODE_OPERATION,        ///< An operation node within an instruction.
    NODE_OPERAND,          ///< An operand node.
    NODE_OPTION,           ///< An option or modifier node.
    NODE_MODE,             ///< Addressing or operation mode node.
    NODE_DATA_DECLARATION, ///< Data declaration node (e.g., .BYTE, .WORD).
    NODE_EXPRESSION,       ///< An expression node.
    NODE_CONSTANT,         ///< Constant value node.
    NODE_LABEL             ///< Label node.
} AST_NodeType;


// -----------------------------------------------------------------------------
// Source node classification
// -----------------------------------------------------------------------------

/// \brief Types of source code nodes (for logging, errors, etc.).
typedef enum {
    SRC_GLOBAL,  ///< Global scope element.
    SRC_PROGRAM, ///< Program-level element.
    SRC_SOURCE,  ///< Source code line.
    SRC_BIN,     ///< Binary code section.
    SRC_ERROR,   ///< Error message.
    SRC_WARNING, ///< Warning message.
    SRC_INFO     ///< Informational message.
} SRC_NodeType;


// -----------------------------------------------------------------------------
// Operand types
// -----------------------------------------------------------------------------

/// \brief Types of operands used in the AST.
typedef enum {
    OT_NOTHING,  ///< No operand.
    OT_REGISTER, ///< Register operand.
    OT_MEMGLOB,  ///< Global memory operand.
    OT_MEMLOC,   ///< Local memory operand.
    OT_LABEL,    ///< Label operand.
    OT_VALUE     ///< Immediate value operand.
} AST_OperandType;


// -----------------------------------------------------------------------------
// Symbol table scope types
// -----------------------------------------------------------------------------

/// \brief Scope types for symbol table entries.
typedef enum {
    SCOPE_PROGRAM = 1, ///< Program or file-level scope.
    SCOPE_MODULE = 2,  ///< Module-level scope.
    SCOPE_FUNCTION = 3,///< Function-level (local) scope.
    SCOPE_DIRECT = 4   ///< Directive scope.
} SYM_ScopeType;


// -----------------------------------------------------------------------------
// Symbol table variable types
// -----------------------------------------------------------------------------

/// \brief Variable types stored in the symbol table.
typedef enum {
    V_VALUE,      ///< Constant value.
    V_MEMGLOBAL,  ///< Global memory reference.
    V_MEMLOCAL,   ///< Local memory reference.
    V_LABEL       ///< Label reference.
} SYM_VarType;


// -----------------------------------------------------------------------------
// Binary instruction node types
// -----------------------------------------------------------------------------

/// \brief Classification of binary instruction presence in a source node.
typedef enum {
    B_NOBIN,    ///< No binary instruction.
    B_BIN,      ///< Binary instruction present.
    B_BINCHILD, ///< Child binary instruction present.
} BIN_Type;


// -----------------------------------------------------------------------------
// Program types
// -----------------------------------------------------------------------------

/// \brief Program classification.
typedef enum {
    P_UNDEFINED,  ///< Undefined program type.
    P_STANDALONE, ///< Standalone program.
    P_MODULE,     ///< Module program.
} ProgramType;


// -----------------------------------------------------------------------------
// Opcodes
// -----------------------------------------------------------------------------

/// \brief Enumeration of supported opcodes.
/// \details
/// Each opcode represents a distinct machine instruction supported by ASM32.
typedef enum {
    ADD, ADC, ADDIL, AND,
    B, BE, BR, BRK,
    BV, BVE, CBR, CBRU,
    CMP, CMPU, CMR, DEP,
    DIAG, DS, DSR, EXTR,
    GATE, ITLB, LD, LDA,
    LDIL, LDO, LDPA, LDR,
    LSID, MR, MST, OR,
    PCA, PRB, PTLB, RFI,
    SBC, SHLA, ST, STA,
    STC, SUB, XOR
} Opcodes;


// -----------------------------------------------------------------------------
// Directives
// -----------------------------------------------------------------------------

/// \brief Enumeration of assembler directives.
typedef enum {
    D_ALIGN,
    D_BEGIN,
    D_BUFFER,
    D_BYTE,
    D_CODE,
    D_DATA,
    D_EQU,
    D_END,
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


// -----------------------------------------------------------------------------
// Directive table
// -----------------------------------------------------------------------------

/// \brief Table of supported assembler directives.
const struct directInfo {
    char directive[12]; ///< Directive name.
    int  directNum;     ///< Associated directive enumeration value.
} dirCodeTab[] = {
    { "ALIGN" ,       D_ALIGN },
    { "BEGIN" ,       D_BEGIN },
    { "BUFFER",       D_BUFFER },
    { "BYTE",         D_BYTE },
    { "CODE",         D_CODE },
    { "DATA" ,        D_DATA },
    { "EQU" ,         D_EQU },
    { "END" ,         D_END },
    { "HALF" ,        D_HALF },
    { "EXPORT" ,      D_EXPORT},
    { "IMPORT" ,      D_IMPORT},
    { "REG" ,         D_REG },
    { "STRING" ,      D_STRING },
    { "WORD" ,        D_WORD },
    { "DOUBLE",       D_DOUBLE },
    { "GLOBAL" ,      D_GLOBAL },
    { "PROGRAM" ,     D_PROGRAM },
    { "MODULE" ,      D_MODULE },
    { "ENDMODULE" ,   D_ENDMODULE },
    { "FUNCTION" ,    D_FUNCTION },
    { "ENDFUNCTION" , D_ENDFUNCTION }
};


// -----------------------------------------------------------------------------
// Reserved words
// -----------------------------------------------------------------------------

/// \brief Table of reserved words that cannot be used as identifiers.
const struct reservedInfo {
    char resWord[12]; ///< Reserved word string.
} resWordTab[] = {
    { "IF" }, { "ELSE" },
    { "R0" }, { "R1" }, { "R2" }, { "R3" },
    { "R4" }, { "R5" }, { "R6" }, { "R7" },
    { "R8" }, { "R9" }, { "R10" }, { "R11" },
    { "R12" }, { "R13" }, { "R14" }, { "R15" },
    { "R16" },
};


// -----------------------------------------------------------------------------
// Opcode table
// -----------------------------------------------------------------------------

/// \brief Table of opcode mnemonics and binary encodings.
/// \details
/// Each entry contains:
/// - The mnemonic string (e.g., "ADD").
/// - The 32-bit binary instruction pattern.
/// - The associated enum value for instruction classification.
const struct opCodeInfo {
    char        mnemonic[8]; ///< Instruction mnemonic.
    uint32_t    binInstr;    ///< 32-bit binary instruction encoding.
    int         instrType;   ///< Associated opcode enumeration value.
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
    { "CBR",     0x98000000 , CBR },
    { "CBRU",    0x9C000000 , CBRU },
    { "CMP",     0x5C020000 , CMP },
    { "CMPB",    0x5C000000 , CMP },
    { "CMPH",    0x5C010000 , CMP },
    { "CMPW",    0x5C020000 , CMP },
    { "CMPU",    0x60020000 , CMPU },
    { "CMPUB",   0x60000000 , CMPU },
    { "CMPUH",   0x60010000 , CMPU },
    { "CMPUW",   0x60020000 , CMPU },
    { "CMR",     0x24000000 , CMR },
    { "DEP",     0x18000000 , DEP },
    { "DIAG",    0xF8000000 , DIAG },
    { "DS",      0x30000000 , DS },
    { "DSR",     0x1C000000 , DSR },
    { "EXTR",    0x14000000 , EXTR },
    { "GATE",    0x84000000 , GATE },
    { "ITLB",    0xEC000000 , ITLB },
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
