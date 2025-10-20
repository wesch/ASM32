#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief Provides helper and utility functions for the ASM32 assembler.
/// \details
/// This file implements common helper routines, such as:
/// - File handling (open/close source files).
/// - Error and warning reporting.
/// - String manipulation (uppercase conversion, numeric parsing).
/// - Debug output.
/// - Character classification helpers.


// ====================================================================================
//  File Handling
// ====================================================================================

/// \brief Opens the source file for reading.
/// \details
/// Attempts to open the source file specified in `SourceFileName` for reading.
/// If the file cannot be opened, an error message is printed to stderr.
void openSourceFile() {
    printf("source %s", SourceFileName);
    inputFile = fopen(SourceFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr,
            "\n----- Input file \"%s\" could not be opened for reading -----\n\n", SourceFileName);
    }
}

/// \brief Closes the currently opened source file.
void closeSourceFile() {
    fclose(inputFile);
}

void extract_path(const char* fullpath, char* path_out, size_t out_size) {
    const char* last_slash = strrchr(fullpath, '/');
    const char* last_backslash = strrchr(fullpath, '\\');
    const char* sep = last_slash;

    // On Windows, the last backslash may come after the last forward slash
    if (last_backslash && (!sep || last_backslash > sep))
        sep = last_backslash;

    if (sep) {
        size_t len = sep - fullpath;
        if (len >= out_size)
            len = out_size - 1; // truncate if too long
        memcpy(path_out, fullpath, len);
        path_out[len] = '\0';
    }
    else {
        // No slash or backslash found — no path
        path_out[0] = '\0';
    }
}

// ====================================================================================
//  Error Handling
// ====================================================================================

/// \brief Prints a fatal error and terminates the program.
/// \param msg A null-terminated error message string.
/// \details
/// Displays the given message on standard output, followed by a termination
/// notice, then exits with status code 255.
void fatalError(const char* msg) {
    printf("Fatal Error> %s\n", msg);
    printf("\n----- ASM32 Assembler terminated -----\n");
    exit(255);
}

/// \brief Reports an error encountered during processing.
/// \param msg The error message to record.
/// \details
/// - Sets the `lineERR` flag.
/// - Wraps the error message into a `SRC_ERROR` node in the source tree.
/// - Links the error node into the current SRC node hierarchy.
/// - Sets binary status to `B_NOBIN`.
void processError(const char* msg) {
    lineERR = TRUE;
    sourceERR = TRUE;
    strcpy(buffer, msg);
    strcat(buffer, "\n");

    searchSRC(GlobalSRC, 0);

    bin_status = B_NOBIN;
    SRCerror = createSRCnode(SRC_ERROR, buffer, lineNr);

    addSRCchild(SRCcurrent, SRCerror);
}

/// \brief Reports a warning encountered during processing.
/// \param msg The warning message to record.
/// \details
/// - Wraps the warning into a `SRC_WARNING` node in the source tree.
/// - Links the warning node into the current SRC node hierarchy.
/// - Sets binary status to `B_NOBIN`.
void processWarning(const char* msg) {
    strcpy(buffer, msg);
    strcat(buffer, "\n");

    searchSRC(GlobalSRC, 0);

    bin_status = B_NOBIN;
    SRCerror = createSRCnode(SRC_WARNING, buffer, lineNr);

    addSRCchild(SRCcurrent, SRCerror);
}

void processInfo(const char* msg) {
    strcpy(buffer, msg);
    strcat(buffer, "\n");

    searchSRC(GlobalSRC, 0);

    bin_status = B_NOBIN;
    SRCerror = createSRCnode(SRC_WARNING, buffer, lineNr);

    addSRCchild(SRCcurrent, SRCerror);
}

/// \brief Prints a debug message.
/// \param msg The debug message string.
void printDebug(const char* msg) {
    printf("D: %s\n", msg);
}


// ====================================================================================
//  String Utilities
// ====================================================================================

/// \brief Converts a string to uppercase in place.
/// \param _str Null-terminated string to convert.
/// \details
/// Iterates through all characters in the string and converts alphabetic
/// characters to uppercase using `toupper()`.
void strToUpper(char* _str) {
    for (int i = 0; i <= strlen(_str); i++) {
        _str[i] = toupper(_str[i]);
    }
}

/// \brief Converts a string to an integer.
/// \param _str Null-terminated string containing a number.
/// \return Integer value parsed from the string.
/// \details
/// Uses `sscanf()` for parsing. Accepts decimal notation only.
int strToNum(char* _str) {
    int i = 0;
    sscanf(_str, "%d", &i);
    return i;
}


// ====================================================================================
//  Character Classification
// ====================================================================================

/// \brief Checks whether a character is an underscore.
/// \param ch Character to check.
/// \return 1 if the character is an underscore ('_'), 0 otherwise.
/// \details
/// Works similarly to `isdigit()` but specialized for underscores.
int isunderline(char ch) {
    if (ch == '_') {
        return 1;
    }
    else {
        return 0;
    }
}
