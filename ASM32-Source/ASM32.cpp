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
char        opCode[MAX_WORD_LENGTH];        // Opcode

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

ASTNode* ASTprogram = NULL;
ASTNode* ASTinstruction = NULL;
ASTNode* ASToperation = NULL;
ASTNode* ASTop1 = NULL;
ASTNode* ASTop2 = NULL;
ASTNode* ASTop3 = NULL;
ASTNode* ASTop4 = NULL;
ASTNode* ASTlabel = NULL;
ASTNode* ASTmode = NULL;
ASTNode* ASTopt1 = NULL;
ASTNode* ASTopt2 = NULL;


// --------------------------------------------------------------------------------
//      Sub Routines  
// --------------------------------------------------------------------------------


// -------------------------------------------------------------------------------- 
//  Main Routine
// --------------------------------------------------------------------------------

int main(int argc, char** argv) {

    // First version no options only one source file

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    if (argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    strcpy(SourceFileName, argv[1]);
    OpenSourceFile();

    printf("\n\nAssembler start %s\n", VERSION);
    ASTprogram = Create_ASTnode(NODE_PROGRAM, SourceFileName, 0);

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
    






    printf("\n\nParser\n");
    printf("----------------------------\n");

    // Globaler Scope
    currentScopeLevel = GLOBAL;
    strcpy(label, "GLOBAL");
    strcpy(symFunc, "");
    strcpy(symValue, "");
    global = Create_SYMnode(GLOBAL, label, symFunc, symValue, 0);
    scopeTab[currentScopeLevel] = global;

    // ADD Programm P1 Scope
    strcpy(label, "P1");
    strcpy(symFunc, "PROGRAM");
    AddScope(PROGRAM, label, symFunc, SourceFileName, 0);
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

            ParseLabel();
        }
        if (is_instruction == TRUE) {
            tokTyp = ptr_t->tokTyp;
            ParseInstruction();
        }
        if (is_directive == TRUE) {

            ParseDirective();
        }




        if (ptr_t->tokTyp == T_EOL) {             // here we are at the end of the statement
 //           printf("\n");
        }
        
        ptr_t = ptr_t->next;
    }

    printf("\n\nSYMBOL TABLE\n");
    printf("----------------------------\n");
    PrintSYM(global, 0);

    printf("\n\nAbstract Syntax Tree\n");
    printf("----------------------------\n");
    PrintAST(ASTprogram, 0);

    CloseSourceFile();
    exit(0);

    // -------------------------------------------------------------------------------- 
    //  END of MAIN Routine
    // --------------------------------------------------------------------------------

}



