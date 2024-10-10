// --------------------------------------------------**
// 
// ASM32.cpp    
// Two pass assembler for VCPU32
// 
// Version 
// --------------------------------
// 


#include "constants.hpp"
#include "ASM32.hpp"


// --------------------------------------------------------------------------------
//      Global variables
// --------------------------------------------------------------------------------

int         lineNr = 0;                     // line number source file
int         column;                         // column of token in sourceline
char        sl[MAX_LINE_LENGTH];            // Line from source file
char        token[MAX_WORD_LENGTH];         // token value
char        token_old[MAX_WORD_LENGTH];     // save previous token
int         tokTyp;                         // type of token see enum
int         tokTyp_old;                     // save previous tokTyp
int         numToken;                       // numeric value of token
int         value;
int         mode;                           // mode for arithmetic functions
char        label[MAX_WORD_LENGTH];         // actual label
int         ind = 0;                        // index in sourceline for tokenizing
int         j;                              // inhdex to fill token

char        errmsg[MAX_ERROR_LENGTH];       // Error 
bool        is_label;
bool        is_instruction;
bool        is_directive;
bool        is_negative;
char        SourceFileName[255];            // The .s filename
FILE*       inputFile;                      // The .s input file

// --------------------------------------------------------------------------------
//      token list
// --------------------------------------------------------------------------------

struct tokenList* next_t = NULL;
struct tokenList* start_t = NULL;
struct tokenList* ptr_t;                    // regular pointer

// --------------------------------------------------------------------------------
//      SymbolTable
// --------------------------------------------------------------------------------

SymNode* scopeTab[10];
char scopeNameTab[50][10];          // table contains ScopeName per level
int currentScopeLevel;              // Level in Symboltab tree
int maxScopeLevel = 4;              // Max number of nested level
char currentScopeName[50];          // Name = label of current Scope
ScopeType currentScopeType;               // actual ScopeType
bool symFound = FALSE;
int scopeLevelSave;
SymNode* global = NULL;
SymNode* program = NULL;
SymNode* module = NULL;
SymNode* function = NULL;
SymNode* block = NULL;
SymNode* directive = NULL;

char symFunc[50];           // Directive in Symtab 
char symValue[50];             // Wert in Symtab

// --------------------------------------------------------------------------------
//      Sub Routines  
// --------------------------------------------------------------------------------


// -------------------------------------------------------------------------------- 
//  Main Routine
// --------------------------------------------------------------------------------

int main(int argc, char** argv) {


    // -------------------------------------------------------------------------------- 
    //  FOR TEST ONLY. Source File: test.s 
    // --------------------------------------------------------------------------------

    strcpy(SourceFileName, "C:\\Users\\User\\source\\repos\\ASM32\\x64\\Debug\\Test.s");

    if (SourceFileName == NULL) {
        FatalError("No source file given");
    }
    OpenSourceFile();
    printf("\n\nAssembler start %s\n", VERSION);

    // --------------------------------------------------------------------------------
    //  Lexer
    //  reads sourcefile and generates chained list of tokens
    // --------------------------------------------------------------------------------

    lineNr = 1;

    while (TRUE) {

        ind = 0;
        j = 0;
        tokTyp = NONE;
        fgets(sl, MAX_LINE_LENGTH, inputFile);
        if (feof(inputFile) != 0) {
            break;
        }


        while (tokTyp != T_EOL) {
            GetToken();
            NewTokenList();
            ptr_t->lineNumber = lineNr;
            ptr_t->column = column + 1;
            strcpy(ptr_t->token, token);
            ptr_t->tokTyp = tokTyp;
        }


        lineNr++;
    }

    NewTokenList();
    ptr_t->lineNumber = lineNr;
    ptr_t->column = 0;
    strcpy(ptr_t->token, "");
    ptr_t->tokTyp = EOF;

    PrintTokenList();

    // --------------------------------------------------------------------------------
    //  Parser
    //  reads chained list of tokens and generates Abstarct syntax tree
    // --------------------------------------------------------------------------------
    




    printf("===== Parser ======\n");

    // Globaler Scope
    currentScopeLevel = GLOBAL;
    strcpy(label, "GLOBAL");
    strcpy(symFunc, "");
    strcpy(symValue, "");
    global = create_node(GLOBAL, label, symFunc, symValue, 0);
    scopeTab[currentScopeLevel] = global;

    // ADD Programm P1 Scope
    strcpy(label, "P1");
    strcpy(symFunc, "PROGRAM");
    addScope(PROGRAM, label, symFunc, SourceFileName, 0);
    currentScopeType = PROGRAM;


    ptr_t = start_t;

    while (ptr_t != NULL) {

        is_label = FALSE;
        is_instruction = FALSE;
        is_directive = FALSE;

        strcpy(token_old, ptr_t->token);
        
        if (ptr_t->tokTyp == T_IDENTIFIER) {

            NextToken();
            if (tokTyp == T_COLON) {
                is_label = TRUE;
            }
            else {
                is_instruction = TRUE;
            }
        }
        else if (ptr_t->tokTyp == T_DOT) {
            NextToken();
            if (tokTyp == T_IDENTIFIER) {
                strcpy(token_old, ptr_t->token);
                is_directive = TRUE;
            }
        }
        if (is_label == TRUE) {

            ProcessLabel();
        }
        if (is_instruction == TRUE) {
            tokTyp = ptr_t->tokTyp;
            ProcessInstruction();
        }
        if (is_directive == TRUE) {

            ProcessDirective();
        }




        if (ptr_t->tokTyp == T_EOL) {             // here we are at the end of the statement
 //           printf("\n");
        }
        
        ptr_t = ptr_t->next;
    }

    printf("\n\nSYMBOL TABLE\n");
    printf("----------------------------\n");

    print_sym(global, 0);

    CloseSourceFile();
    exit(0);

    // -------------------------------------------------------------------------------- 
    //  END of MAIN Routine
    // --------------------------------------------------------------------------------

}



