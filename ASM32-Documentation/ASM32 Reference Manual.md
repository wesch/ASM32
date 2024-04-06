# ASM32 Assembler Reference Manual



Wolfgang Eschenfelder

Version A.00.01   April 2024



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
Arithmetic expressions can be formed from symbolic addresses and constants, integer constants, and arithmetic operators. Expressions involving only symbolic and integer constants, or the difference between two symbolic adresses, defined in the current module, are called Symbolic Constants. They can be used wherever an integer constant can be used. Expressions involving the sum or difference of a symbolic address and an absolute expression are called address expressions. The constant part of an expression, the part that does not refer to symbolic adresses, can use parenthesized subexpressions to alter operator precedence.



- **Storage Allocation:**  
In addition to encoding machine language instructions symbolically, storage may be initialized to constant values or simply reserved. Symbolic addresses and labels can be associated with these memory locations.


## Program Structure

An assembly language program is a sequence of statements. There are three classes of statements:

**Instructions**  
represent a single machine instruction in symbolic form. 


**Directives**   
communicate information about the program to the Assembler or cause the Assembler to initialize or reserve one or more words of storage for data


An assembly statement contains four fields:

- Label

- Opcode

- Operands

- Comments



Each of these fields is optional. However the operands field cannot appear without an opcode field. 



The **label** field is used to associate a symbolic address with an instruction or data location, or to define a symbolic constant using the .EQU, or .REG directives. This field is optional for all but a few statement types. If a label appears on a line by itself, or with a comment only, the label is associated with the next address within the same subspace and location counter.

A **comment** is starting with a **;**. All text after the comment until eond of line is ignored by the assembler.

## Symbols and Constants

Both addresses and constants can be represented symbolically. Labels represent a symbolic address except when the label is on an .EQU or .REG  directive. If the label is on an .EQU or .REG directive, the label represents a symbolic constant. 

Symbols are composed of 

- uppercase and lowercase letters (A-Z and a-z)

- decimal digits (0-9)

- dollar signs ($)

- underscores (_). 

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
| S5 | TS | Taks Segment |
| S6 | JS | Job Segment |
| S7 | SS | System Segment |



## Expressions



Arithmetic expressions are often valuable in writing assembly code. The Assembler allows expressions involving integer constants, symbolic constants, and symbolic addresses. These terms can be combined with the standard arithmetic operators. 



| Operator | Operation |
| - | - |
| + | Integer addition |
| - | Integer subtraction |
| * | Integer multiplication |
| / | Integer division (result is truncated) |



## Parenthesized Subexpressions



The constant term of an expression may contain parenthesized subexpressions that alter the order of evaluation from the precedence normally associated with arithmetic operators.



For example 



    LDI R5, data5 + ( data7 - 4 ) * data0





# Assembler Directives



Assembler directives allow you to take special programming actions during the assembly process. The directive names begin with a period (.) to distinguish them from machine instruction opcodes.



| Directive | Function |
| - | - |
| .ALIGN | Forces location counter to the next largest multiple of the supplied alignment value. |
| .BLOCK | Reserves a block of data storage |
| .BYTE | Reserves 8 bits (a byte) of storage and initializes it to the given value. |
| .CODE | Indicates the start of the instruction area |
| .DATA | Indicates the start of the data area |
| .END | Terminates an assembly language program. |
| .EQU | Assigns an expression to an identifier. |
| .HALF | Reserves 16 bits (a half word) of storage and initializes it to the given value. |
| .REG | Attaches a type and number to a user-defined register name.|
| .STRING | Reserves the appropriate amount of storage and initializes it to the given string.|
| .WORD | Reserves 32 bits (a word) of storage and initializes it to the given value. |





## .ALIGN Directive



Forces location counter to the next largest multiple of the supplied alignment value.



**Syntax**



    .ALIGN [boundary]



**Parameter** boundary



An integer value for the byte boundary to which you want to advance the location counter. The Assembler advances the location counter to that boundary. Permissible values must be a power of 2 and can range from one to 4096. The default value is 4.



## .BLOCK Directive



Reserves a block of data storage





**Syntax**



    symbolic_name  .BLOCK [num_bytes]



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns to the block.



**Parameter**   num_bytes



An integer value for the number of bytes you want to reserve. Permissible values range from zero to 0x3FFFFFFF. The default value is zero.



<### to be clarified>



## .BYTE Directive



Reserves 8 bits (a byte) of storage and initializes it to the given value.



**Syntax**



    symbolic_name  .BYTE [init_value]



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns to the byte.



**Parameter**   init_value



Either a decimal, octal, or hexadecimal number or a sequence of ASCII characters, surrounded by quotation marks. If you omit the initializing value, the Assembler initializes the area to zero.



## .CODE Directive



Indicates the start of the instruction area. After this directive  the LOCATION counter for instruction is active. The area ends either with a .DATA or an .END directive.





**Syntax**



     .CODE 

	

## .DATA Directive



Indicates the start of the data area. After this directive  ths the LOCATION counter for data is active. The area ends either with a .CODE or an .END directive.





**Syntax**



     .DATA 



## .END Directive



Terminates an assembly language program.





**Syntax**



    .END 



This directive is the last statement in an assembly language program. If a source file lacks an .END directive, the Assembler terminates the program when it encounters the end of the file.



## .EQU Directive



Assigns an expression to an identifier.





**Syntax**



    symbolic_name    .EQU    value



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns the expression.



**Parameter**  value



An integer expression. The Assembler evaluates the expression, which must be absolute, and assigns this value to symbolic_name.



**NOTE**



The Assembler prohibits the use of relocatable symbols (instruction labels) and imported symbols as components of an .EQU expression.



## .HALF Directive



Reserves 16 bits (a half word) of storage and initializes it to the given value.



**Syntax**



    symbolic_name  .HALF [init_value]



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns to the half-byte.



**Parameter**   init_value



Either a decimal, octal, or hexadecimal number or a sequence of ASCII characters, surrounded by quotation marks. If you omit the initializing value, the Assembler initializes the area to zero.



## .RECORD Directive



Defines a structure name where fields are part of the structure.



**Syntax**



      symbolic_name .RECORD 

	  

**Parameter**   symbolic_name 



A user-defined name of the structure. The structure is closed by  **.ENDRECORD**.



**Example**



	leaf    .RECORD 
		    .WORD   backward
		    .WORD   forward
		    .WORD   pointer
    	    .ENDRECORD   ; a data structure with 3 fields, 12 bytes





## .REG Directive



Attaches a type and number to a user-defined register name.



**Syntax**



    symbolic_name    .REG [register]



**Parameter**   symbolic_name 



A user-defined register name.



**Parameter**   register



Must be one of the predefined Assembler registers or a previously defined user-defined register name





## .STRING Directive



Reserves the appropriate amount of storage and initializes it to the given string. It appends a zero byte to the data. (C-Language-Style String)



**Syntax**



    symbolic_name .STRING ["init_value"]



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns to the string.



**Parameter**   init_value



A sequence of ASCII characters, surrounded by quotation marks. A string can contain up to 256 characters. The enclosing quotation marks are not stored.



## .WORD Directive



Reserves 32 bits (a  word) of storage and initializes it to the given value.



**Syntax**



    symbolic_name  .WORD [init_value]



**Parameter**   symbolic_name



The name of the identifier to which the Assembler assigns to the word.



**Parameter**   init_value



Either a decimal, octal, or hexadecimal number or a sequence of ASCII characters, surrounded by quotation marks. If you omit the initializing value, the Assembler initializes the area to zero.