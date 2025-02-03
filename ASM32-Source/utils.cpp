
#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief contains various helper functions


// --------------------------------------------------------------------------------
//      Sub Routines  
// --------------------------------------------------------------------------------

void OpenSourceFile() {

    printf("source %s", SourceFileName);
    inputFile = fopen(SourceFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr,
            "\n----- Input file \"%s\" could not be opened for reading -----\n\n", SourceFileName);
    }
}

void CloseSourceFile() {

    fclose(inputFile);
}

// --------------------------------------------------------------------------------
//      FatalError (msg)
//      This function prints the given error message on stderr and terminates 
// --------------------------------------------------------------------------------

void FatalError(const char* msg) {

    printf("Fatal Error> %s\n", msg);
    printf("\n---- - ASM32 Assembler terminated---- - \n");
    exit(255);
}




// --------------------------------------------------------------------------------
// StrToUpper (string)
//  This function converts all letters in a string to uppercase
// --------------------------------------------------------------------------------

void StrToUpper(char* _str) {

    for (int i = 0; i <= strlen(_str); i++) {
        _str[i] = toupper(_str[i]);
    }

}


// --------------------------------------------------------------------------------
// StrToNum (string)
//  This function converts all letters in a string to a number
// --------------------------------------------------------------------------------

int StrToNum(char* _str) {
    int i = 0;
    sscanf(_str, "%d", &i);
    return i;
}


void ProcessError(const char* msg) {

    // printf("E: %s\n", msg);

    lineERR = TRUE;
    
    strcpy(buffer, msg);
    strcat(buffer, "\n");

    Search_SRC(GlobalSRC, 0);

    bin_status = B_NOBIN;
    SRCerror = Create_SRCnode(SRC_ERROR, buffer, lineNr);

    Add_SRCchild(SRCcurrent, SRCerror);
}

void PrintDebug(const char* msg) {
    printf("D: %s\n", msg);
}

// --------------------------------------------------------------------------------
//      hexCharToInt (char)  --> int
//
//      This function is passed a character. If it is a hex digit, i.e.,
//          0, 1, 2, ... 9, a, b, ... f, A, B, ... F
//          then it returns its value (0..15).  Otherwise, it returns -1.
// --------------------------------------------------------------------------------
int HexCharToInt(char ch) {

    if (('0' <= ch) && (ch <= '9')) {
        return ch - '0';
    }
    else if (('a' <= ch) && (ch <= 'f')) {
        return ch - 'a' + 10;
    }
    else if (('A' <= ch) && (ch <= 'F')) {
        return ch - 'A' + 10;
    }
    else {
        return -1;
    }
}