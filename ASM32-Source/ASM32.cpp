// --------------------------------------------------**
//
// ASM32.cpp
// Two-pass assembler for VCPU32
//
// --------------------------------
//

/// @file
/// \brief Main translation unit for the ASM32 two-pass assembler.
/// \details
/// This file wires together the lexer, parser, AST, code generator,
/// source listing, and ELF writer. It owns global state used across
/// passes and drives the full compile → assemble flow.

#include "constants.hpp"
#include "ASM32.hpp"


// --------------------------------------------------------------------------------
//      Global variables
// --------------------------------------------------------------------------------

int         lineNr = 0;                     ///< Current source file line number (1-based).
int         column;                         ///< Column of the current token within the source line (0-based).
char        sl[MAX_LINE_LENGTH];            ///< Raw input buffer holding the current source line.
int         prgType;                        ///< Program type: 0 = undefined, 1 = standalone, 2 = module.
char        token[MAX_WORD_LENGTH];         ///< Current token text.
char        tokenSave[MAX_WORD_LENGTH];     ///< Previously seen token text (look-behind).
int         tokTyp;                         ///< Current token type (see TokenType).
int         tokTypSave;                     ///< Previous token type (look-behind).
int         numToken;                       ///< Integer value parsed from a numeric token.
int64_t     value;                          ///< Evaluated numeric value from an expression.
int         align_val;                      ///< Alignment value for directives that require alignment.
int         mode;                           ///< Mode for arithmetic or addressing operations.
bool        lineERR;                        ///< Flag: true if the current line is in error.
char        label[MAX_WORD_LENGTH];         ///< Most recent label text.
int         ind = 0;                        ///< Scanner index into the source line during tokenization.
char        opCode[MAX_WORD_LENGTH];        ///< Opcode mnemonic.
int         opInstrType;                    ///< Opcode type (maps to opCodeTab[].instrType).
int         operandType;                    ///< Operand classification used for AST construction.
int         operandTyp[10];                 ///< Operand types used by code generation.
int         directiveType;                  ///< Current directive kind (see Directives).
char        dirCode[MAX_WORD_LENGTH];       ///< Directive mnemonic (as text).
int         varType;                        ///< Variable classification: 0 = none, 1 = global, 2 = local.
char        varName[MAX_WORD_LENGTH];       ///< Name of a global/local variable.
char        buffer[255];                    ///< General-purpose text buffer.
int         symcodeAdr;                     ///< Code address associated with a symbol table entry.
int         bin_status;                     ///< Binary status for instructions stored in SRC nodes.

char        errmsg[MAX_ERROR_LENGTH];       ///< Last error message text.
char        infmsg[MAX_ERROR_LENGTH];       ///< Informational message (e.g., emitted by codegen).
bool        is_label;                       ///< Parser helper: current statement starts with a label.
bool        is_instruction;                 ///< Parser helper: current statement is an instruction.
bool        is_directive;                   ///< Parser helper: current statement is a directive.
bool        is_negative;                    ///< Parser helper: next numeric literal is negated.

int         binInstr;                       ///< Current 32-bit binary instruction being emitted.
int         binInstrSave;                   ///< Saved binary instruction (e.g., for large offset fixups).
int         codeAdr;                        ///< Code address (text section address counter).
int         dataAdr;                        ///< Data address (data section address counter).

uint32_t    elfCodeAddr;                    ///< ELF: base virtual address of the text section.
uint32_t    elfDataAddr;                    ///< ELF: base virtual address of the data section.
uint32_t    elfEntryPoint;                  ///< ELF: program entry point address.
uint32_t    elfCodeAlign;                   ///< ELF: alignment for the text section.
uint32_t    elfDataAlign;                   ///< ELF: alignment for the data section.
bool        elfEntryPointStatus = FALSE;    ///< ELF: true once the entry point has been set.

char        elfData[MAX_WORD_LENGTH];       ///< Small buffer for data bytes pushed into the data section.
char        elfCode[MAX_WORD_LENGTH];       ///< Small buffer for instruction bytes pushed into the text section.
int         elfDataLength;                  ///< Offset (write position) in the data memory area.
bool        elfDataSectionStatus;           ///< True if data have already been placed in the data section (set vs. append).
bool        elfCodeSectionStatus;           ///< True if text has already been placed in the text section (set vs. append).
char*       elfBuffer;                      ///< Pointer to the allocated data buffer used for section building.
int         elfBufferSize = 4096;           ///< Size of the data buffer allocated via malloc.

char        func_entry[MAX_WORD_LENGTH];    ///< Name of the function currently being processed.
bool        main_func_detected;             ///< True once a 'main' function (or equivalent) is detected.

char        opchar[5][10];                  ///< Codegen staging: textual operands collected from the AST.
int         opnum[5];                       ///< Codegen staging: numeric operands collected from the AST.
char        option[2][10];                  ///< Codegen staging: instruction options/modifiers collected from the AST.
int         opCount = 0;                    ///< Codegen staging: number of collected operands.
int         optCount = 0;                   ///< Codegen staging: number of collected options.

char        SourceFileName[255];            ///< Input source filename (.s / .asm).
FILE* inputFile;                            ///< Open handle for the input source file.

// --------------------------------------------------------------------------------
//      Debug switches
// --------------------------------------------------------------------------------

bool DBG_TOKEN = FALSE;  ///< Dump tokens produced by the lexer.
bool DBG_PARSER = FALSE; ///< Trace parser decisions.
bool DBG_GENBIN = FALSE; ///< Trace binary generation.
bool DBG_SYMTAB = TRUE;  ///< Dump symbol table after codegen.
bool DBG_AST = FALSE;    ///< Dump abstract syntax tree after parsing.
bool DBG_SOURCE = TRUE;  ///< Print source listing with addresses and binary.

// --------------------------------------------------------------------------------
/** \name Token list
 *  \brief Linked list built by the lexer and consumed by the parser.
 */
 // --------------------------------------------------------------------------------

struct tokenList* next_t = NULL;            ///< Tail insertion helper.
struct tokenList* start_t = NULL;           ///< Head of the token list.
struct tokenList* ptr_t;                    ///< Iteration pointer over the token list.

// --------------------------------------------------------------------------------
/** \name Symbol table
 *  \brief Hierarchical scope management for symbols, labels, variables, and directives.
 */
 // --------------------------------------------------------------------------------

SymNode* scopeTab[10];                      ///< Scope stack: one pointer per nested level.
char        scopeNameTab[50][10];           ///< Human-readable scope names by level.
int         currentScopeLevel;              ///< Current depth of the scope stack.
int         searchScopeLevel;               ///< Search depth used by symbol lookup.
int         maxScopeLevel = 4;              ///< Maximum supported nested scope depth.
char        currentScopeName[50];           ///< Label/name of the current scope.
char        currentScopeNameSave[50];       ///< Saved scope name (temporary).
SYM_ScopeType currentScopeType;             ///< Current scope type (see ::SYM_ScopeType).

bool        symFound = FALSE;               ///< Result flag for symbol searches.
SymNode* GlobalSYM = NULL;                  ///< Root/global scope node.
SymNode* program = NULL;                    ///< Program-level scope node.
SymNode* module = NULL;                     ///< Module-level scope node.
SymNode* function = NULL;                   ///< Function-level scope node.
SymNode* block = NULL;                      ///< Block/local scope node.
SymNode* directive = NULL;                  ///< Directive scope node.
SymNode* currentSym = NULL;                 ///< Current symbol node (context-dependent).
SymNode* currentSymSave = NULL;             ///< Saved symbol node (temporary).

char        symFunc[50];                    ///< Symbol "function"/kind stored in the symbol table (e.g., PROGRAM, REG).
char        symValue[50];                   ///< Symbol value stored in the symbol table (textual form).

// --------------------------------------------------------------------------------
/** \name Abstract Syntax Tree (AST)
 *  \brief Pointers to commonly referenced AST nodes during construction and traversal.
 */
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
/** \name Source and messages
 *  \brief Structures holding the hierarchical source listing and diagnostics.
 */
 // --------------------------------------------------------------------------------

SRC_NodeType currentSRC_type;               ///< Helper for building/printing the source tree.
SRCNode* SRCLINE = NULL;                    ///< (unused placeholder) line node.
SRCNode* SRCTEXT = NULL;                    ///< (unused placeholder) text node.
SRCNode* GlobalSRC = NULL;                  ///< Root node for the program’s source listing.
SRCNode* SRCprogram = NULL;                 ///< Program node in the source listing.
SRCNode* SRCsource = NULL;                  ///< One node per input source line.
SRCNode* SRCbin = NULL;                     ///< Node for additional binary rows (e.g., emitted by pseudo-ops).
SRCNode* SRCerror = NULL;                   ///< Node representing an error message.
SRCNode* SRCcurrent = NULL;                 ///< Scratch pointer used when updating nodes.

// --------------------------------------------------------------------------------
//      Subroutines
// --------------------------------------------------------------------------------



// --------------------------------------------------------------------------------
//      Set default EQU and REG
// --------------------------------------------------------------------------------

/// \brief Seed default register aliases (REG directives) into the directive scope.
/// \details
/// Creates common aliases such as DP, RL, SP, ARG0..3 and RET0..3 and inserts
/// them into the directive scope so that code can refer to symbolic register
/// names instead of raw R* numbers.
void setDefaultDirectives() {

    ///< Default register names
    strcpy(label, "DP");
    strcpy(dirCode, "REG");
    strcpy(token, "R13");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RL");
    strcpy(dirCode, "REG");
    strcpy(token, "R14");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "SP");
    strcpy(dirCode, "REG");
    strcpy(token, "R15");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG0");
    strcpy(dirCode, "REG");
    strcpy(token, "R11");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG1");
    strcpy(dirCode, "REG");
    strcpy(token, "R10");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG2");
    strcpy(dirCode, "REG");
    strcpy(token, "R9");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "ARG3");
    strcpy(dirCode, "REG");
    strcpy(token, "R8");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RET0");
    strcpy(dirCode, "REG");
    strcpy(token, "R11");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RET1");
    strcpy(dirCode, "REG");
    strcpy(token, "R10");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RET2");
    strcpy(dirCode, "REG");
    strcpy(token, "R9");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

    strcpy(label, "RET3");
    strcpy(dirCode, "REG");
    strcpy(token, "R8");
    addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);
}

/// \brief Create a new source tree node.
/// \param type Classification of the node (see ::SRC_NodeType).
/// \param text Display text (typically the raw source line or a message).
/// \param lineNr Line number associated with this node (0 for the root/program).
/// \return Pointer to the allocated node (owned by the caller).
SRCNode* createSRCnode(SRC_NodeType type, const char* text, int lineNr) {
    SRCNode* node = (SRCNode*)malloc(sizeof(SRCNode));

    if (node == NULL) {
        fatalError("malloc failed");
    }
    node->type = type;
    node->linenr = lineNr;
    node->binStatus = bin_status;
    node->text = strdup(text);
    node->children = NULL;
    node->childCount = 0;
    return node;
}

/// \brief Append a child to a parent node in the source tree.
/// \param parent Parent node to receive the child.
/// \param child Child node to append (ownership transferred).
void addSRCchild(SRCNode* parent, SRCNode* child) {
    parent->children = (SRCNode**)realloc(parent->children, sizeof(SRCNode*) * (parent->childCount + 1));
    if (parent->children == NULL) {
        fatalError("realloc failed");
    }
    parent->children[parent->childCount++] = child;
}

/// \brief Locate the current source node (matching ::lineNr) within the source tree.
/// \details
/// On match, sets the global ::SRCcurrent to the node found.
/// \param node Subtree root to search.
/// \param depth Current recursion depth (informational).
void searchSRC(SRCNode* node, int depth) {

    if (!node) return;

    if (node->type == SRC_SOURCE &&
        node->linenr == lineNr) {
        SRCcurrent = node;
    }

    for (int i = 0; i < node->childCount; i++) {
        searchSRC(node->children[i], depth + 1);
    }

}

/// \brief Insert the current binary instruction into the matching source node.
/// \details
/// For the current ::lineNr, stores the code address, binary instruction,
/// and binary status on the corresponding SRC node and eligible BIN child nodes.
/// \param node Subtree root to update.
/// \param depth Current recursion depth (informational).
void insertBinToSRC(SRCNode* node, int depth) {

    if (!node) return;

    if (node->type == SRC_SOURCE &&
        node->linenr == lineNr) {

        node->codeAdr = codeAdr - 4;
        node->binInstr = binInstr;
        node->binStatus = bin_status;
    }

    for (int i = 0; i < node->childCount; i++) {
        insertBinToSRC(node->children[i], depth + 1);
    }

}

/// \brief Print the hierarchical source listing with addresses, binaries, and diagnostics.
/// \param node Subtree root to print.
/// \param depth Current recursion depth (informational).
void printSourceListing(SRCNode* node, int depth) {

    if (!node) return;

    if (node->linenr != 0) {
        if (node->type == SRC_ERROR) {
            printf("                    E: \t%s", node->text);
        }
        else if (node->type == SRC_WARNING) {
            printf("                    W: \t%s", node->text);
        }
        else if (node->type == SRC_SOURCE &&
            node->binStatus == B_BIN) {
            printf(" %04x %08x %4d %s", node->codeAdr, node->binInstr, node->linenr, node->text);
        }
        else if (node->type == SRC_BIN &&
            node->binStatus == B_BINCHILD) {
            printf(" %04x %08x %4d %s", node->codeAdr, node->binInstr, node->linenr, node->text);
        }
        else if (node->type == SRC_SOURCE &&
            node->binStatus == B_NOBIN) {
            printf("               %4d %s", node->linenr, node->text);
        }
        else if (node->type == SRC_INFO) {
            printf("                    I:  %s", node->text);
        }
    }
    else {

        printf("                             Program: %s", node->text);
        printf("--------------------------------------------------------------------------------------\n");
    }

    for (int i = 0; i < node->childCount; i++) {
        printSourceListing(node->children[i], depth + 1);
    }
}

/// \brief Copy binary instructions from the source tree into ELF sections.
/// \param node Subtree root to read from.
/// \param depth Current recursion depth (informational).
void copyToELF(SRCNode* node, int depth) {

    if (!node) return;

    if (node->linenr != 0) {

        if (node->type == SRC_SOURCE &&
            node->binStatus == B_BIN) {
            elfCode[0] = (node->binInstr >> 24) & 0xFF;
            elfCode[1] = (node->binInstr >> 16) & 0xFF;
            elfCode[2] = (node->binInstr >> 8) & 0xFF;
            elfCode[3] = node->binInstr & 0xFF;
            addTextSectionData();
        }
        else if (node->type == SRC_BIN &&
            node->binStatus == B_BINCHILD) {
            elfCode[0] = (node->binInstr >> 24) & 0xFF;
            elfCode[1] = (node->binInstr >> 16) & 0xFF;
            elfCode[2] = (node->binInstr >> 8) & 0xFF;
            elfCode[3] = node->binInstr & 0xFF;
            addTextSectionData();
        }
    }
    for (int i = 0; i < node->childCount; i++) {
        copyToELF(node->children[i], depth + 1);
    }
}

// -------------------------------------------------------------------------------- 
//  Main Routine
// --------------------------------------------------------------------------------

/// \brief Program entry point.
/// \details
/// Expected usage: `asm32 <filename>`.
/// The assembler performs:
/// 1) Lexing (build token list),
/// 2) Parsing (build AST),
/// 3) Code generation (emit binary, build SRC tree),
/// 4) ELF construction and file emission,
/// 5) Optional diagnostics: tokens, AST, symbol table, source listing.
int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    if (argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    strcpy(SourceFileName, argv[1]);
    openSourceFile();

    printf("\n\nAssembler start %s\n\n", VERSION);

    // printf("Line\tadr:code\tInput disasm\n");
    printf("--------------------------------------------------------------------------------------\n");

    ASTprogram = createASTnode(NODE_PROGRAM, SourceFileName, 0);

    // --------------------------------------------------------------------------------
    //  Lexer
    //  Reads the source file and generates a linked list of tokens.
    // --------------------------------------------------------------------------------

    lineNr = 1;
    main_func_detected = FALSE;
    prgType = P_UNDEFINED;        // Program type not yet defined.

    strcpy(buffer, SourceFileName);
    strcat(buffer, "\n");
    GlobalSRC = createSRCnode(SRC_PROGRAM, buffer, 0);

    while (TRUE) {

        ind = 0;
        int j = 0;
        tokTyp = NONE;
        fgets(sl, MAX_LINE_LENGTH, inputFile);

        // Push the source line into the linked list structure.

        if (feof(inputFile) != 0) {
            break;
        }

        // Create a SRC node for the raw source line.
        strcpy(buffer, sl);
        SRCsource = createSRCnode(SRC_SOURCE, buffer, lineNr);
        addSRCchild(GlobalSRC, SRCsource);

        // Tokenize the current line.
        while (tokTyp != T_EOL) {
            createToken();
            createTokenEntry();
            ptr_t->lineNumber = lineNr;
            ptr_t->column = column + 1;
            strcpy(ptr_t->token, token);
            ptr_t->tokTyp = tokTyp;
        }

        lineNr++;
    }

    // Append explicit end-of-input token.
    createTokenEntry();
    ptr_t->lineNumber = lineNr;
    ptr_t->column = 0;
    strcpy(ptr_t->token, "");
    ptr_t->tokTyp = EOF;

    printTokenList();

    // --------------------------------------------------------------------------------
    //  Parser
    //  Consumes the token list and generates the abstract syntax tree (AST).
    // --------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------- 
    //  Prepare ELF output
    // --------------------------------------------------------------------------------
    createELF();
    createTextSegment();
    createTextSection();
    addTextSectionData();

    createDataSegment();
    createDataSection();
    elfDataSectionStatus = FALSE;
    elfCodeSectionStatus = FALSE;

    if (DBG_PARSER) {
        printf("\n\nParser\n");
        printf("----------------------------\n");
    }

    // Global scope
    lineNr = 1;
    currentScopeLevel = SCOPE_PROGRAM;
    strcpy(label, "GLOBAL");
    strcpy(symFunc, "");
    strcpy(symValue, "");
    GlobalSYM = createSYMnode(SCOPE_PROGRAM, label, symFunc, SourceFileName, 0);
    scopeTab[currentScopeLevel] = GlobalSYM;

    // Optionally add program scope here:
    // strcpy(label, "P1");
    // strcpy(symFunc, "PROGRAM");
    // AddScope(SCOPE_PROGRAM, label, symFunc, SourceFileName, 0);

    setDefaultDirectives();

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
            fetchToken();
            if (tokTyp == T_COLON) {
                is_label = TRUE;
            }
            else {
                is_instruction = TRUE;
            }
        }
        else if (ptr_t->tokTyp == T_DOT) {
            fetchToken();
            if (tokTyp == T_IDENTIFIER) {
                strcpy(tokenSave, ptr_t->token);
                is_directive = TRUE;
            }
        }

        if (is_label == TRUE) {
            parseLabel();
        }
        if (is_instruction == TRUE) {
            tokTyp = ptr_t->tokTyp;
            parseInstruction();
        }
        if (is_directive == TRUE) {
            ParseDirective();
        }

        if (lineERR == TRUE) {
            // Skip to end of line after an error to resynchronize.
            while (ptr_t->tokTyp != T_EOL) {
                ptr_t = ptr_t->next;
                if (ptr_t == NULL)  break;
            }
        }

        if (ptr_t != NULL) {
            ptr_t = ptr_t->next;
        }
    }

    // Insert a dummy instruction at the end to ensure complete processing.
    operandType = 0;
    ASTinstruction = createASTnode(NODE_INSTRUCTION, "", binInstr);
    addASTchild(ASTprogram, ASTinstruction);

    // --------------------------------------------------------------------------------
    //  Codegen
    //  Traverses the AST and emits binary code.
    // --------------------------------------------------------------------------------

    binInstr = 0;
    codeAdr = 0;

    readAST(ASTprogram, 0);

    if (DBG_SYMTAB == TRUE) {
        printf("\n\n+------------------------------------------------------------------------------------+\n");
        printf("|                           SYMBOL TABLE                                             |\n");
        printf("+------------------------------------------------------------------------------------+ \n");
        printf("Node\tScope\tCAdr\tDAdr\tlabel   Func\tvalue\tVarType\tlinenr\tScopeName\n");
        printf("+------------------------------------------------------------------------------------+ \n");

        printSYM(GlobalSYM, 0);
    }

    if (DBG_AST == TRUE) {
        printf("\n\n+--------------------------------------------------------------------------+\n");
        printf("|                               SYNTAX TREE                                |\n");
        printf("+--------------------------------------------------------------------------+\n");
        printf("Node  \tlineNr\tCAdr OpT ValueC\tValNum\tScpL\tScpN\n");
        printf("+--------------------------------------------------------------------------+\n");

        printAST(ASTprogram, 0);
    }

    if (DBG_SOURCE == TRUE) {
        printf("\n\n+------------------------------------------------------------------------------------+\n");
        printf("|                           SOURCE LISTING                                           |\n");
        printf("+------------------------------------------------------------------------------------+ \n");
        printf("CAdr Code       Line Source\n");
        printf("+------------------------------------------------------------------------------------+ \n");


        printSourceListing(GlobalSRC, 0);
    }

    copyToELF(GlobalSRC, 0);

    closeSourceFile();

    // -------------------------------------------------------------------------------- 
    //  Finalize ELF output
    // --------------------------------------------------------------------------------


    addDataSectionToSegment();
    addTextSectionToSegment();
    addNote();
    writeElfFile();

    exit(0);

    // -------------------------------------------------------------------------------- 
    //  END of MAIN Routine
    // --------------------------------------------------------------------------------
}
