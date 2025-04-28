// --------------------------------------------------**
// 
// ASM32.cpp    
// Two pass assembler for VCPU32
// 
// Version 
// --------------------------------
// 

/// @file
/// \brief Central file containing the main section

#include "constants.hpp"
#include "ASM32.hpp"


// --------------------------------------------------------------------------------
//      Global variables
// --------------------------------------------------------------------------------

int         lineNr = 0;                     ///< line number source file
int         column;                         ///< column of token in sourceline
char        sl[MAX_LINE_LENGTH];            ///< Line from source file
char        token[MAX_WORD_LENGTH];         ///< token value
char        tokenSave[MAX_WORD_LENGTH];     ///< save previous token
int         tokTyp;                         ///< type of token see enum
int         tokTyp_old;                     ///< save previous tokTyp
int         numToken;                       ///< numeric value of token
int         value;
int         mode;                           ///< mode for arithmetic functions
bool        lineERR;                        ///< Flag actual line is in error
char        label[MAX_WORD_LENGTH];         ///< actual label
int         ind = 0;                        ///< index in sourceline for tokenizing
int         j;                              ///< inhdex to fill token
char        opCode[MAX_WORD_LENGTH];        ///< Opcode
int         OpType;                         ///< contains opCodetab.instr_type
int         operandType;                    ///< Type of Operand for AST
int         operandTyp[10];                 ///< Type of Operand for codegen
int         DirType;                        ///< contains type of directive 
char        dirCode[MAX_WORD_LENGTH];
int         VarType;                        ///< Variable Type (0=no Variable, 1=global,2=local)
char        Variable[MAX_WORD_LENGTH];      ///< contains global/local variable name
char        buffer[255];                    ///< general buffer 
int         symcodeAdr;                     ///< contains codeAdr from symtab
int         bin_status;                     ///< status for bininstr in SRCnode

char        errmsg[MAX_ERROR_LENGTH];       ///< Error 
char        infmsg[MAX_ERROR_LENGTH];
bool        is_label;
bool        is_instruction;
bool        is_directive;
bool        is_negative;

int         binInstr;                       ///< binaer instruction
int         binInstrSave;                    ///< binaer instruction save für große offset
int         codeAdr;                        ///< adress counter code
int         dataAdr;                        ///< adress counter data

char        func_entry[MAX_WORD_LENGTH];    ///< name of current function


char opchar[5][10];                         ///< collect data from AST in function codegen
int  opnum[5];                              ///< collect data from AST in function codegen
char option[2][10];                         ///< collect data from AST in function codegen
int  opCount = 0;                           ///< collect data from AST in function codegen
int  optCount = 0;                          ///< collect data from AST in function codegen

char        SourceFileName[255];            ///< The .s filename
FILE*       inputFile;                      ///< The .s input file

// DEBUG switches

bool DBG_TOKEN = FALSE;
bool DBG_PARSER = FALSE;
bool DBG_GENBIN = FALSE;
bool DBG_SYMTAB = TRUE;
bool DBG_AST = TRUE;
bool DBG_SOURCE = TRUE;
bool DBG_DIS = TRUE;

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
char scopeNameTab[50][10];          ///< table contains ScopeName per level
int currentScopeLevel;              ///< Level Symboltab tree for build 
int searchScopeLevel;               ///< Level symtab for search
int maxScopeLevel = 4;              ///< Max number of nested level
char currentScopeName[50];          ///< Name = label of current Scope
char currentScopeNameSave[50];
SYM_ScopeType currentScopeType;     ///< actual ScopeType
  
bool symFound = FALSE;
SymNode* GlobalSYM = NULL;
SymNode* program = NULL;
SymNode* module = NULL;
SymNode* function = NULL;
SymNode* block = NULL;
SymNode* directive = NULL;
SymNode* currentSym = NULL;
SymNode* currentSymSave = NULL;

char symFunc[50];                   ///< Directive in Symtab 
char symValue[50];                  ///< Wert in Symtab

// --------------------------------------------------------------------------------
//      Abstract Syntax Tree (AST)
// --------------------------------------------------------------------------------

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
//      Sourcecode and Error/Info
// --------------------------------------------------------------------------------

SRC_NodeType currentSRC_type;
SRCNode* SRCLINE = NULL;
SRCNode* SRCTEXT = NULL;
SRCNode* GlobalSRC = NULL;
SRCNode* SRCprogram = NULL;
SRCNode* SRCsource = NULL;
SRCNode* SRCbin = NULL;
SRCNode* SRCerror = NULL;
SRCNode* SRCcurrent = NULL;

// --------------------------------------------------------------------------------
//      Sub Routines  
// --------------------------------------------------------------------------------

SRCNode* Create_SRCnode(SRC_NodeType type, const char* text, int lineNr) {
    SRCNode* node = (SRCNode*)malloc(sizeof(SRCNode));

    if (node == NULL) {
        FatalError("malloc failed");
    }
    node->type = type;
    node->linenr = lineNr;
    node->binstatus = bin_status;
    node->text = strdup(text);
    node->children = NULL;
    node->child_count = 0;
    return node;
}

// Function to add a child node
void Add_SRCchild(SRCNode* parent, SRCNode* child) {
    parent->children = (SRCNode**)realloc(parent->children, sizeof(SRCNode*) * (parent->child_count + 1));
    if (parent->children == NULL) {
        FatalError("realloc failed");
    }
    parent->children[parent->child_count++] = child;
}

void Search_SRC(SRCNode* node, int depth) {
    
    if (!node) return;
    
    if (node->type == SRC_SOURCE &&
        node->linenr == lineNr) {
        SRCcurrent =  node;
    }

    for (int i = 0; i < node->child_count; i++) {
        Search_SRC(node->children[i], depth + 1);
    }

}

void InsertMC_SRC(SRCNode* node, int depth) {

    if (!node) return;

    if (node->type == SRC_SOURCE &&
        node->linenr == lineNr) {

        node->codeAdr = codeAdr - 4;
        node->binInstr = binInstr;
        node->binstatus = bin_status;
    }

    for (int i = 0; i < node->child_count; i++) {
        InsertMC_SRC(node->children[i], depth + 1);
    }

}

/// @par Function to print the SRC
/// 
void PrintSRC(SRCNode* node, int depth) {

    if (DBG_SOURCE == FALSE) {

        printf("======== SUPPRESSED =========\n");
        return;
    }

    if (!node) return;

//     printf("type=%d %04x %08x %4d\t%s", node->type, node->codeAdr, node->binInstr, node->linenr, node->text);

    if (node->linenr != 0) {
        if (node->type == SRC_ERROR) {
            printf("                    E: \t%s", node->text);
        }
        else if (node->type == SRC_SOURCE && 
                 node->binstatus == B_BIN) {
            printf(" %04x %08x %4d\t%s", node->codeAdr, node->binInstr, node->linenr, node->text);
        }
        else if (node->type == SRC_BIN &&
            node->binstatus == B_BINCHILD) {
            printf(" %04x %08x %4d\t%s", node->codeAdr, node->binInstr, node->linenr, node->text);
        }
        else if (node->type == SRC_SOURCE &&
            node->binstatus == B_NOBIN) {
            printf("               %4d\t%s",  node->linenr, node->text);
        }
        else if (node->type == SRC_INFO) {
            printf("                    I: \t%s", node->text);
        }
    }
    else {
        printf("\n\n");
        printf("  Program: %s", node->text);
        printf("--------------------------------------------------------------------------------------\n");
    }


    for (int i = 0; i < node->child_count; i++) {
        PrintSRC(node->children[i], depth + 1);
    }
}

// @par Function to print the SRC for DISASSEMBLER
/// 
void PrintSRC_DIS(SRCNode* node, int depth) {

    if (DBG_DIS == FALSE) {

        printf("======== SUPPRESSED =========\n");
        return;
    }

    if (!node) return;

    //     printf("type=%d %04x %08x %4d\t%s", node->type, node->codeAdr, node->binInstr, node->linenr, node->text);

    if (node->linenr != 0) {
        if (node->type == SRC_ERROR) {
//            printf("                    E: \t%s", node->text);
        }
        else if (node->type == SRC_SOURCE &&
            node->binstatus == B_BIN) {
            printf("w disasm (0x%08x) # line %d  %s", node->binInstr, node->linenr,node->text);
            
        }
        else if (node->type == SRC_BIN &&
            node->binstatus == B_BINCHILD) {
            printf("w disasm (0x%08x) # line %d  %s", node->binInstr, node->linenr, node->text);
        }
        else if (node->type == SRC_SOURCE &&
            node->binstatus == B_NOBIN) {
//            printf("               %4d\t%s", node->linenr, node->text);
        }
        else if (node->type == SRC_INFO) {
//            printf("                    I: \t%s", node->text);
        }
    }
    else {
        printf("\n\n");
        printf("  Program: %s", node->text);
        printf("--------------------------------------------------------------------------------------\n");
    }


    for (int i = 0; i < node->child_count; i++) {
        PrintSRC_DIS(node->children[i], depth + 1);
    }
}

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

    printf("\n\nAssembler start %s\n\n", VERSION);

    // printf("Line\tadr:code\tInput disasm\n");
    printf("--------------------------------------------------------------------------------------\n");

    ASTprogram = Create_ASTnode(NODE_PROGRAM, SourceFileName, 0);

    // --------------------------------------------------------------------------------
    //  Lexer
    //  reads sourcefile and generates chained list of tokens
    // --------------------------------------------------------------------------------

    lineNr = 1;

    strcpy(buffer, SourceFileName);
    strcat(buffer, "\n");
    GlobalSRC = Create_SRCnode(SRC_PROGRAM, buffer, 0);

    while (TRUE) {

        ind = 0;
        j = 0;
        tokTyp = NONE;
        fgets(sl, MAX_LINE_LENGTH, inputFile);

        // hier wird die source zeile in eine verkettete Liste gestellt.


        if (feof(inputFile) != 0) {
            break;
        }

        // SRC Text --> SRCNode structure

        strcpy(buffer, sl);

        SRCsource = Create_SRCnode(SRC_SOURCE, buffer, lineNr);
        Add_SRCchild(GlobalSRC, SRCsource);

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
    
    if (DBG_PARSER) {
        printf("\n\nParser\n");
        printf("----------------------------\n");
    }
    // Globaler Scope
    lineNr = 1;
    currentScopeLevel = SCOPE_GLOBAL;
    strcpy(label, "GLOBAL");
    strcpy(symFunc, "");
    strcpy(symValue, "");
    GlobalSYM = Create_SYMnode(SCOPE_GLOBAL, label, symFunc, symValue, 0);
    scopeTab[currentScopeLevel] = GlobalSYM;

    // ADD Programm P1 Scope
    strcpy(label, "P1");
    strcpy(symFunc, "PROGRAM");
    AddScope(SCOPE_PROGRAM, label, symFunc, SourceFileName, 0);

    ///< default register namen
    strcpy(label, "DP");
    strcpy(dirCode, "REG");
    strcpy(token, "R13");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RL");
    strcpy(dirCode, "REG");
    strcpy(token, "R14");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "SP");
    strcpy(dirCode, "REG");
    strcpy(token, "R15");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG0");
    strcpy(dirCode, "REG");
    strcpy(token, "R11");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG1");
    strcpy(dirCode, "REG");
    strcpy(token, "R10");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG2");
    strcpy(dirCode, "REG");
    strcpy(token, "R9");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG3");
    strcpy(dirCode, "REG");
    strcpy(token, "R8");
    AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

    currentScopeType = SCOPE_PROGRAM;

    ptr_t = start_t;
    codeAdr = 0;
    dataAdr = 0;

    while (ptr_t != NULL) {

        is_label = FALSE;
        is_instruction = FALSE;
        is_directive = FALSE;
        strcpy(tokenSave, ptr_t->token);

        if (ptr_t->tokTyp == T_IDENTIFIER) {

            GetNextToken();
            if (tokTyp == T_COLON) {

                is_label = TRUE;
            }
            else {

                is_instruction = TRUE;
            }
        }
        else if (ptr_t->tokTyp == T_DOT) {
            GetNextToken();
            if (tokTyp == T_IDENTIFIER) {

                strcpy(tokenSave, ptr_t->token);
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

        if (lineERR == TRUE)
        {
            while (ptr_t->tokTyp != T_EOL)
            {

                ptr_t = ptr_t->next;
                if (ptr_t == NULL)  break;
            }
        }


        if (ptr_t != NULL) {
            ptr_t = ptr_t->next;
        }

    }
    // insert dummy instruction at the end to ensure complete processing

    operandType = 0;
    ASTinstruction = Create_ASTnode(NODE_INSTRUCTION, "", binInstr);
    Add_ASTchild(ASTprogram, ASTinstruction);

    // --------------------------------------------------------------------------------
    //  Codegen
    //  reads AST and genertes binarty code
    // --------------------------------------------------------------------------------
 
    binInstr = 0;
    codeAdr = 0;

    CodeGenAST(ASTprogram, 0);



    printf("\n\n+------------------------------------------------------------------------------------+\n");
    printf("|                           SYMBOL TABLE                                             |\n");
    printf("+------------------------------------------------------------------------------------+ \n");
    printf("Node\tScope\tCAdr\tDAdr\tlabel\tFunc\tvalue\tVarType\tlinenr\tScopeName\n");
    printf("+------------------------------------------------------------------------------------+ \n");

    PrintSYM(GlobalSYM, 0);

    printf("\n\n+--------------------------------------------------------------------------+\n");
    printf("|                               SYNTAX TREE                                |\n");
    printf("+--------------------------------------------------------------------------+\n");
    printf("Node  \tlineNr\tCAdr OpT ValueC\tValNum\tScpL\tScpN\n");
    printf("+--------------------------------------------------------------------------+\n");

    PrintAST(ASTprogram, 0);


    printf("\n\n+------------------------------------------------------------------------------------+\n");
    printf("|                           SOURCE LISTING                                           |\n");
    printf("+------------------------------------------------------------------------------------+ \n");
 
    PrintSRC(GlobalSRC, 0);

    PrintSRC_DIS(GlobalSRC, 0);

    CloseSourceFile();
    exit(0);

    // -------------------------------------------------------------------------------- 
    //  END of MAIN Routine
    // --------------------------------------------------------------------------------

}



