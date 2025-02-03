#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief reads the AST and the symtab and builds the maschine code
/// 


// Funktion suche label in Symboltabelle



/// @par sets bit in instruction
///    - pos -> Position in instruction
///    - x   -> zu setzender Wert
///    - num -> Anzahl Stellen -> maske

void SetBit(int pos, int x, int num) {
    
    int limit = pow(2, num);
    if (x > (limit - 1) ||
        x < (-limit)) {

        snprintf(errmsg, sizeof(errmsg), "Value %d out of range", x);
        ProcessError(errmsg);
    }
    int mask = pow(2, num) - 1;
    int val = (x & mask);
    binInstr = binInstr | val << 31 - pos;
}


void SetOffset(int pos, int x, int num) {
    
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
        WriteBinary();   

        codeAdr = codeAdr + 4;
        binInstr = binInstrSave;
        binInstr = binInstr & 0xFFFFFFF0;
        strcpy(opchar[2], "R1");
        strcpy(infmsg, "modified Instruction offset regB -> R1, R%limit \n");

        SetGenRegister('B', opchar[2]);

        opnum[1] = 0;
    }
    num = x & 0x3FF;
    SetBit(26, num, 11);


}

/// @par sets register nummer
///     - reg     -> R, B, A register
///     - regname -> Name das Register e.g. R12
///     only regular register yet

void SetGenRegister(int reg, char* regname) {

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
        
        snprintf(errmsg, sizeof(errmsg), "General register # %d out of range", value);
        ProcessError(errmsg);
        return; 
    }
    switch (reg) {
    case 'R':
        
        SetBit(9, value,4);
        break;
    
    case 'A':
        
        SetBit(27, value,4);
        break;
    
    case 'B':
    
        SetBit(31, value,4);
        break;
    }

}

/// @par provides segment register number
///     - regname -> Name das Register e.g. S1

int GetSegRegister(char* regname) {

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

        snprintf(errmsg, sizeof(errmsg), "Segment register # %d out of range", value);
        ProcessError(errmsg);
        return 0;
    }


    return value;

}




/// @par Generate binary Instruction option part

void GenBinOption() {
    // printf("adr: %04x binInstr %08x, Optype %d, Option %s %s Opchar %s %s %s Opnum %d %d %d Mode %d\t", codeAdr, binInstr, OpType, option[0], option[1], opchar[0], opchar[1], opchar[2], opnum[0], opnum[1], opnum[2], mode);


    int count = 0;

    // process option
    switch (OpType) {

    case ADD:
    case ADC:
    case SBC:
    case SHLA:
    case SUB:

        
        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'L':
                SetBit(10, 1, 1);
                break;

            case 'O':
                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j],lineNr,column);
                ProcessError(errmsg);
            }
        }
        break;

    case AND:
    case OR:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'N':
            case 'C':
                count++;
            }
            if (count > 1) {
                snprintf(errmsg, sizeof(errmsg), "Option N and C not at the same time");
                ProcessError(errmsg);
                break;
            }
        }
        for (j = 0; j < strlen(option[0]); j++) {
            switch (option[0][j]) {

            case 'N':
                SetBit(10, 1, 1);
                break;

            case 'C':
                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case CMP:
    case CMPU:

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        }
        else if (option[0][0] == 'L' && option[0][1] == 'T') {
            SetBit(11, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            SetBit(10, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            SetBit(10, 1, 1);
            SetBit(11, 1, 1);
        }
        else {
            if (option[0][0] == '\0') {
                snprintf(errmsg, sizeof(errmsg), "Missing Option for CMP/CMPU");
                ProcessError(errmsg);
                break;
            }
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            ProcessError(errmsg);
        }
        break;

    case CBR:
    case CBRU:

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        } else if (option[0][0] == 'L' && option[0][1] == 'T') {
            SetBit(7, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            SetBit(6, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            SetBit(6, 1, 1);
            SetBit(7, 1, 1);
        }
        else {
            if (option[0][0] == '\0') {
                snprintf(errmsg, sizeof(errmsg), "Missing Option for CBR/CBRU");
                ProcessError(errmsg);
                break;
            }
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            ProcessError(errmsg);
        }
        break;

    case XOR:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'N':
                SetBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;



    case CMR:

        if (strcmp(option[0],"") == NULL) {
            break;
        }

        if (option[0][0] == 'E' && option[0][1] == 'Q') {

        }
        else if (option[0][0] == 'L' && option[0][1] == 'T') {
            SetBit(13, 1, 1);
        }
        else if (option[0][0] == 'N' && option[0][1] == 'E') {
            SetBit(12, 1, 1);
        }
        else if (option[0][0] == 'L' && option[0][1] == 'E') {
            SetBit(12, 1, 1);
            SetBit(13, 1, 1);
        }
        else if (option[0][0] == 'G' && option[0][1] == 'T') {
            SetBit(11, 1, 1);
        }
        else if (option[0][0] == 'G' && option[0][1] == 'E') {
            SetBit(11, 1, 1);
            SetBit(13, 1, 1);
        }
        else if (option[0][0] == 'H' && option[0][1] == 'I') {
            SetBit(11, 1, 1);
            SetBit(12, 1, 1);
        }
        else if (option[0][0] == 'H' && option[0][1] == 'E') {
            SetBit(11, 1, 1);
            SetBit(12, 1, 1);
            SetBit(13, 1, 1);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c%c", option[0][0], option[0][1]);
            ProcessError(errmsg);
        }
        break;


    case EXTR:

        mode = 0;
        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'S':
                SetBit(10, 1, 1);
                break;

            case 'A':
                SetBit(11, 1, 1);
                mode = 1;
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case PCA:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                SetBit(10, 1, 1);
                break;

            case 'M':
                SetBit(11, 1, 1);
                break;

            case 'F':
                SetBit(14, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case PTLB:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                SetBit(10, 1, 1);
                break;

            case 'M':
                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case ITLB:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'T':
                SetBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case DSR:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'A':
                SetBit(10, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case MR:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'D':
                SetBit(10, 1, 1);
                break;

            case 'M':
                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case MST:

        switch (option[0][0]) {

        case 'S':
            SetBit(11, 1, 1);
            break;

        case 'C':
            SetBit(10, 1, 1);
            break;

        default:
            snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
            ProcessError(errmsg);
        }
        break;


    case PRB:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'W':
                SetBit(10, 1, 1);
                break;

            case 'I':
                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case DEP:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'Z':
                SetBit(10, 1, 1);
                break;

            case 'A':
                SetBit(11, 1, 1);
                break;

            case 'I':
                SetBit(12, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;

    case LD:
    case LDA:
    case ST:

        for (j = 0; j < strlen(option[0]); j++) {

            switch (option[0][j]) {

            case 'M':

                SetBit(11, 1, 1);
                break;

            default:
                snprintf(errmsg, sizeof(errmsg), "Invalid Option %c ", option[0][j]);
                ProcessError(errmsg);
            }
        }
        break;
    }
    return;
}

/// @par Generate binary Instruction option part

void GenBinInstruction() {
    switch (OpType) {

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

            SetGenRegister('R', opchar[0]);
            SetBit(30, opnum[1], 17);
            if (opnum[1] < 0) {
                
                SetBit(31, 1,1);
            }
        }
        else if (mode == 1) {

            SetGenRegister('R', opchar[0]);
            SetGenRegister('A', opchar[1]);
            SetGenRegister('B', opchar[2]);
            SetBit(13, 1,1);
        }
        else if (mode == 2) {

            SetGenRegister('R', opchar[0]);
            SetGenRegister('A', opchar[1]);
            SetGenRegister('B', opchar[2]);
            SetBit(12, 1,1);
        }
        else if (mode == 3) {

            SetGenRegister('R', opchar[0]);
            if (operandTyp[1] == OT_MEMGLOB) {

                strcpy(opchar[2], "R13");
                SetGenRegister('B', opchar[2]);
                SetOffset(26, opnum[1], 11);

            } else if (operandTyp[1] == OT_MEMLOC) {

                strcpy(opchar[2], "R15");
                SetGenRegister('B', opchar[2]);
                SetOffset(26, opnum[1], 11);

            } else if (operandTyp[1] == OT_VALUE) {

                SetGenRegister('B', opchar[2]);
                SetBit(26, opnum[1], 11);           // offset
            }

            if (opnum[1] < 0) {

                SetBit(27, 1,1);
            }
            SetBit(12, 1,1);
            SetBit(13, 1,1);
        }
        break;

    /// @par ADDIL,LDIL
    ///    - OP regR, value (22bitU)

    case ADDIL:
    case LDIL:
        SetGenRegister('R', opchar[0]);
        SetBit(31, opnum[1], 22);
        break;

    case B:
  
        if (operandTyp[0] == OT_VALUE) {

            if ((opnum[0] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[2]);
                ProcessError(errmsg);
            }
            else
            {
                value = opnum[0] >> 2;
                SetBit(30, value, 21);
                if (value < 0) {

                    SetBit(31, 1, 1);
                }
            }
        }

        if (operandTyp[0] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            SearchSymLevel(currentSymSave, opchar[0], 0);
            if (symFound == TRUE) {

                value = symcodeAdr - codeAdr + 4;

                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[0]);
                    ProcessError(errmsg);
                }
                value = value >> 2;
                SetBit(30, value, 21);
                if (value < 0) {

                    SetBit(31, 1, 1);
                }
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                ProcessError(errmsg);
            }
        }
        if (DBG_GENBIN == TRUE) {
            printf("B line %03d addroffset= %d\n", lineNr, value);
        }

        if (strcmp(opchar[1], "") != 0) {

            SetGenRegister('R', opchar[1]);
        }


        break;

    case GATE:
        
        SetGenRegister('R', opchar[0]);

        if (operandTyp[1] == OT_VALUE) {

            if ((opnum[1] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[1]);
                ProcessError(errmsg);
            }
            else
            {
                value = opnum[1] >> 2;
                SetBit(30, value, 21);
                if (value < 0) {

                    SetBit(31, 1, 1);
                }
            }
        }

        if (operandTyp[1] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            SearchSymLevel(currentSymSave, opchar[1], 0);
            if (symFound == TRUE) {
                value = symcodeAdr - codeAdr + 4;


                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[1]);
                    ProcessError(errmsg);
                }
                value = value >> 2;
                SetBit(30, value, 21);
                if (value < 0) {

                    SetBit(31, 1, 1);
                }
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                ProcessError(errmsg);
            }
        }
        if (DBG_GENBIN == TRUE) {
            printf("GATE line %03d addroffset= %d\n", lineNr, value);
        }
        break;

    case BR:
    case BV:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('B', opchar[1]);
        break;

    case CBR:
    case CBRU:

        SetGenRegister('A', opchar[0]);
        SetGenRegister('B', opchar[1]);

        if (operandTyp[2] == OT_VALUE) {
            if ((opnum[2] % 4) != 0) {
                snprintf(errmsg, sizeof(errmsg), "Value %d not on word boundary", opnum[2]);               
                ProcessError(errmsg);
            }
            else
            {
                value = opnum[2] >> 2;
                SetBit(22, value, 15);
                if (value < 0) {

                    SetBit(23, 1, 1);
                }
            }
        }

        if (operandTyp[2] == OT_LABEL) {
            searchScopeLevel = currentScopeLevel;
            symFound = FALSE;

            SearchSymLevel(currentSymSave, opchar[2], 0);
            if (symFound == TRUE) {
                value = symcodeAdr - codeAdr + 4 ;

                if ((value % 4) != 0) {
                    snprintf(errmsg, sizeof(errmsg), "Label %s not on word boundary", opchar[2]);
                    ProcessError(errmsg);
                }
                value = value >> 2;
                SetBit(22, value, 15);
                if (value < 0) {

                    SetBit(23, 1, 1);
                }
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Label %s not found", opchar[2]);
                ProcessError(errmsg);
            }
        }
        if (DBG_GENBIN) {

            printf("CBR/CBRU line %03d addroffset= %d\n", lineNr, value);
        }
        
        break;

    case BVE:
        SetGenRegister('A', opchar[0]);
        SetGenRegister('B', opchar[1]);
        if (strcmp(opchar[2],"") != 0) {


            SetGenRegister('R', opchar[2]);
        }
        break;

    case EXTR:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('B', opchar[1]);
        if (mode == 0) {
            SetBit(27, opnum[2], 5);
            SetBit(21, opnum[3], 5);
        }
        else {
            SetBit(21, opnum[2], 5);
        }
        break;

    case DEP:
        if (mode == 0) {

            SetGenRegister('R', opchar[0]);
            SetGenRegister('B', opchar[1]);
            SetBit(27, opnum[2], 5);
            SetBit(21, opnum[3], 5);
        }
        else if (mode == 1) {

            SetGenRegister('R', opchar[0]);
            SetGenRegister('B', opchar[1]);
            SetBit(21, opnum[2], 5);
        }
        else if (mode == 2) {

            SetGenRegister('R', opchar[0]);
            SetBit(31, opnum[1], 4);
            SetBit(27, opnum[2], 5);
            SetBit(21, opnum[3], 5);
        }
        else if (mode == 3) {

            SetGenRegister('R', opchar[0]);
            SetBit(31, opnum[1], 4);
            SetBit(21, opnum[2], 5);
        }
        break;


    case LDR:
    case STC:

        SetGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            SetBit(26, opnum[1], 11);
            if (opnum[1] < 0) {

                SetBit(27, 1, 1);
            }
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            SetGenRegister('B', opchar[2]);
            SetOffset(26, opnum[1], 11);
            if (opnum[1] < 0) {

                SetBit(27, 1, 1);
            }
        }
        if (opchar[2][0] == 'S') {
            value = GetSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                SetBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", value);
                ProcessError(errmsg);
                break;
            }
            SetGenRegister('B', opchar[3]);
        }
        else {
            SetGenRegister('B', opchar[2]);
        }
        break;

    case BE:

        if (operandTyp[0] == OT_VALUE) {
            SetBit(22, opnum[0], 13);
            if (opnum[0] < 0) {

                SetBit(23, 1, 1);
            }
            value = opnum[0];
            SetGenRegister('A', opchar[1]);
            SetGenRegister('B', opchar[2]);
            if (strcmp(opchar[3], "") != 0) {

                SetGenRegister('R', opchar[3]);
            }
        }
        break;

    case BRK:
        
        SetBit(9, opnum[0], 4);
        SetBit(31, opnum[1], 16);

        break;

    case DSR:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        SetGenRegister('B', opchar[2]);
        if (strcmp(opchar[3], "") != 0) {
            SetBit(21, opnum[3], 5);
        }
        break;

    case SHLA:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        SetGenRegister('B', opchar[2]);
        SetBit(21, opnum[3], 2);
        break;

    case PCA:
    case PTLB:

        printf("regA,regB\n");
        break;

    case CMR:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        SetGenRegister('B', opchar[2]);
        break;

    case DIAG:
        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        SetGenRegister('B', opchar[2]);
        SetBit(13, opnum[3], 4);
        break;

    case ITLB:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        SetGenRegister('B', opchar[2]);
        break;

    case LDO:

        SetGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            SetGenRegister('B', opchar[2]);
            SetOffset(26, opnum[1], 17);

        }
        else if (operandTyp[1] == OT_MEMLOC) {

            strcpy(opchar[2], "R15");
            SetGenRegister('B', opchar[2]);
            SetOffset(26, opnum[1], 17);

        }
        else if (operandTyp[1] == OT_VALUE) {

            SetGenRegister('B', opchar[2]);
            SetBit(26, opnum[1], 17);           // offset
        }

        if (opnum[1] < 0) {

            SetBit(27, 1, 1);
        }
        break;

    case LSID:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('B', opchar[1]);
        break;

    case PRB:

        SetGenRegister('R', opchar[0]);
        if (opchar[1][0] == 'S') {
            value = GetSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                SetBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", value);
                ProcessError(errmsg);
                break;
            }
            SetGenRegister('B', opchar[2]);
            if (strcmp(opchar[3], "") != 0) {

                SetGenRegister('A', opchar[3]);
            }
        }
        else {
            SetGenRegister('B', opchar[1]);        
            if (strcmp(opchar[2], "") != 0) {

                SetGenRegister('A', opchar[2]);
            }
        }

        break;

    case LDPA:

        SetGenRegister('R', opchar[0]);
        SetGenRegister('A', opchar[1]);
        if (opchar[2][0] == 'S') {
            value = GetSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                SetBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n", value);
                ProcessError(errmsg);
                break;
            }
            SetGenRegister('B', opchar[3]);
        }
        else {
            SetGenRegister('B', opchar[2]);
        }
        break;

    case MR:

        printf("regR,regS or regR,regC or vice versa\n");
        break;

    case MST:

        printf("val|regB\n");
        break;

    case RFI:

        break;

    case LD:
    case ST:

        SetGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            SetBit(26, opnum[1], 11);
            if (opnum[1] < 0) {

                SetBit(27, 1, 1);
            }
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

                strcpy(opchar[2], "R13");
                SetGenRegister('B', opchar[2]);
                SetOffset(26, opnum[1], 11);
                if (opnum[1] < 0) {

                    SetBit(27, 1, 1);
                }
        }
        else {
            SetGenRegister('A', opchar[1]);
            SetBit(21, 1, 1);
        }
        if (opchar[2][0] == 'S') {
            value = GetSegRegister(opchar[2]);
            if (value > 0 && value < 4) {
                SetBit(13, value, 2);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Segmentregister S%d not allowed\n",value);
                ProcessError(errmsg);
                break;
            }
            SetGenRegister('B', opchar[3]);
        }
        else {
            SetGenRegister('B', opchar[2]);
        }
        break;

    case LDA:
    case STA:

        SetGenRegister('R', opchar[0]);
        if (operandTyp[1] == OT_VALUE) {
            SetBit(26, opnum[1], 11);
            if (opnum[1] < 0) {

                SetBit(27, 1, 1);
            }
            value = opnum[0];
        }
        else if (operandTyp[1] == OT_MEMGLOB) {

            strcpy(opchar[2], "R13");
            SetGenRegister('B', opchar[2]);
            SetOffset(26, opnum[1], 11);
            if (opnum[1] < 0) {

                SetBit(27, 1, 1);
            }
        }
        else {
            SetGenRegister('A', opchar[1]);
            SetBit(21, 1, 1);
        }
        SetGenRegister('B', opchar[2]);
        break;
        
    default: 
        snprintf(errmsg, sizeof(errmsg), "this should not happen\n");
        ProcessError(errmsg);
        break;
    }

    WriteBinary();
    bin_status = B_BIN; 
    
}

void WriteBinary() {
    _ltoa(binInstr, buffer, 2);

    // printf("%d\t%04x:%08x\t%031s\n", lineNr, codeAdr - 4, binInstr, buffer);
     printf("w disasm (0x%08x) # line %d\n",  binInstr, lineNr);

    if (bin_status == B_BINCHILD) {

        // SRCbin = Create_SRCnode(SRC_BIN, buffer, lineNr);

        Search_SRC(GlobalSRC, 0);


        SRCNode* node = (SRCNode*)malloc(sizeof(SRCNode));

        if (node == NULL) {
            FatalError("malloc failed");
        }
        node->type = SRC_BIN;
        node->linenr = lineNr;
        node->binstatus = bin_status;
        node->binInstr = binInstr;
        node->codeAdr = codeAdr - 4;
        node->text = _strdup(infmsg);
        node->children = NULL;
        node->child_count = 0;
        SRCbin = node;

        Add_SRCchild(SRCcurrent, SRCbin);
    }
    else {
        bin_status = B_BIN;
        InsertMC_SRC(GlobalSRC, 0);
    }

    return;
    
}

/// @par Abstract Syntax Tree durcharbeiten

void CodeGenAST(ASTNode* node, int depth) {

    if (!node) return;

    strcpy(currentScopeName, node->scopeName);
    currentScopeLevel = node->scopeLevel;

    SymNode* currentSym = node->symNodeAdr;
    


    switch (node->type) {

        case NODE_PROGRAM: 
        
            break;
        
        case NODE_INSTRUCTION:  
        
            if (codeAdr > 0) {
                GenBinOption();
                GenBinInstruction();
            }
            codeAdr = codeAdr + 4;
            opCount = 0;
            optCount = 0;
            mode = 0;
            for (int i = 0; i < 5; i++) {
                strcpy(opchar[i], "");
                opnum[i] = 0;
            }
            strcpy(option[0], "");
            strcpy(option[1], "");
            currentSymSave = currentSym;
            binInstr = node->valnum;
            lineNr = node->linenr;
            break;
        
        case NODE_LABEL: break;
    
        case NODE_OPTION: 
            strcpy(option[optCount], node->value);
            optCount++;
            break;

        case NODE_OPERATION: 
            OpType = node->valnum;
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
    }


       
    // cgMode --> bit 12,13

       


    for (int i = 0; i < node->child_count; i++) {
        CodeGenAST(node->children[i], depth + 1);
    }
}





