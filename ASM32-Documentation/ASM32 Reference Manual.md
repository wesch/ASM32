# ASM32 Assembler Reference Manual



Wolfgang Eschenfelder

Version A.00.03.20  February 2025


- [ASM32 Assembler Reference Manual](#asm32-assembler-reference-manual)
  - [Introduction](#introduction)
  - [Assembler Features](#assembler-features)
  - [Program Structure](#program-structure)
  - [Symbols and Constants](#symbols-and-constants)
  - [Register and Register Mnemonics](#register-and-register-mnemonics)
    - [General Register Mnemonics](#general-register-mnemonics)
    - [Segment Register Mnemonics](#segment-register-mnemonics)
  - [Expressions](#expressions)
  - [Parenthesized Subexpressions](#parenthesized-subexpressions)
- [Scope](#scope)
- [Assembler Directives](#assembler-directives)
  - [.BYTE Directive](#byte-directive)
  - [.END Directive](#end-directive)
  - [.EQU Directive](#equ-directive)
  - [.HALF Directive](#half-directive)
  - [.REG Directive](#reg-directive)
  - [.WORD Directive](#word-directive)
- [Assembler Processing](#assembler-processing)
  - [Main Program](#main-program)
    - [DBG\_TOKEN](#dbg_token)
    - [DBG\_PARSER](#dbg_parser)
    - [DBG\_GENBIN](#dbg_genbin)
    - [DBG\_SYMTAB](#dbg_symtab)
    - [DBG\_AST](#dbg_ast)
    - [DBG SOURCE](#dbg-source)
    - [DBG\_DIS](#dbg_dis)
  - [Lexer](#lexer)
  - [Parser](#parser)
    - [ParseLabel](#parselabel)
    - [ParseDirective](#parsedirective)
    - [ParseInstruction](#parseinstruction)
      - [ParseExpression](#parseexpression)
    - [Codegen](#codegen)


## Introduction 

This document describes the use of the ASM32 Assembler for the VCPU-32. The ASM32 Assembly Language represents machine language instructions symbolically, and permits declaration of addresses symbolically as well. The Assembler's function is to translate an assembly language program, stored in a source file, into machine language.

As the concept for the VCPU-32 is heavily influenced by Hewlett Packards PA_RISC architecture, the following reference manual follows the structure of the PA RISC Assmebler manual.

For details about the VCPU-32 architecture please refer to 

_VCPU-32 System Architecture and instructions Set Reference_


## Assembler Features

The Assembler provides a number of features to make assembly language programming convenient. These features include:

- **Mnemonic Instructions:**  
Each machine instruction is represented by a mnemonic operation code, which is easier to remember than the binary machine language operation code. The operation code, together with operands, directs the Assembler to output a binary machine instruction to the object file.

- **Symbolic Addresses:**
You can select a symbol to refer to the address of a location in virtual memory. The address is often referred to as the value of the symbol, which should not be confused with the value of the memory locations at that address. 

- **Symbolic Constants:**  
A symbol can also be selected to stand for an integer constant.

- **Expressions:**  
Arithmetic expressions can be formed from symbolic constants, integer constants, and arithmetic operators. Expressions involving only symbolic and integer constants, defined in the current module, are called Symbolic Constants. They can be used wherever an integer constant can be used. 


- **Storage Allocation:**  
In addition to encoding machine language instructions symbolically, storage may be initialized to constant values or simply reserved. Symbolic addresses and labels can be associated with these memory locations.


## Program Structure

An assembly language program is a sequence of statements. 

Each statement contains up to four fields:
- Label
- Opcode or directive
- Operands
- Comments

The operands field cannot appear without an opcode field. 

There are three classes of statements:

**Instructions**  
represent a single machine instruction in symbolic form. 

**Directives**   
communicate information about the program to the Assembler or cause the Assembler to initialize or reserve one or more words of storage for data

The **label** field is used to associate a symbolic address with an instruction or data location, or to define a symbolic constant using the .EQU, or .REG directives. This field is optional for all but a few statement types. If a label appears on a line by itself, or with a comment only, the label is associated with the next address within the same subspace and location counter.

A **comment** is starting with a **;**. All text after the comment until end of line is ignored by the assembler.

## Symbols and Constants

Both addresses and constants can be represented symbolically. Labels represent a symbolic address except when the label is on an .EQU or .REG  directive. If the label is on an .EQU or .REG directive, the label represents a symbolic constant. The Label needs to end with a ":".

Symbols are composed of 

- uppercase and lowercase letters (A-Z and a-z)

- decimal digits (0-9)


A symbol needs to begin with a letter.

The Assembler considers uppercase and lowercase letters in symbols _not distinct_. The mnemonics for operation codes, directives, and pseudo-operations can be written in either case. 

The length of a symbol name is restricted to 32. The name of a symbol needs to be unique within a scope, This means it can not occur twice or more within an adressable range. 

Integer constants can be written in decimal, octal, or hexadecimal notation, as in the C language.

## Register and Register Mnemonics

The VCPU-32 features 3 types of registers. 

- 16 general register R0 - R15

- 8 segment register S0 - S7

- 32 control register C0 - C31

Data is loaded from memory into general registers and stored into memory from general registers. Arithmetic and logical operations are performed on the contents of the general registers.

Some additional predefined register mnemonics are provided based on the standard procedure-calling convention.

### General Register Mnemonics

| Register | Mnemonic | Description |
| -------- | ----------------- | ----------- |
|  R9 | ARG3 ||
| R10 | ARG2 ||
| R11 | ARG1 ||
| R12 | ARG0 ||
| R13 | DP | Global Data |
| R14 | RL | Return Link |
| R15 | SP | Stack Pointer |

### Segment Register Mnemonics

| Register | Mnemonic | Description |
| -------- | ----------------- | ----------- |
| S5 | TS | Task Segment |
| S6 | JS | Job Segment |
| S7 | SS | System Segment |

## Expressions

Arithmetic expressions are often valuable in writing assembly code. The Assembler allows expressions involving integer constants and symbolic constants. These terms can be combined with the standard arithmetic operators. 


| Operator | Operation |
| - | - |
| + | Integer addition |
| - | Integer subtraction |
| * | Integer multiplication |
| / | Integer division (result is truncated) |
| L% | Left 22bit |
| R% | Right 10 bit |

## Parenthesized Subexpressions

The constant term of an expression may contain parenthesized subexpressions that alter the order of evaluation from the precedence normally associated with arithmetic operators.

For example 

    LDI R5, data5 + ( data7 - 4 ) * data0

# Scope

An assembler program has 4 Scope levels:

- Global Scope is the root of all scopes. 
- Program Scope contains all information of a single source code file.
- Module Scope is the next level unit within a source file (Program Scope).
- Function Scope is the next level beyond the Module Scope.

![b9b07d9cb0ef713995c30eb8cb4f8d4d.png](./b9b07d9cb0ef713995c30eb8cb4f8d4d.png)


Every source file passed to the assembler defines a program scope.

The directives .MODULE and .FUNCTION determine the scope level with a program.

The symbol table is organized according to the scope level. If a new symbol is defined it must be unique within the current scope. Reference to the symbol walks along the scope tree upwards. E.g. is a symbol is referenced on function scope, it is searched on function scope level. If it is not found it is searched on module scope level. If it is not found on module level, it is searched on program scope level. 

Currently no symbols on global level are supported. 


# Assembler Directives

Assembler directives allow to take special programming actions during the assembly process. The directive names begin with a period (.) to distinguish them from machine instruction opcodes.



| Directive | Function |
| - | - |
| .BYTE | Reserves 8 bits (a byte) of storage and initializes it to the given value. |
| .END | Terminates an assembly language program. |
| .EQU | Assigns an expression to an identifier. |
| .HALF | Reserves 16 bits (a half word) of storage and initializes it to the given value. |
| .REG | Attaches a type and number to a user-defined register name.|
| .WORD | Reserves 32 bits (a word) of storage and initializes it to the given value. |
| .PROGRAM .ENDPROGRAM | Start and end of a Source Program scope |
| .MODULE .ENDMODULE | Start and end of Module scope |
| .FUNCTION .ENDFUNCTION | Start and end of Function scope |


Symbolic names must not be equal to a reserved word. 


## .BYTE Directive

Reserves 8 bits (a byte) of storage and initializes it to the given value.

**Syntax**

    symbolic_name  .BYTE [init_value]

**Parameter**   symbolic_name

The name of the identifier to which the Assembler assigns to the byte.

**Parameter**   init_value

Either a decimal or hexadecimal number. If you omit the initializing value, the Assembler initializes the area to zero.



## .END Directive

Terminates an assembly language program.

**Syntax**

    .END 


This directive is the last statement in an assembly language program. If a source file lacks an .END directive, the Assembler terminates the program when it encounters the end of the file.

## .EQU Directive

Assigns an expression to an identifier. It must be defined before it is referenced.

**Syntax**

    symbolic_name    .EQU    value


**Parameter**   symbolic_name

The name of the identifier to which the Assembler assigns the expression.

**Parameter**  value

An integer expression. The Assembler evaluates the expression, which must be absolute, and assigns this value to symbolic_name.

**NOTE**

The Assembler prohibits the use of relocatable symbols (instruction labels) and imported symbols as components of an .EQU expression. 

Nested EQU are not allowed. E.g.  V1  .EQU 5, V2 .EQU V1.

## .HALF Directive

Reserves 16 bits (a half word) of storage and initializes it to the given value.

**Syntax**

    symbolic_name  .HALF [init_value]

**Parameter**   symbolic_name

The name of the identifier to which the Assembler assigns to the half-byte.

**Parameter**   init_value

Either a decimal or hexadecimal number. If you omit the initializing value, the Assembler initializes the area to zero.

## .REG Directive

Attaches a type and number to a user-defined register name. It must be defined before it is referenced.

**Syntax**

    symbolic_name    .REG [register]

**Parameter**   symbolic_name 

A user-defined register name.

**Parameter**   register

Must be one of the predefined Assembler registers (R0-R15,S0-S7,....)

Nested REG are not allowed. E.g.  SP1 .REG R5, SP2  .REG SP1.

## .WORD Directive

Reserves 32 bits (a  word) of storage and initializes it to the given value.

**Syntax**

    symbolic_name  .WORD [init_value]

**Parameter**   symbolic_name

The name of the identifier to which the Assembler assigns to the word.3

**Parameter**   init_value

Either a decimal or hexadecimal number. If you omit the initializing value, the Assembler initializes the area to zero.

# Assembler Processing

The assembler processes the sourcecode within the following steps:

- Main program
- Lexer
- Parser
- Codegen

## Main Program

The main program establishes the overall environment, opens the source file, reads the sourcline and passes into the lexer.

In parallel it builds a tree structure (SRCNode) global->program->sourceline as a base for printing sourclines, binary instructions and errormessages

The main programm contains a couple of DEBUG Switches, which provide additonal listings.
Currently they are implemented as global variables which can be set to TRUE or FALSE.

### DBG_TOKEN 
Prints the list of tokens generated by the lexer

### DBG_PARSER
Prints details of the parsing process

### DBG_GENBIN
Prints details of codegen process

### DBG_SYMTAB
Prints the symbol table

### DBG_AST
Prints the abstract syntax tree.

### DBG SOURCE
Prints the following information:

Address | maschine code | lineNr source | Source Text

 0000 4042005e   25     BEG:    ADD             R1,47

 0004 40460023   26                     ADD             GREG1,R2,R3

 0008 408a0034   27                     ADD             R2,R3(R4)

all errors during assembly are printed after the source line

### DBG_DIS

Prints input lines for the VCPU32 disassembler:

w disasm (0x4042005e) # line 25  BEG:   ADD             R1,47

w disasm (0x40460023) # line 26                 ADD             GREG1,R2,R3

w disasm (0x408a0034) # line 27                 ADD             R2,R3(R4)

## Lexer

The Lexer reads the source line and extracts the tokens and provides the tokenList. This is a linked list with all tokens, sourcline number and column.
During processing the lexer converts **hexadecimal values to decimal** values and masks L% and R% values accordingly.

## Parser

The Parser reads the tokenList and generates the symbol table (SymNode) and the abstract syntax tree (ASTNode). The parser consists of mutliple subprocessing routines

for label (ParseLabel), directives (ParseDirectives) and instructions (Parseinstruction).

All tokens are translated to uppercase to ensure a commen naming inised symboltable etc.

### ParseLabel

This is the most simple routing which just moves a valid label token into a field named label. It will be processes with the ParseInstruction  or ParseDirective routine.

### ParseDirective

The tokens identified as directives (starting with a .) are checked against a table dirCodeTab. If the directive is valid the appropriate processing takes place.

For each directive an entry in the symbol table is created. **MODULE** adn **FUNCTON** directives and their corresponding end functions (ENDFUNCTION) build scope levels withinn the symbol table tree.

**SymbolTable**
- scope type
- scope level   -> global=0, program=1, module=2, function=3
- scope name    
- label         
- function      -> directive (without .)    
- value          
- variable type 
- linenr
- code address
- data address
- pointer to children nodes
- number of children nodes

**Scope types:**
- SCOPE_GLOBAL
- SCOPE_PROGRAM
- SCOPE_MODULE
- SCOPE_FUNCTION
- SCOPE_DIRECT

**Variable Type**
- Value
- Memory global
- Memory local
- Label

**.MODULE and .FUNCTION processing**

If a .MODULE directive is detected, scopetype is set to SCOPE_MODULE, dataAdr is set to zero. If .ENDMODULE is detected, scope level is set back to SCOPE_PROGRAM. 
Analog for .FUNCTION.

**.EQU and .REG processing**
For both directives it is checked if a directive with the same name (label) is already existing on the same scope level. If not, then the Symbol is inserted in the symboltable.

**.BYTE, .HALF and .WORD processing**

Depending of the scope level (module or function) the variable type is set to Memory global or Memory local. 
It is checked if a directive with the same name (label) is already existing on the same scope level. If not, then the Symbol is inserted in the symboltable.
Furthermore .WORD is aligend to word address, .HALF is aligned to halfword address.

### ParseInstruction

The tokens identified as instructions are checked against a table opCodeTab. If the opcoode is valid the further processing takes place in base of the type of the opcode. An entry in the opCodeTab assigns every opcode to an OpType. This Optype determines smae parsing procedure for a goup of opcodes which have the same mnemonic structure.

For every group a dedicated parsing routine is in place. In these routines every token is analyzed. if the token is an expected token an entry into the abstract systax tree (AST) is created.

**AST** 
- node type
- value in character of the entry
- numeric value of the entry if available
- linenr of the source line
- column number of the token
- scopelevel (program, module, function)
- name of the scope
- adress of the corresponging node of the symbol table for easier search function in the symbol table.
- operand type to define the type of the value (register, memorylocation, label, value)
- address of code 
- binary word of the instruction derived from opCodeTab.
- pointer to children nodes
- number of children nodes

#### ParseExpression

Expressions are recursively parsed. ParseExpression calls **ParseTerm** and calculates plus, minus, or, xor operations.

**ParseTerm** calls **ParseFactor** and calculates multiply, division, modulo, and operations.

**ParseFactor** processes the numbers, substitutes the EQU or global/local offsets. 


### Codegen

The codegen routine reads the **AST** sequentially, calculates code address and populates the binary word of the instruction.

The procedure GenBinOption checks the options by groups of instructions with the same options.

For instructions using a memory location the memory type can be:

- Memory global (in module)
    offset is calculated as the data offset within the module and register R13. 
- Memory local (in function)
    tbd