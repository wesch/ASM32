#include "constants.hpp"
#include "ASM32.hpp"

/// \file
/// \brief AST reader and binary generator
/// \details
/// This module reads the abstract syntax tree (AST) and the symbol table,
/// translates instructions into binary form, and builds the final machine code
/// representation. It provides helper functions for bit manipulation,
/// register encoding, option parsing, and AST traversal.

// ============================================================================
// Bit Manipulation Helpers
// ============================================================================

/// \brief Set bits in the current instruction.
/// \details
/// Inserts a value into a specific bit range of the current binary instruction.
/// Performs range checks and signals errors if the input exceeds the allowed
/// number of bits.
/// 
/// \param pos Position in the instruction (0 = MSB, 31 = LSB).
/// \param x   Value to be inserted.
/// \param num Number of bits.

void setBit(int pos, int x, int num) {
    
    int limit = pow(2, num);
    if (x > (limit - 1) ||
        x < (-limit)) {

        snprintf(errmsg, sizeof(errmsg), "Value %d out of range", x);
        processError(errmsg);
    }
    int mask = pow(2, num) - 1;
    int val = (x & mask);
    binInstr = binInstr | val << (31 - pos);
}

/// \brief Clear a single bit in the current instruction.
/// \param pos Position in the instruction (0 = MSB, 31 = LSB).
void clrBit(int pos) {

    binInstr &= ~(1 << (31 - pos));

}

/// \brief Set an instruction offset field.
/// \details
/// Handles immediate offsets and generates additional instructions if the
/// offset exceeds the field width. Supports special handling for offsets larger
/// than the hardware limit.
/// 
/// \param pos Position in the instruction.
/// \param x   Offset value.
/// \param num Number of bits available for encoding.
void setOffset(int pos, int x, int num) {
    
//   x = 4096;   // Test for large offset
    
    int limit = pow(2, num);
    if (x > (limit - 1) ||
        x < (-limit)) {
        // aktuelle Instruction speichern
        // neue instruction ADDIL DP,L%limit -> 0x0B400000 | 0xFFFFFC00 & x
        // gespeicherte Instruction offset = R%limit Instruktion ->  SetBit(26, 0x3FF & x, 11);   
        // regB = R1 -> SetRegister('B', opchar[2]);
        // opNum[1] = 0 negative offset 
        binInstrSave = binInstr;
        binInstr = 0x0B400000 | (0xFFFFFC00 & x);
        bin_status = B_BINCHILD;
        strcpy(infmsg, "generated Instruction: ADDIL DP,L%limit due to offset > limit\n");
        createBinary();   

        codeAdr = codeAdr + 4;
        binInstr = binInstrSave;
        binInstr = binInstr & 0xFFFFFFF0;
        strcpy(opchar[2], "R1");
        strcpy(infmsg, "modified Instruction offset regB -> R1, R%limit \n");

        setGenRegister('B', opchar[2]);

        opnum[1] = 0;
    }
    num = x & 0x3FF;
    setBit(26, num, 11);


}
// ============================================================================
// Register Encoding Helpers
// ============================================================================

/// \brief Encode a general-purpose register.
/// \details
/// Encodes register operands (R, A, or B) into the correct instruction fields.
/// Only values between 1 and 15 are valid.
/// 
/// \param reg     Register type ('R', 'A', or 'B').
/// \param regname Name of the register, e.g., "R12".

void setGenRegister(int reg, char* regname) {

    int x = strlen(regname);
    char t[2];
    if (x == 2) {
    
        t[0] = regname[1];
        value = t[0] - 48;
    }
    else {
        
        t[0] = regname[1];
        t[1] = regname[2];
        value = 10 * (t[0] - 48) + (t[1] - 48);
    }
    if (value < 1 || value > 15) {
        
        snprintf(errmsg, sizeof(errmsg), "General register # %d out of range", (int)value);
        processError(errmsg);
        return; 
    }
    switch (reg) {
    case 'R':   setBit(9, value,4); break;
    case 'A':   setBit(27, value,4); break;
    case 'B':   setBit(31, value,4); break;
    }

}

/// \brief Encode a special register for MR instructions.
/// \details
/// Supports Segment (S1–S7) and Control (C1–C31) registers. Validates
/// register number ranges and signals errors for invalid names.
/// 
/// \param regname Register name, e.g., "S1" or "C12".
void setMRRegister(char* regname) {

    int x = strlen(regname);
    char t[2];
    if (x == 2) {

        t[0] = regname[1];
        value = t[0] - 48;
    }
    else {

        t[0] = regname[1];
        t[1] = regname[2];
        value = 10 * (t[0] - 48) + (t[1] - 48);
    }
    if (regname[0] == 'S') {
        if (value < 1 || value > 7) {

            snprintf(errmsg, sizeof(errmsg), "Segment register # %d out of range", (int ) value);
            processError(errmsg);
            return;
        }
    }
    else if (regname[0] == 'C') {
        if (value < 1 || value > 31) {

            snprintf(errmsg, sizeof(errmsg), "Control register # %d out of range", (int ) value);
            processError(errmsg);
            return;
        }
    }
    setBit(31, value, 5);
}

/// \brief Retrieve a segment register number.
/// \details
/// Extracts the numeric part from a segment register name (S1–S7).
/// Returns 0 and signals an error if the value is out of range.
/// 
/// \param regname Segment register name (e.g., "S1").
/// \return The numeric value (1–7) or 0 on error.

int getSegRegister(char* regname) {

    int x = strlen(regname);
    char t[2];
    if (x == 2) {

        t[0] = regname[1];
        value = t[0] - 48;
    }
    else {

        t[0] = regname[1];
        t[1] = regname[2];
        value = 10 * (t[0] - 48) + (t[1] - 48);
    }
    if (value < 1 || value > 7) {
        snprintf(errmsg, sizeof(errmsg), "Segment register # %d out of range", (int)value);
        processError(errmsg);
        return 0;
    }


    return value;

}




// ============================================================================
// Binary Generation
// ============================================================================

/// \brief Generate the option part of a binary instruction.
/// \details
/// Parses instruction options (e.g., CMP conditions, AND/OR modifiers)
/// and sets the corresponding bits. Performs validation to prevent
/// invalid combinations or missing options.

void genBinOption() {
    // printf("adr: %04x binInstr %08x, Optype %d, Option %s %s Opchar %s %s %s Opnum %d %d %d Mode %d\t", codeAdr, binInstr, OpType, option[0], option[1], opchar[0], opchar[1], opchar[2], opnum[0], opnum[1], opnum[2], mode);


    int count = 0;

    // process option
    switch (opInstrType) {

    case ADD:
    case ADC:
    case SBC:
    case SHLA:
    case SUB:

        
        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'L':   setBit(10, 1, 1); break;
            case 'O':   setBit(11, 1, 1); break;
            default:    snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]); 
                processError(errmsg);
            }
        }
        break;

    case AND:
    case OR:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'N':
            case 'C':   count++;
            }
            if (count > 1) {
                snprintf(errmsg, sizeof(errmsg), "Option N and C not at the same time");
                processError(errmsg);
                break;
            }
        }
        for (int j = 0; j < strlen(option[0]); j++) {
            switch (option[0][j]) {

            case 'N':   setBit(10, 1, 1); break;
            case 'C':   setBit(11, 1, 1); break;
            default:    snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);    
                processError(errmsg);
            }
        }
        break;

    case CMP:
    case CMPU:

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        }
        else if (option[0][0] == 'L' && option[0][1] == 'T') {
            setBit(11, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            setBit(10, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            setBit(10, 1, 1);
            setBit(11, 1, 1);
        }
        else {
            if (option[0][0] == '\0') {
                snprintf(errmsg, sizeof(errmsg), "Missing Option for CMP/CMPU");
                processError(errmsg);
                break;
            }
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            processError(errmsg);
        }
        break;

    case CBR:
    case CBRU:

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        } else if (option[0][0] == 'L' && option[0][1] == 'T') {
            setBit(7, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            setBit(6, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            setBit(6, 1, 1);
            setBit(7, 1, 1);
        }
        else {
            if (option[0][0] == '\0') {
                snprintf(errmsg, sizeof(errmsg), "Missing Option for CBR/CBRU");
                processError(errmsg);
                break;
            }
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            processError(errmsg);
        }
        break;

    case XOR:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'N':
                setBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;



    case CMR:

        if (strcmp(option[0],"") == 0) {
            break;
        }

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        }
        else if (option[0][0] == 'L' && option[0][1] == 'T') {
            setBit(13, 1, 1);
        }
        else if (option[0][0] == 'G' && option[0][1] == 'T') {
            setBit(12, 1, 1);
        }
        else if (option[0][0] == 'E' && option[0][1] == 'V') {
            setBit(12, 1, 1);
            setBit(13, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            setBit(11, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            setBit(11, 1, 1);
            setBit(13, 1, 1);
        }
        else if (option[0][0] == 'G' && option[0][1] == 'E') {
            setBit(11, 1, 1);
            setBit(12, 1, 1);
        }
        else if (option[0][0] == 'O' && option[0][1] == 'D') {
            setBit(11, 1, 1);
            setBit(12, 1, 1);
            setBit(13, 1, 1);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            processError(errmsg);
        }
        break;


    case EXTR:

        mode = 0;
        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'S':
                setBit(10, 1, 1);
                break;

            case 'A':
                setBit(11, 1, 1);
                mode = 1;
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case PCA:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                setBit(10, 1, 1);
                break;

            case 'M':
                setBit(11, 1, 1);
                break;

            case 'F':
                setBit(14, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case PTLB:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                setBit(10, 1, 1);
                break;

            case 'M':
                setBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case ITLB:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                setBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case DSR:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'A':
                setBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case MR:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'D':
                setBit(10, 1, 1);
                break;

            case 'M':
                setBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case MST:

        switch (option[0][0]) {

        case ' ':

            break;

        case 'S':
            setBit(11, 1, 1);
            break;

        case 'C':
            setBit(10, 1, 1);
            break;

        default:
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][0]);
            processError(errmsg);
        }
        break;


    case PRB:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'W':
                setBit(10, 1, 1);
                break;

            case 'I':
                setBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case DEP:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'Z':
                setBit(10, 1, 1);
                break;

            case 'A':
                setBit(11, 1, 1);
                break;

            case 'I':
                setBit(12, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                processError(errmsg);
            }
        }
        break;

    case LD:
    case LDA:
    case ST:

        for (int j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'M':

                setBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c ", option[0][j]);
                processError(errmsg);
            }
        }
        break;
    }
    return;
}

/// \brief Generate the binary encoding of an instruction.
/// \details
/// Based on the current operation type, mode, operands, and options,
/// this function encodes the full machine instruction into binary form.
/// Includes handling for immediates, registers, memory operands, labels,
/// and special instructions.

void genBinInstruction() {
    switch (opInstrType) {

    /// @par ADD,ADC,AND,CMP,CMPU,OR,SBC,SUB,XOR
    ///     - OP<.XX> regR, value (17bitS)          ->  mode 0
    ///     - OP<.XX> regR, regA, regB              ->  mode 1
    ///     - OP[W | H | B]<.XX> regR, regA(regB)   ->  mode 2
    ///     - OP[W | H | B]<.XX> regR, ofs(regB)    ->  mode 3

    case ADD:
    case ADC:
    case AND:
    case CMP:
    case CMPU:
    case OR:
    case SBC:
    case SUB:
    case XOR:
        if (mode == 0) {

            setGenRegister('R', opchar[0]);
            setBit(31, opnum[1], 18);
            // no BYTE,HALF,WORD
            clrBit(15);
            clrBit(14);
            setBit(31, opnum[1], 18);
        }
        else if (mode == 1) {
 
            if (strcmp(opchar[0],opchar[2]) == 0) {
                setGenRegister('R', opchar[0]);
                setGenRegister('B', opchar[1]);
                setGenRegister('A', opchar[2]);
            }
            else {
                setGenRegister('R', opchar[0]);
                setGenRegister('A', opchar[1]);
                setGenRegister('B', opchar[2]);
            }
            setBit(13, 1,1);
            // no BYTE,HALF,WORD
            clrBit(15);
            clrBit(14);
            setBit(31, opnum[1], 18);
        }
        else if (mode == 2) {

            setGenRegister('R', opchar[0]);
            setGenRegister('A', opchar[1]);
            setGenRegister('B', opchar[2]);
            setBit(12, 1,1);
        }
        else if (mode == 3) {

            setGenRegister('R', opchar[0]);
            if (operandTyp[1] == OT_MEMGLOB) {

                strcpy(opchar[2], "R13");
                setGenRegister('B', opchar[2]);
                setOffset(27, opnum[1], 12);

            } else if (operandTyp[1] == OT_MEMLOC) {

                strcpy(opchar[2], "R15");
                setGenRegister('B', opchar[2]);
                setOffset(27, opnum[1], 12);

            } else if (operandTyp[1] == OT_VALUE) {

                setGenRegister('B', opchar[2]);
                setBit(27, opnum[1], 12);           // offset
            }
            setBit(12, 1,1);
            setBit(13, 1,1);
        }
        break;

    /// @par ADDIL,LDIL
    ///    - OP regR, value (22bitU)

    case ADDIL:
    case LDIL:
        setGenRegister('R', opchar[0]);
        setBit(31, opnum[1], 22);
        break;

    case B:
  
        if (operandTyp[0] == OT_VALUE) {

            if ((opnum[0] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[2]);
                processError(errmsg);
            }
            else
            {
                value = opnum[0] >> 2;
                setBit(31, value, 22);
            }
        }

        if (operandTyp[0] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            searchSymLevel(currentSymSave, opchar[0], 0);
            if (symFound == TRUE) {

                value = symcodeAdr - codeAdr + 4;

                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[0]);
                    processError(errmsg);
                }
                value = value >> 2;
                setBit(31, value, 22);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                processError(errmsg);
            }
        }
        if (DBG_GENBIN == TRUE) {
            printf("B line %03d addroffset= %d\n", lineNr, (int) value);
        }

        if (strcmp(opchar[1], "") != 0) {

            setGenRegister('R', opchar[1]);
        }


        break;

    case GATE:
        
        setGenRegister('R', opchar[0]);

        if (operandTyp[1] == OT_VALUE) {

            if ((opnum[1] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[1]);
                processError(errmsg);
            }
            else
            {
                value = opnum[1] >> 2;
                setBit(31, value, 22);
            }
        }

        if (operandTyp[1] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            searchSymLevel(currentSymSave, opchar[1], 0);
            if (symFound == TRUE) {
                value = symcodeAdr - codeAdr + 4;


                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[1]);
                    processError(errmsg);
                }
                value = value >> 2;
                setBit(31, value, 22);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                processError(errmsg);
            }
        }
        if (DBG_GENBIN == TRUE) {
            printf("GATE line %03d addroffset= %d\n", lineNr, (int) value);
        }
        break;

    case BR:
    case BV:

        setGenRegister('B', opchar[0]);
        if (strcmp(opchar[1], "") != 0) {
            setGenRegister('R', opchar[1]);
        }
        break;


    case CBR:
    case CBRU:

        setGenRegister('A', opchar[0]);
        setGenRegister('B', opchar[1]);

        if (operandTyp[2] == OT_VALUE) {
            if ((opnum[2] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[2]);               
                processError(errmsg);
            }
            else
            {
                value = opnum[2] >> 2;
                setBit(23, value, 16);
            }
        }

        if (operandTyp[2] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            searchSymLevel(currentSymSave, opchar[2], 0);
            if (symFound == TRUE) {
                value = symcodeAdr - codeAdr + 4 ;

                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[2]);
                    processError(errmsg);
                }
                value = value >> 2;
                setBit(23, value, 16);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                processError(errmsg);
            }
        }
        if (DBG_GENBIN) {

            printf("CBR/CBRU line %03d addroffset= %d\n", lineNr, (int)value);
        }
        
        break;

    case BVE:
        setGenRegister('A', opchar[0]);
        setGenRegister('B', opchar[1]);
        if (strcmp(opchar[2],"") != 0) {


            setGenRegister('R', opchar[2]);
        }
        break;

    case EXTR:

        setGenRegister('R', opchar[0]);
        setGenRegister('B', opchar[1]);
        if (mode == 0) {
            setBit(27, opnum[2], 5);
            setBit(21, opnum[3], 5);
        }
        else {
            setBit(21, opnum[2], 5);
        }
        break;

    case DEP:
        if (mode == 0) {

            setGenRegister('R', opchar[0]);
            setGenRegister('B', opchar[1]);
            setBit(27, opnum[2], 5);
            setBit(21, opnum[3], 5);
        }
        else if (mode == 1) {

            setGenRegister('R', opchar[0]);
            setGenRegister('B', opchar[1]);
            setBit(21, opnum[2], 5);
        }
        else if (mode == 2) {

            setGenRegister('R', opchar[0]);
            setBit(31, opnum[1], 4);
            setBit(27, opnum[2], 5);
            setBit(21, opnum[3], 5);
        }
        else if (mode == 3) {

            setGenRegister('R', opchar[0]);
            setBit(31, opnum[1], 4);
            setBit(21, opnum[2], 5);
        }
        break;


    case LDR:
    case STC:

        setGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            setBit(27, opnum[1], 12);
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            setGenRegister('B', opchar[2]);
            setOffset(27, opnum[1], 12);
        }
        if (opchar[2][0] == 'S') {
            value = getSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                setBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", (int)value);
                processError(errmsg);
                break;
            }
            setGenRegister('B', opchar[3]);
        }
        else {
            setGenRegister('B', opchar[2]);
        }
        break;

    case BE:

        if (operandTyp[0] == OT_VALUE) {


            setBit(23, opnum[0] >> 2, 14);


            setGenRegister('A', opchar[1]);
            setGenRegister('B', opchar[2]);
            if (strcmp(opchar[3], "") != 0) {

                setGenRegister('R', opchar[3]);
            }
        }
        break;

    case BRK:
        
        setBit(9, opnum[0], 4);
        setBit(31, opnum[1], 16);

        break;

    case DSR:

        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        setGenRegister('B', opchar[2]);
        if (strcmp(opchar[3], "") != 0) {
            setBit(21, opnum[3], 5);
        }
        break;

    case SHLA:

        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        setGenRegister('B', opchar[2]);
        setBit(21, opnum[3], 2);
        break;

    case PCA:
    case PTLB:
        
        setGenRegister('A', opchar[0]);
        if (opchar[1][0] == 'S') {
            value = getSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                setBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", (int) value);
                processError(errmsg);
                break;
            }
            setGenRegister('B', opchar[2]);
        }
        else {
            setGenRegister('B', opchar[1]);
        }

        break;

    case CMR:

        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        setGenRegister('B', opchar[2]);
        break;

    case DIAG:
        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        setGenRegister('B', opchar[2]);
        setBit(13, opnum[3], 4);
        break;

    case ITLB:

        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        setGenRegister('B', opchar[2]);
        break;

    case LDO:

        setGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            setGenRegister('B', opchar[2]);
            setOffset(27, opnum[1], 18);

        }
        else if (operandTyp[1] == OT_MEMLOC) {

            strcpy(opchar[2], "R15");
            setGenRegister('B', opchar[2]);
            setOffset(27, opnum[1], 18);

        }
        else if (operandTyp[1] == OT_VALUE) {

            setGenRegister('B', opchar[2]);
            setBit(27, opnum[1], 18);           // offset
        }
        break;

    case LSID:

        setGenRegister('R', opchar[0]);
        setGenRegister('B', opchar[1]);
        break;

    case PRB:

        setGenRegister('R', opchar[0]);
        if (opchar[1][0] == 'S') {
            value = getSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                setBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", (int)value);
                processError(errmsg);
                break;
            }
            setGenRegister('B', opchar[2]);
            if (strcmp(opchar[3], "") != 0) {

                setGenRegister('A', opchar[3]);
            }
        }
        else {
            setGenRegister('B', opchar[1]);        
            if (strcmp(opchar[2], "") != 0) {

                setGenRegister('A', opchar[2]);
            }
        }

        break;

    case LDPA:

        setGenRegister('R', opchar[0]);
        setGenRegister('A', opchar[1]);
        if (opchar[2][0] == 'S') {
            value = getSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                setBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", (int)value);
                processError(errmsg);
                break;
            }
            setGenRegister('B', opchar[3]);
        }
        else {
            setGenRegister('B', opchar[2]);
        }
        break;

    case MR:

        if (opchar[0][0] == 'R') {
            setGenRegister('R', opchar[0]);
        }
        else {
            setMRRegister(opchar[0]);

        }
        if (opchar[1][0] == 'R') {
            setGenRegister('R', opchar[1]);
        }
        else {
            setMRRegister(opchar[1]);
        }
        break;

    case MST:

        setGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_REGISTER) {

            setGenRegister('B', opchar[1]);
        }
        else {
            setBit(31, opnum[1], 4);
        }
        break;

    case RFI:

        // nothing to do
        break;

    case LD:
    case ST:

        setGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            setBit(27, opnum[1], 12);
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

                strcpy(opchar[2], "R13");
                setGenRegister('B', opchar[2]);
                setOffset(27, opnum[1], 12);
        }
        else {
            setGenRegister('A', opchar[1]);
            setBit(10, 1, 1);
        }
        if (opchar[2][0] == 'S') {
            value = getSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                setBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", (int)value);
                processError(errmsg);
                break;
            }
            setGenRegister('B', opchar[3]);
        }
        else {
            setGenRegister('B', opchar[2]);
        }
        break;

    case LDA:
    case STA:

        setGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            setBit(27, opnum[1], 12);
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            setGenRegister('B', opchar[2]);
            setOffset(27, opnum[1], 12);
        }
        else {
            setGenRegister('A', opchar[1]);
            setBit(10, 1, 1);
        }
        setGenRegister('B', opchar[2]);
        break;
        
    default: 
        snprintf(errmsg, sizeof(errmsg), "this should not happen\n");
        processError(errmsg);
        break;
    }

    createBinary();
    elfCode[0] = (binInstr >> 24) & 0xFF;
    elfCode[1] = (binInstr >> 16) & 0xFF;
    elfCode[2] = (binInstr >> 8) & 0xFF;
    elfCode[3] = binInstr & 0xFF;
    addTextSectionData();
    numOfInstructions++;
    bin_status = B_BIN; 
    
}

/// \brief Finalize and insert a binary instruction.
/// \details
/// Creates or inserts a `SRCNode` representing the current binary
/// instruction into the global source structure, depending on the
/// current binary status (`B_BIN` or `B_BINCHILD`).
void createBinary() {

    if (bin_status == B_BINCHILD) {

        // SRCbin = Create_SRCnode(SRC_BIN, buffer, lineNr);

        searchSRC(GlobalSRC, 0);


        SRCNode* node = (SRCNode*)malloc(sizeof(SRCNode));

        if (node == NULL) {
            fatalError("malloc failed");
        }
        node->type = SRC_BIN;
        node->linenr = lineNr;
        node->binStatus = bin_status;
        node->binInstr = binInstr;
        node->codeAdr = codeAdr - 4;
        node->text = strdup(infmsg);
        node->children = NULL;
        node->childCount = 0;
        SRCbin = node;

        addSRCchild(SRCcurrent, SRCbin);
    }
    else {
        bin_status = B_BIN;
        insertBinToSRC(GlobalSRC, 0);
    }

    return;
    
}

// ============================================================================
// AST Processing
// ============================================================================

/// \brief Traverse and process the abstract syntax tree (AST).
/// \details
/// Recursively visits AST nodes, extracts instruction data, manages
/// scope information, and triggers binary generation for instructions.
/// 
/// \param node  Pointer to the current AST node.
/// \param depth Current traversal depth (used for recursion).

void processAST(ASTNode* node, int depth) {

    if (!node) return;

    strcpy(currentScopeName, node->scopeName);
    currentScopeLevel = node->scopeLevel;

    SymNode* currentSym = node->symNodeAdr;
    


    switch (node->type) {

        case NODE_PROGRAM: 
        
            break;
        
        case NODE_INSTRUCTION:  

            if (codeInstrFlag == TRUE ) {
                if (nodeTypeOld == NODE_INSTRUCTION) {
                    genBinOption();
                    genBinInstruction();

                }
                else if (nodeTypeOld == NODE_CODE) {
                    // create ELF structure
                    createTextSegment();
                    strcpy(buffer, ".text.");
                    strcat(buffer, label);
                    createTextSection(buffer);
                    addTextSectionToSegment();

                    // add textsegment address to segment table
                    addSegmentEntry(numSegment, labelCodeOld, 'T', elfCodeAddrOld, numOfInstructions * 4);
                    numSegment++;
                    numOfInstructions = 0;
                    codeAdr = 0;
                    elfCodeAddrOld = elfCodeAddr;
                    strcpy(labelCodeOld, label);
                    codeExist = TRUE;
                }
            }
            codeAdr = codeAdr + 4;
            opCount = 0;
            optCount = 0;
            mode = 0;
            for (int i = 0; i < 5; i++) {
                strcpy(opchar[i], "");
                opnum[i] = 0;
            }
            nodeTypeOld = node->type;

            strcpy(option[0], "");
            strcpy(option[1], "");
            currentSymSave = currentSym;
            binInstr = node->valnum;
            lineNr = node->lineNr;
            codeInstrFlag = TRUE;
            break;


        case NODE_CODE:
            if (codeInstrFlag == TRUE) {

                if (nodeTypeOld == NODE_INSTRUCTION) {

                    genBinOption();
                    genBinInstruction();
                }
                else if (nodeTypeOld == NODE_CODE) {

                    // create ELF structure
                    createTextSegment();
                    strcpy(buffer, ".text.");
                    strcat(buffer, label);
                    createTextSection(buffer);
                    addTextSectionToSegment();



                    // add textsegment address to segment table
                    addSegmentEntry(numSegment, labelCodeOld, 'T', elfCodeAddrOld, numOfInstructions*4);
                    elfCodeAddrOld = elfCodeAddr;
                    strcpy(labelCodeOld, label);
                    numSegment++;
                    codeAdr = 0;
                    numOfInstructions = 0;
                    codeExist = TRUE;
                }

             }
            nodeTypeOld = node->type;
            codeInstrFlag = TRUE;
            break;
        
        case NODE_ADDR:
            elfCodeAddr = node->valnum;

            break;

        case NODE_ALIGN:
            elfCodeAlign = node->valnum;
            break;

        case NODE_ENTRY:
            elfEntryPoint = elfCodeAddr;
            break;

        case NODE_LABEL: 
            strcpy(label, node->value);

            break;


        case NODE_DIRECTIVE:
            break;
    
        case NODE_OPTION: 
            strcpy(option[optCount], node->value);
            optCount++;
            break;

        case NODE_OPERATION: 
            opInstrType = node->valnum;
            break;
        
        case NODE_OPERAND: 
            //printf("Operand %s %d  Opcount %d\n", node->value, node->valnum, opCount);
            strcpy(opchar[opCount], node->value);
            opnum[opCount] = node->valnum;
            operandTyp[opCount] = node->operandType;
            opCount++;
            break;
        
        case NODE_MODE:

            mode = node->valnum;
            // now we can populate the instruction
            break;

        default:
            snprintf(errmsg, sizeof(errmsg), "This should not happen!!");
            processError(errmsg);
    }


       
    // cgMode --> bit 12,13

       


    for (int i = 0; i < node->childCount; i++) {
        processAST(node->children[i], depth + 1);
    }
}





