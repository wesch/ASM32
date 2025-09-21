
#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief Implements all parser functions for ASM32.
/// 
/// This file contains routines to:
/// - Build the abstract syntax tree (AST).
/// - Manage the symbol table and scope hierarchy.
/// - Perform symbol lookup and validation.
/// - Provide semantic checks for opcodes, modes, and registers.


// =================================================================================
// AST Node Management
// =================================================================================

/// \brief Create a new AST node.
/// \param type The type of AST node (instruction, identifier, literal, etc.).
/// \param value String value (identifier name, literal, etc.).
/// \param valnum Numeric value (if applicable).
/// \return Pointer to the newly allocated ASTNode.
/// 
/// Each AST node stores source location, scope, and semantic attributes.
/// Fatal error if memory allocation fails.
ASTNode* createASTnode(AST_NodeType type, const char* value, int valnum) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fatalError("malloc failed");
    }
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    node->type = type;
    node->value = strdup(value);
    node->valnum = valnum;
    node->lineNr = lineNr;
    node->column = column;
    node->scopeLevel = currentScopeLevel;
    node->scopeName = strdup(currentScopeName);
    node->symNodeAdr = scopeTab[currentScopeLevel];
    node->children = NULL;
    node->codeAdr = codeAdr;
    node->operandType = operandType;
    node->childCount = 0;
    return node;
}

/// \brief Add a child AST node.
/// \param parent Parent AST node.
/// \param child Child AST node to attach.
/// 
/// Expands the parent's child list dynamically.
/// Fatal error if memory reallocation fails.
void addASTchild(ASTNode* parent, ASTNode* child) {
    parent->children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*) * (parent->childCount + 1));
    if (parent->children == NULL) {
        fatalError("realloc failed");
    }
    parent->children[parent->childCount++] = child;
}

/// \brief Recursively free an AST node and all its children.
/// \param node Root of the AST subtree to free.
void freeASTnode(ASTNode* node) {
    if (node) {
        free(node->value);
        for (int i = 0; i < node->childCount; i++) {
            freeASTnode(node->children[i]);
        }
        free(node->children);
        free(node);
    }
}


// =================================================================================
// Symbol Table Management
// =================================================================================

/// \brief Create a new symbol table node.
/// \param type Scope type (program, module, function, directive).
/// \param label Symbol label.
/// \param func Function name associated with the symbol.
/// \param value Symbol value as string.
/// \param linenr Source line number.
/// \return Pointer to the new SymNode.
SymNode* createSYMnode(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {
    SymNode* node = (SymNode*)malloc(sizeof(SymNode));
    if (node == NULL) {
        fatalError("malloc failed");
    }
    node->type = type;
    node->scopeLevel = currentScopeLevel;
    strcpy(node->scopeName, currentScopeName);
    strcpy(node->label, label);
    strcpy(node->func, func);
    strcpy(node->value, value);
    node->varType = varType;
    node->lineNr = lineNr;
    node->codeAdr = codeAdr;
    node->dataAdr = dataAdr;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

/// \brief Add a child symbol node to the symbol table.
/// \param parent Parent symbol node.
/// \param child Child symbol node to attach.
/// 
/// Expands the parent's child list dynamically.
/// Fatal error if memory reallocation fails.
void addSYMchild(SymNode* parent, SymNode* child) {
    parent->children = (SymNode**)realloc(parent->children, sizeof(SymNode*) * (parent->childCount + 1));
    if (parent->children == NULL) {
        fatalError("realloc failed");
    }
    parent->children[parent->childCount++] = child;
}

/// \brief Add a new scope to the symbol table.
/// \param type Scope type (program, module, function, etc.).
/// \param label Scope label.
/// \param func Function name associated with the scope.
/// \param value Additional value.
/// \param linenr Line number where scope was defined.
void addScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {
    currentScopeLevel++;
    if (currentScopeLevel > maxScopeLevel) {
        printf("Too many Scope Levels\n");
        return;
    }
    strcpy(currentScopeName, label);
    strcpy(scopeNameTab[currentScopeLevel], currentScopeName);
    scopeTab[currentScopeLevel] = createSYMnode(type, label, func, value, linenr);
    addSYMchild(scopeTab[currentScopeLevel - 1], scopeTab[currentScopeLevel]);
}

/// \brief Attach a directive to the current scope.
/// \param type Directive type.
/// \param label Directive label.
/// \param func Directive function.
/// \param value Directive value.
/// \param linenr Line number of directive.
void addDirectiveToScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {
    directive = createSYMnode(type, label, func, value, linenr);
    addSYMchild(scopeTab[currentScopeLevel], directive);
}


// =================================================================================
// Symbol Lookup
// =================================================================================

/// \brief Search a symbol recursively from the current scope upward.
/// \param node Current symbol node.
/// \param label Symbol label to search for.
/// \param depth Recursive depth (for traversal).
/// 
/// Updates global variables if a match is found.
void searchSymAll(SymNode* node, char* label, int depth) {
    if (!node) return;
    if (strcmp(node->label, label) == 0 &&
        node->scopeLevel <= searchScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        strcpy(symFunc, node->func);
        strcpy(symValue, node->value);
        dataAdr = node->dataAdr;
        symcodeAdr = node->codeAdr;
        symFound = TRUE;
        return;
    }
    for (int i = 0; i < node->childCount; i++) {
        searchSymAll(node->children[i], label, depth + 1);
    }
}

/// \brief Search for a symbol in the current scope only.
/// \param node Current symbol node.
/// \param label Symbol label to search for.
/// \param depth Recursive depth (for traversal).
void searchSymLevel(SymNode* node, char* label, int depth) {
    if (!node) return;
    if (strcmp(node->label, label) == 0 &&
        node->scopeLevel == searchScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        symcodeAdr = node->codeAdr;
        symFound = TRUE;
        return;
    }
    for (int i = 0; i < node->childCount; i++) {
        searchSymLevel(node->children[i], label, depth + 1);
    }
}

/// \brief Search for a symbol in the symbol table, moving up through scopes if needed.
/// \param node Root of the symbol table.
/// \param label Symbol label.
/// \return TRUE if found, FALSE otherwise.
bool searchSymbol(SymNode* node, char* label) {
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
    symFound = FALSE;

    while (symFound == FALSE) {
        searchSymAll(scopeTab[searchScopeLevel], label, 0);
        if (symFound == TRUE) {
            return TRUE;
        }
        searchScopeLevel--;
        strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
        if (searchScopeLevel == 0) break;
    }
    return symFound;
}

/// \brief Print the symbol table hierarchy.
/// \param node Current symbol node.
/// \param depth Indentation level for pretty-printing.
void printSYM(SymNode* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
    printf("%s:\t%d\t%04x\t%04x\t%-8s %-6s\t%.5s\t%d\t%d\t%s\n",
        (node->type == SCOPE_PROGRAM) ? "P" :
        (node->type == SCOPE_MODULE) ? "M" :
        (node->type == SCOPE_FUNCTION) ? "F" :
        (node->type == SCOPE_DIRECT) ? "D" :
        "Unknown", node->scopeLevel, node->codeAdr, node->dataAdr, node->label, node->func, node->value, node->varType, node->lineNr, node->scopeName);
    if (node->type == SCOPE_PROGRAM) {
        printf("-->Source File: %s\n", node->value);
    }
    for (int i = 0; i < node->childCount; i++) {
        printSYM(node->children[i], depth + 1);
    }
}


// =================================================================================
// Parsing Utilities
// =================================================================================

/// \brief Skip tokens until end-of-line.
/// 
/// Consumes tokens until a T_EOL token is encountered.
/// Resets line error flag afterward.
void skipToEOL() {
    while (tokTyp != T_EOL) {
        fetchToken();
    }
    lineERR = FALSE;
}

/// \brief Check if the current token is a reserved word.
/// \return TRUE if not reserved, FALSE if it matches an opcode, directive, or reserved keyword.
bool checkReservedWord() {
    strToUpper(label);

    // Check Opcodes
    int num_Opcode = (sizeof(opCodeTab) / sizeof(opCodeTab[0]));
    for (int i = 0; i < num_Opcode; i++) {
        if (strcmp(label, opCodeTab[i].mnemonic) == 0) {
            return FALSE;
        }
    }

    // Check Directives
    int num_dircode = (sizeof(dirCodeTab) / sizeof(dirCodeTab[0]));
    for (int i = 0; i < num_dircode; i++) {
        if (strcmp(label, dirCodeTab[i].directive) == 0) {
            return FALSE;
        }
    }

    // Check Reserved Words
    int num_resWord = (sizeof(resWordTab) / sizeof(resWordTab[0]));
    for (int i = 0; i < num_resWord; i++) {
        if (strcmp(label, resWordTab[i].resWord) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

/// \brief Validate opcode against addressing mode.
/// 
/// Certain opcodes may not be valid with byte/word/halfword suffixes.
/// If invalid, an error is reported.
void checkOpcodeMode() {
    if (strcmp(opCode, "SUB") == 0) {
        return;
    }
    int x = opCode[strlen(opCode) - 1];
    if (mode == 0 || mode == 1) {
        if (x == 'H' || x == 'W' || x == 'B') {
            snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s mode %d combination ", opCode, mode);
            processError(errmsg);
        }
    }
}


// =================================================================================
// Register Validation
// =================================================================================

/// \brief Check if the current token is a valid general-purpose register.
/// \return TRUE if valid, FALSE otherwise.
/// 
/// Recognizes R0–R31, or resolves register aliases from the symbol table.
bool checkGenReg() {
    int reg = 0;
    strToUpper(token);

    if ((token[0] == 'R' && token[1] != 'L') && (token[0] == 'R' && token[1] != 'E')) {
        if (strlen(token) == 3) {
            if (isdigit(token[1]) && isdigit(token[2])) {
                reg = ((token[1] - '0') * 10 + token[2] - '0');
                return TRUE;
            }
            return FALSE;
        }
        else if (strlen(token) == 2) {
            if (isdigit(token[1])) {
                reg = token[1] - '0';
                return TRUE;
            }
            return FALSE;
        }
        return FALSE;
    }
    else {
        searchScopeLevel = currentScopeLevel;
        if (searchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {
                strcpy(token, symValue);
                return checkGenReg();
            }
            else {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        return FALSE;
    }
}

/// \brief Check if the current token is a valid segment register.
/// \return TRUE if valid, FALSE otherwise.
bool checkSegReg() {
    int reg = 0;
    strToUpper(token);

    if (token[0] == 'S') {
        if (strlen(token) == 3 && isdigit(token[1]) && isdigit(token[2])) {
            reg = ((token[1] - '0') * 10 + token[2] - '0');
            return TRUE;
        }
        else if (strlen(token) == 2 && isdigit(token[1])) {
            reg = token[1] - '0';
            return TRUE;
        }
        return FALSE;
    }
    else {
        searchScopeLevel = currentScopeLevel;
        if (searchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {
                strcpy(token, symValue);
                return checkGenReg();
            }
            else {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        return FALSE;
    }
}

/// \brief Check if the current token is a valid control register.
/// \return TRUE if valid, FALSE otherwise.
bool checkCtrlReg() {
    int reg = 0;
    strToUpper(token);

    if (token[0] == 'C') {
        if (strlen(token) == 3 && isdigit(token[1]) && isdigit(token[2])) {
            reg = ((token[1] - '0') * 10 + token[2] - '0');
            return TRUE;
        }
        else if (strlen(token) == 2 && isdigit(token[1])) {
            reg = token[1] - '0';
            return TRUE;
        }
        return FALSE;
    }
    else {
        searchScopeLevel = currentScopeLevel;
        if (searchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {
                strcpy(token, symValue);
                return checkGenReg();
            }
            else {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        return FALSE;
    }
}

//--------------------------------

// ====================================================================================
//  Expression Parsing and AST Construction
// ====================================================================================

/// @brief Parse a factor in an expression.
/// 
/// This function handles numbers, identifiers, parentheses, and negation.
/// It may resolve identifiers against the current scope and determine whether
/// they are constants, global memory, or local memory.
/// 
/// @return The computed value of the parsed factor.
///
int64_t parseFactor() {
    int64_t n = 0;
    intmax_t tmp;
    char* endptr;
    errno = 0;

    if (tokTyp == T_LPAREN) {
        fetchToken();
        n = parseExpression();

        if (tokTyp == T_RPAREN) {
            fetchToken();
            return n;
        }
    }
    else if (tokTyp == T_NEG) {
        n = ~n;
    }
    else {
        if (tokTyp == T_NUM) {
            tmp = strtoimax(token, &endptr, 0);
            n = (int64_t)tmp;
        }
        else if (tokTyp == T_IDENTIFIER) {
            searchScopeLevel = currentScopeLevel;
            if (searchSymbol(scopeTab[searchScopeLevel], token)) {
                if (strcmp(symFunc, "EQU") == 0) {
                    strcpy(token, symValue);
                    varType = V_VALUE;
                }
                else if (strcmp(symFunc, "WORD") == 0 ||
                    strcmp(symFunc, "HALF") == 0 ||
                    strcmp(symFunc, "BYTE") == 0) {
                    // Variables: global (DP) or local (SP) based on scope
                    strcpy(varName, token);
                    varType = (currentScopeType == SCOPE_MODULE) ? V_MEMGLOBAL : V_MEMLOCAL;
                }
                else {
                    snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s ", token);
                    processError(errmsg);
                    return 0;
                }
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s ", token);
                processError(errmsg);
                return 0;
            }

            // Resolve based on type
            if (varType == V_VALUE) {
                n = atoll(token);
            }
            else if (varType == V_MEMGLOBAL) {
                // TODO: Confirm HALF and BYTE alignment in memory
                n = dataAdr;
            }
            else if (varType == V_MEMLOCAL) {
                // Placeholder for local memory handling
            }
        }
        fetchToken();
    }
    return n;
}

/// @brief Parse a term in an expression.
/// 
/// A term can involve multiplication, division, modulo, or bitwise AND.
/// 
/// @return The computed value of the parsed term.
///
int64_t parseTerm() {
    int64_t first = parseFactor();
    if (lineERR) return 0;

    for (;;) {
        if (tokTyp == T_MUL) {
            fetchToken();
            first *= parseFactor();
        }
        else if (tokTyp == T_DIV) {
            fetchToken();
            first /= parseFactor();
        }
        else if (tokTyp == T_MOD) {
            fetchToken();
            first %= parseFactor();
        }
        else if (tokTyp == T_AND) {
            fetchToken();
            first &= parseExpression();
        }
        else {
            return first;
        }
    }
}

/// @brief Parse a full expression.
/// 
/// Expressions can include addition, subtraction, OR, and XOR.  
/// Handles operator precedence by combining terms appropriately.
/// 
/// @return The computed value of the parsed expression.
///
int64_t parseExpression() {
    int64_t first = parseTerm();
    if (lineERR) return 0;
    if (is_negative == TRUE) {
        first = -first;
    }

    for (;;) {
        if (tokTyp == T_PLUS) {
            fetchToken();
            first += parseTerm();
        }
        else if (tokTyp == T_MINUS) {
            fetchToken();
            first -= parseTerm();
        }
        else if (tokTyp == T_OR) {
            fetchToken();
            first |= parseExpression();
        }
        else if (tokTyp == T_XOR) {
            fetchToken();
            first ^= parseExpression();
        }
        else {
            return first;
        }
    }
}

/// @brief Advance to the next token in the token stream.
/// 
/// Updates the global token information (`token`, `tokTyp`, `lineNr`, `column`)
/// from the linked token list.
///
void fetchToken() {
    ptr_t = ptr_t->next;
    strcpy(token, ptr_t->token);
    tokTyp = ptr_t->tokTyp;
    lineNr = ptr_t->lineNumber;
    column = ptr_t->column;
}

/// @brief Print the Abstract Syntax Tree (AST).
/// 
/// Recursively traverses the AST and prints each node, indented by depth,
/// including its type, line number, code address, operand type, and scope.
/// 
/// @param node  Pointer to the AST node to print.
/// @param depth Current depth in the tree (used for indentation).
///
void printAST(ASTNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    value = (node->type == NODE_INSTRUCTION) ? 0 : node->valnum;

    printf("%s:\t%4d\t%04x %1d   %.6s\t%4d\t%d\t%s\n",
        (node->type == NODE_PROGRAM) ? "Prg" :
        (node->type == NODE_INSTRUCTION) ? "Ins" :
        (node->type == NODE_DIRECTIVE) ? "Dir" :
        (node->type == NODE_CODE) ? "Cod" :
        (node->type == NODE_OPERATION) ? "OpC" :
        (node->type == NODE_OPERAND) ? "Op " :
        (node->type == NODE_OPTION) ? "Opt" :
        (node->type == NODE_MODE) ? "Mod" :
        (node->type == NODE_LABEL) ? "Lab" :
        (node->type == NODE_ADDR) ? "Adr" :
        (node->type == NODE_ALIGN) ? "Alg" :
        (node->type == NODE_ENTRY) ? "Ent" :
        "Unknown",
        node->lineNr, node->codeAdr, node->operandType,
        node->value, (int)value, node->scopeLevel, node->scopeName);

    for (int i = 0; i < node->childCount; i++) {
        printAST(node->children[i], depth + 1);
    }
}

// ====================================================================================
//  Debugging Helpers
// ====================================================================================

/// @brief Print the human-readable representation of a token code.
/// 
/// @param i The token code.
///
void printTokenCode(int i) {
    switch (i) {
    case NONE:          printf("NONE"); break;
    case T_IDENTIFIER:  printf("IDENTIFIER"); break;
    case T_NUM:         printf("NUM"); break;
    case T_COMMA:       printf("COMMA"); break;
    case T_COLON:       printf("COLON"); break;
    case T_QUOT:        printf("QUOT"); break;
    case T_DOT:         printf("DOT"); break;
    case T_UNDERSCORE:  printf("UNDERSCORE"); break;
    case T_LPAREN:      printf("LPAREN"); break;
    case T_RPAREN:      printf("RPAREN"); break;
    case T_COMMENT:     printf("COMMENT"); break;
    case T_DIRECTIVE:   printf("DIRECTIVE"); break;
    case T_OPCODE:      printf("OPCODE"); break;
    case T_LABEL:       printf("LABEL"); break;
    case T_MINUS:       printf("MINUS"); break;
    case T_PLUS:        printf("PLUS"); break;
    case T_MUL:         printf("MUL"); break;
    case T_DIV:         printf("DIV"); break;
    case T_NEG:         printf("NEG"); break;
    case T_MOD:         printf("MOD"); break;
    case T_OR:          printf("OR"); break;
    case T_AND:         printf("AND"); break;
    case T_XOR:         printf("XOR"); break;
    case T_EOL:         printf("EOL"); break;
    case EOF:           printf("EOF"); break;
    default:            printf("-----  unknown symbol  -----");
    }
}

/// @brief Print the symbol representation of an expression token.
/// 
/// @param i The token code.
///
void printSymbolTokenCode(int i) {
    switch (i) {
    case T_LPAREN:     printf("("); break;
    case T_RPAREN:     printf(")"); break;
    case T_MINUS:      printf("-"); break;
    case T_PLUS:       printf("+"); break;
    case T_MUL:        printf("*"); break;
    case T_DIV:        printf("/"); break;
    case T_NEG:        printf("~"); break;
    case T_MOD:        printf("%%"); break;
    case T_OR:         printf("|"); break;
    case T_AND:        printf("&"); break;
    case T_XOR:        printf("^"); break;
    case T_EOL:        printf("EOL"); break;
    case EOF:          printf("EOF"); break;
    case T_IDENTIFIER: printf("ID"); break;
    default:           printf("-----  unknown symbol  -----");
    }
}

// ====================================================================================
//  Label and Instruction Parsing
// ====================================================================================

/// @brief Parse a label and normalize it to uppercase.
///
void parseLabel() {
    strcpy(label, tokenSave);
    strToUpper(label);
}

/// @brief Parse the `ADDIL` and `LDIL` instructions.
/// 
/// Expected syntax:  
/// `OP regR, value (22-bit signed)`
/// 
/// - First operand must be a general-purpose register.  
/// - Second operand can be an immediate value or memory reference.
///
void parseADDIL_LDIL() {
    // Parse register operand
    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regR %s ", token);
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Expect a comma
    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    fetchToken();
    value = parseExpression();
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
    if (lineERR) return;

    // Immediate operand
    if (varType == V_VALUE) {
        if (DBG_PARSER) printf("[%" PRId64 "]", value);
        operandType = OT_VALUE;
        ASTop2 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop2);
    }
    // Memory operand
    else {
        if (DBG_PARSER) printf("Variable %s global/local %d\n", varName, varType);
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = createASTnode(NODE_OPERAND, varName, value);
        addASTchild(ASTinstruction, ASTop2);

        operandType = OT_NOTHING;
        ASTmode = createASTnode(NODE_MODE, "", mode);
        addASTchild(ASTinstruction, ASTmode);
        return;
    }
}

/// @file
/// @brief Parsing routines for branch-related instructions (CBR, BR, BRK, BR/BV, BVE, BE).
///
/// These functions handle syntax checking, AST node construction, and error reporting
/// for various conditional and unconditional branch instructions in the assembler.


// ========================================================================================
//  Parse CBR, CBRU
//      Syntax: OP[.<opt1>] regA, regB, ofs
// ========================================================================================
/**
 * @brief Parse a CBR/CBRU instruction.
 *
 * CBR and CBRU are conditional branches that optionally take an instruction option,
 * followed by two registers (regA and regB), and an offset operand.
 *
 * The function validates the presence of registers and delimiters (commas, parentheses),
 * constructs AST nodes for operands, and reports errors when invalid tokens are encountered.
 */
void parseCBR_CBRU() {
    // Handle optional instruction option (e.g., .<opt1>)
    if (tokTyp == T_DOT) {
        fetchToken();
        if (tokTyp == T_IDENTIFIER) {
            if (DBG_PARSER) printf("OPT1 %s ", token);
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Expect register A
    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regA %s ", token);
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Expect comma
    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Expect register B
    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regB %s ", token);
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Expect comma
    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Handle optional negative sign before offset
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {
        is_negative = TRUE;
        fetchToken();
    }

    // Parse offset operand
    if (tokTyp == T_NUM) {
        value = strToNum(token);
        if (is_negative) value = -value;
        operandType = OT_VALUE;
        ASTop3 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop3);
    }
    else if (tokTyp == T_IDENTIFIER) {
        operandType = OT_LABEL;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
    }
}


// ========================================================================================
//  Parse B
//      Syntax: OP ofs [,regR]
// ========================================================================================
/**
 * @brief Parse a B (unconditional branch) instruction.
 *
 * The branch takes an offset operand, optionally followed by a register.
 * This function handles numeric offsets, labels, and validates register syntax.
 */
void parseB() {
    // Optional negative offset
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {
        is_negative = TRUE;
        fetchToken();
    }

    // Parse offset
    if (tokTyp == T_NUM) {
        value = strToNum(token);
        if (is_negative) value = -value;
        operandType = OT_VALUE;
        ASTop1 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop1);
    }
    else if (tokTyp == T_IDENTIFIER) {
        operandType = OT_LABEL;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
    }

    // Optional comma + register
    fetchToken();
    if (tokTyp == T_COMMA) {
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) printf("regR %s ", token);
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else if (tokTyp != T_EOL) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
}


// ========================================================================================
//  Parse BRK
//      Syntax: BRK info1, info2
// ========================================================================================
/**
 * @brief Parse a BRK instruction.
 *
 * BRK takes two numeric operands. This function validates syntax and constructs AST nodes
 * for both operands.
 */
void parseBRK() {
    if (tokTyp == T_NUM) {
        value = strToNum(token);
        if (is_negative) value = -value;
        operandType = OT_VALUE;
        ASTop1 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop1);
    }

    // Expect comma + second numeric operand
    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    if (tokTyp == T_NUM) {
        value = strToNum(token);
        if (is_negative) value = -value;
        operandType = OT_VALUE;
        ASTop2 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop2);
    }
}


// ========================================================================================
//  Parse BR, BV
//      Syntax: OP (regB)[, regR]
// ========================================================================================
/**
 * @brief Parse a BR or BV instruction.
 *
 * These branch instructions require a base register enclosed in parentheses,
 * optionally followed by a register operand.
 */
void parseBR_BV() {
    // Expect LPAREN
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Expect register B
    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regR %s ", token);
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Expect RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Optional trailing register
    fetchToken();
    if (tokTyp == T_COMMA) {
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) printf("regB %s ", token);
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
    }
    else if (tokTyp != T_EOL) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
    }
}


// ========================================================================================
//  Parse BVE
//      Syntax: BVE regA (regB)[, regR]
// ========================================================================================
/**
 * @brief Parse a BVE instruction.
 *
 * BVE requires a register A, a base register B enclosed in parentheses,
 * and optionally an additional register R.
 */
void parseBVE() {
    // Expect regA
    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regA %s ", token);
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Expect (regB)
    fetchToken();
    if (tokTyp != T_LPAREN) { processError("Expected '('"); return; }
    fetchToken();
    if (checkGenReg() == TRUE) {
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    if (tokTyp != T_RPAREN) { processError("Expected ')'"); return; }

    // Optional regR
    fetchToken();
    if (tokTyp == T_COMMA) {
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) printf("regR %s ", token);
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
        }
    }
    else if (tokTyp != T_EOL) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
    }
}


// ========================================================================================
//  Parse BE
//      Syntax: BE ofs(regA, regB)[, regR]
// ========================================================================================
/**
 * @brief Parse a BE instruction.
 *
 * BE instructions branch to an offset computed from an immediate or variable
 * plus the values in registers A and B, with an optional register R.
 */
void parseBE() {
    // Parse expression for offset
    value = parseExpression();
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
    if (lineERR) return;

    // Handle value vs variable
    if (varType == V_VALUE) {
        operandType = OT_VALUE;
        ASTop1 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop1);
    }
    else {
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop1 = createASTnode(NODE_OPERAND, varName, value);
        addASTchild(ASTinstruction, ASTop1);
        return;
    }

    // Expect (regA, regB)
    if (tokTyp != T_LPAREN) { processError("Expected '('"); return; }
    fetchToken();
    if (!checkGenReg()) { processError("Invalid regA"); return; }
    ASTop2 = createASTnode(NODE_OPERAND, token, 0);
    addASTchild(ASTinstruction, ASTop2);

    fetchToken();
    if (tokTyp != T_COMMA) { processError("Expected ','"); return; }
    fetchToken();
    if (!checkGenReg()) { processError("Invalid regB"); return; }
    ASTop3 = createASTnode(NODE_OPERAND, token, 0);
    addASTchild(ASTinstruction, ASTop3);

    fetchToken();
    if (tokTyp != T_RPAREN) { processError("Expected ')'"); return; }

    // Optional regR
    fetchToken();
    if (tokTyp == T_COMMA) {
        fetchToken();
        if (checkGenReg() == TRUE) {
            ASTop4 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop4);
        }
        else {
            processError("Invalid regR");
        }
    }
    else if (tokTyp != T_EOL) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
    }
}


///--------------------------------------------------------

// =================================================================================
// Parse PCA / PTLB Instruction
// =================================================================================

/// @brief Parse the PCA or PTLB instruction.
/// @details
/// **Syntax:**
/// - `PCA[.<TM>] regA([regS,]regB)`
/// - `PTLB[.<TM>] regA([regS,]regB)`
///
/// **Operands:**
/// - `regA` : destination register
/// - `regS` : optional segment register (if present, must be followed by a comma)
/// - `regB` : source register inside parentheses
///
/// **Options:**
/// - `.TM` : optional instruction modifier
///
/// This function enforces correct parenthesis usage, comma placement,
/// and ensures valid register types.

void parsePCA_PTLB() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regA

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        fetchToken();
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
    }
    else if (checkSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
}

// =================================================================================
// Parse MR Instruction
// =================================================================================

/// @brief Parse the MR (Move Register) instruction.
/// @details
/// **Syntax:**
/// - `MR regR, <regS|regC>`
/// - `MR <regS|regC>, regR`
///
/// **Operands:**
/// - `regR` : general-purpose register
/// - `regS` : segment register
/// - `regC` : control register
///
/// **Options:**
/// - Implicit options `M`, `D`, `DM` are inserted depending on register pairing:
///   - `M` : general → control
///   - `D` : segment → general
///   - `DM` : control → general
///
/// Errors are raised if invalid combinations of register classes are detected.


void parseMR() {

    // check if general register

    if (checkGenReg() == TRUE) {

        mode = 0;
    }
    else if (checkSegReg() == TRUE) {
        mode = 1;
    }
    else if (checkCtrlReg() == TRUE) {
        mode = 2;
    } 
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    operandType = OT_REGISTER;
    ASTop1 = createASTnode(NODE_OPERAND, token, 0);
    addASTchild(ASTinstruction, ASTop1);



    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    char MRopt[10];
    strcpy(MRopt, "  ");

    // first register is a general register


    if (mode == 0) {
        if (checkSegReg() == TRUE) {
            strcpy(MRopt, "");

        }
        else if (checkCtrlReg() == TRUE) {
            strcpy(MRopt, "M");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else if (mode == 1) {
        if (checkGenReg() == TRUE) {
            strcpy(MRopt, "D");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else if (mode == 2) {
        if (checkGenReg() == TRUE) {
            strcpy(MRopt, "DM");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    operandType = OT_REGISTER;
    ASTop2 = createASTnode(NODE_OPERAND, token, 0);
    addASTchild(ASTinstruction, ASTop2);

    operandType = OT_NOTHING;
    ASTopt1 = createASTnode(NODE_OPTION, MRopt, 1);
    addASTchild(ASTinstruction, ASTopt1);

 //   printf("MR %d    %s\n",  lineNr,  MRopt);


}

// =================================================================================
// Parse MST Instruction
// =================================================================================

/// @brief Parse the MST (Move to Special) instruction.
/// @details
/// **Syntax:**
/// - `MST regR, regB`
/// - `MST.<S|C> regR, val`
///
/// **Operands:**
/// - `regR` : destination register
/// - `regB` : source general-purpose register
/// - `val`  : immediate value (when `.S` or `.C` option is used)
///
/// **Options:**
/// - `.S` or `.C` : specify special value assignment instead of register transfer
///
/// This parser handles both register-to-register and register-to-value forms,
/// including expression parsing for constants.

void parseMST() {

    mode = 0;                       // no option found

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
            mode = 1;
        }
        fetchToken();
    }
    else {
        strcpy(buffer, " ");
        operandType = OT_NOTHING;
        ASTopt1 = createASTnode(NODE_OPTION, buffer, 1);
        addASTchild(ASTinstruction, ASTopt1);
    }
    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    if (mode == 1) {
        value = parseExpression();

        strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
        if (lineERR) return;
        if (varType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%" PRId64 "]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
        }

    }
    else {
        // Here should be regB 

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
}


// =================================================================================
// Parse PRB Instruction
// =================================================================================

/// @brief Parse the PRB instruction.
/// @details
/// **Syntax:**
/// - `PRB[.<WI>] regR, ([regS], regB) [, regA]`
///
/// **Operands:**
/// - `regR` : base register
/// - `regS` : optional segment register
/// - `regB` : primary source register inside parentheses
/// - `regA` : optional trailing register after parentheses
///
/// **Options:**
/// - `.WI` : optional width indicator
///
/// This function validates nested parentheses, optional segment usage,
/// and trailing register arguments.

void parsePRB() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        fetchToken();
    }

 
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
    }
    else if (checkSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    // check for  a comma

    fetchToken();
    if (tokTyp == T_COMMA) {
        fetchToken();
        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }


}

// =================================================================================
// Parse LSID Instruction
// =================================================================================

/// @brief Parse the LSID instruction.
/// @details
/// **Syntax:**
/// - `LSID regR, regB`
///
/// **Operands:**
/// - `regR` : destination register
/// - `regB` : base register
///
/// This instruction form is simple and expects two general-purpose registers,
/// separated by a comma.

void parseLSID() {
    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

}

// =================================================================================
// Parse ITLB Instruction
// =================================================================================

/// @brief Parse the ITLB instruction.
/// @details
/// **Syntax:**
/// - `ITLB[.T] regR, (regS, regB)`
///
/// **Operands:**
/// - `regR` : general-purpose register
/// - `regS` : segment register
/// - `regB` : base register
///
/// **Options:**
/// - `.T` : optional flag to indicate a translation operation
///
/// This function enforces parentheses grouping `(regS, regB)`
/// and ensures proper operand classification.

void parseITLB() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }
    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a LPAREN

    fetchToken();
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regS

    if (tokTyp == T_IDENTIFIER) {

        if (checkSegReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regS %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
}

/// ---------------------------------------------------------


// =================================================================================
// Parse SHLA Instruction
// =================================================================================

/// @brief Parse the SHLA instruction.
/// @details
/// Syntax:
/// - `SHLA[.<LO>] regR, regA, regB, shamt`
///
/// The parser:
/// - Optionally processes an instruction option (`.<LO>`).
/// - Expects destination register `regR`.
/// - Expects source registers `regA` and `regB`.
/// - Expects a shift amount (`shamt`).
///
/// @note Errors are reported via processError().

void parseSHLA() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    value = parseExpression();
    if (varType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop4 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop4);
        strcpy(tokenSave, token);
    }

}

// =================================================================================
// Parse LDPA Instruction
// =================================================================================

/// @brief Parse the LDPA instruction.
/// @details
/// Syntax:
/// - `LDPA regR, ofs([regs], regB)`
///
/// The parser:
/// - Expects destination register `regR`.
/// - Handles optional negative offset.
/// - Validates offset register (`regA`).
/// - Parses addressing mode with base register (`regB`) or segment register + general register.
/// - Requires closing parenthesis `)`.
///
/// @note Errors are reported if syntax or registers are invalid.

void parseLDPA() {

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        fetchToken();
    }

    // Here should be regA

    if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            fetchToken();
    }
  
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
    }
    else if (checkSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

}

// =================================================================================
// Parse LDR / STC Instruction
// =================================================================================

/// @brief Parse the LDR or STC instructions.
/// @details
/// Syntax:
/// - `OP[.<opt>] regR, ofs([regs], regB)`
///
/// The parser:
/// - Optionally processes an instruction modifier (e.g., `.opt`).
/// - Expects destination register `regR`.
/// - Parses an immediate offset or variable.
/// - Expects base register or segment register + index register inside parentheses.
/// - Validates closing parenthesis `)`.
///
/// @note Used for both load (LDR) and store (STC) instructions.

void parseLDR_STC() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    value = parseExpression();
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    if (lineERR) return;
    if (varType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop2 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop2);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", varName, varType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = createASTnode(NODE_OPERAND, varName, value);
        addASTchild(ASTinstruction, ASTop2);
        return;
    }
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
    }
    else if (checkSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

}

// =================================================================================
// Parse LDO Instruction
// =================================================================================

/// @brief Parse the LDO instruction.
/// @details
/// Syntax:
/// - `LDO regR, ofs(regB)`
///
/// The parser:
/// - Expects destination register `regR`.
/// - Parses immediate offset or variable.
/// - Requires a base register `regB` inside parentheses.
/// - Validates proper token sequence and reports unexpected end-of-line or missing arguments.
///
/// @note Adds addressing mode information to AST.

void parseLDO() {

    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    value = parseExpression();
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    if (lineERR) return;
    if (varType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop2 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop2);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", varName, varType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = createASTnode(NODE_OPERAND, varName, value);
        addASTchild(ASTinstruction, ASTop2);
        operandType = OT_NOTHING;
        ASTmode = createASTnode(NODE_MODE, "", mode);
        addASTchild(ASTinstruction, ASTmode);
        return;
    }
    fetchToken();
    if (tokTyp == T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected EOL ");
        processError(errmsg);
        return;
    }
    else if (tokTyp == T_RPAREN) {

        snprintf(errmsg, sizeof(errmsg), "Missing Argument between brackets ");
        processError(errmsg);
        return;
    }

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);

    }
    
}


// =================================================================================
// Parse DSR Instruction
// =================================================================================

/// @brief Parse the DSR instruction.
/// @details
/// Syntax:
/// - `DSR regR, regB, regA`
/// - `DSR.A regR, regB, regA, shAmt`
///
/// The parser:
/// - Optionally parses instruction option (e.g., `.A`).
/// - Expects destination and source registers (`regR`, `regB`, `regA`).
/// - If option present, expects an additional shift amount operand (`shAmt`).
///
/// @note Reports syntax and register errors via processError().

void parseDSR() {

    bool was_option = FALSE;

    // check for option 

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);

            was_option = TRUE;
        }
        fetchToken();
    }

    // Here should be regR 
    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regA

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }


    
    if (was_option == TRUE) {
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }
    }

}

///------------------------------------------------------------------------------

// ============================================================================
//  Parse EXTR
// ============================================================================

/// @brief Parse the **EXTR** instruction.
///
/// **Syntax**:
/// - `EXTR regR, regB, pos, len`
/// - `EXTR.A regR, regB, len`
///
/// The function:
/// - Optionally detects the `.A` option modifier.
/// - Validates and parses `regR` (destination register).
/// - Validates and parses `regB` (source register).
/// - Parses `pos` and `len` as integer expressions or values.
/// - Populates the AST with operand and option nodes.
/// - Reports detailed errors if tokens are missing or invalid.

void parseEXTR() {

    bool was_option = FALSE;

    // check for option 

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);

            was_option = TRUE;
        }
        fetchToken();
    }

    // Here should be regR 
    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    // Here should be a value

    value = parseExpression();
    if (varType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop3 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop3);
        strcpy(tokenSave, token);
    }

    if (was_option == FALSE) 
    {
        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }

    }

}

// ============================================================================
//  Parse GATE
// ============================================================================

/// @brief Parse the **GATE** instruction.
///
/// **Syntax**:
/// - `GATE regR, ofs`
///
/// The function:
/// - Validates and parses `regR` (target register).
/// - Parses `ofs` as either:
///   - A numeric constant (possibly negative), or
///   - A label reference.
/// - Populates the AST with appropriate operand nodes.
/// - Reports detailed errors on invalid register names or missing tokens.

void parseGATE() {

    // Here should be regR 
    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        fetchToken();
    }

    if (tokTyp == T_NUM) {                  // operand is a number

        value = strToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop2 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop2);
    }
    else if (tokTyp == T_IDENTIFIER) {        // operand is a label 

        operandType = OT_LABEL;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);
    }
}

// ============================================================================
//  Parse LD / ST
// ============================================================================

/// @brief Parse **LD** (load) and **ST** (store) instructions.
///
/// **Syntax**:
/// - `OP[.M] regR, ofs([regS], regB)`
/// - `OP[.M] regR, regA([regS], regB)`
///
/// The function:
/// - Detects the optional `.M` modifier.
/// - Parses `regR` (destination/source register).
/// - Handles second operand:
///   - Either an immediate offset expression, a global/local variable reference, or
///   - Another register.
/// - Validates the address form `(regS, regB)` with possible segment register usage.
/// - Builds AST nodes for operands and options.
/// - Reports errors if the instruction format is invalid.

void parseLD_ST() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        fetchToken();
    }

    // Either Register
    
    mode = 0;

    if (tokTyp == T_IDENTIFIER) {
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            mode = 1;
            fetchToken();
        }
    }
    if (mode == 0) {

        value = parseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;
        if (varType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%" PRId64 "]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
        }
        else {

            if (DBG_PARSER) {
                printf("Variable %s global/local %d\n", varName, varType);
            }
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = createASTnode(NODE_OPERAND, varName, value);
            addASTchild(ASTinstruction, ASTop2);
            return;
        }
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
    }
    else if (checkSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
}

// ============================================================================
//  Parse LDA / STA
// ============================================================================

/// @brief Parse **LDA** (load address) and **STA** (store address) instructions.
///
/// **Syntax**:
/// - `OP[.M] regR, ofs(regB)`
/// - `OP[.M] regR, regA(regB)`
///
/// The function:
/// - Detects the optional `.M` modifier.
/// - Parses `regR` (destination/source register).
/// - Parses second operand:
///   - Either an offset expression, variable reference, or
///   - Another register.
/// - Validates the address form `(regB)` used in memory addressing.
/// - Builds AST nodes for operands and options.
/// - Provides detailed error messages on invalid registers or syntax issues.

void parseLDA_STA() {

    // Check Option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        fetchToken();
    }

    // Either Register

    mode = 0;

    if (tokTyp == T_IDENTIFIER) {
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            mode = 1;
            fetchToken();
        }
    }
    if (mode == 0) {

        value = parseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;
        if (varType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%d]", (int)value);
            }
            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
        }
        else {

            if (DBG_PARSER) {
                printf("Variable %s global/local %d\n", varName, varType);
            }
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = createASTnode(NODE_OPERAND, varName, value);
            addASTchild(ASTinstruction, ASTop2);
            return;
        }
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB or regS 1-3

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a RPAREN
    fetchToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
}

// -----------------------------------------------------------------------

// ============================================================================
//  Parse CMR
// ============================================================================

/// @brief Parse the `CMR` instruction.
/// @details
/// Syntax:
///   - `CMR.<opt3> regR, regA, regB`
///
/// Supported conditions for `<opt3>`:
///   - EQ : b == 0  
///   - LT : b < 0 (signed)  
///   - NE : b != 0  
///   - LE : b <= 0 (signed)  
///   - GT : b > 0 (signed)  
///   - GE : b >= 0 (signed)  
///   - HI : b > 0 (unsigned)  
///   - HE : b >= 0 (unsigned)  
///
/// The function checks for an optional condition code, followed by three
/// registers (`regR`, `regA`, `regB`). It adds AST nodes for each operand.

void parseCMR() {


    // check for option

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }


}

// ============================================================================
//  Parse DEP
// ============================================================================

/// @brief Parse the `DEP` instruction with different addressing modes.
/// @details
/// Syntax options:
///   - `DEP.[Z] regR, regB, pos, len` — default mode  
///   - `DEP.[A[Z]] regR, regB, len` — mode 1  
///   - `DEP.[I[Z]] regR, value, pos, len` — mode 2  
///   - `DEP.[IA[Z]] regR, value, len` — mode 3  
///
/// Behavior:
///   - Optional suffixes `A`, `I`, and `Z` modify the addressing mode.  
///   - Depending on the mode, operands may be registers or immediate values.  
///   - The function validates tokens, parses values with `parseExpression()`,
///     and attaches AST nodes for operands and the final addressing mode.
void parseDEP() {

    // check for option

    mode = 0;

    if (tokTyp == T_DOT) {

        fetchToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);

            for (int j = 0; j < strlen(token); j++) {

                switch (token[j]) {

                case 'A':   mode = mode + 1; break;
                case 'I':   mode = mode + 2; break;
                case 'Z':   bool Z = TRUE; break;
                }
            }
        }
        fetchToken();
    }   



    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    if (mode == 0) {

        // Here should be regB

        if (tokTyp == T_IDENTIFIER) {

            if (checkGenReg() == TRUE) {

                if (DBG_PARSER) {
                    printf("regR %s ", token);
                }
                operandType = OT_REGISTER;
                ASTop2 = createASTnode(NODE_OPERAND, token, 0);
                addASTchild(ASTinstruction, ASTop2);
                strcpy(tokenSave, token);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
                processError(errmsg);
                return;
            }
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }

        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }


    }
    else if (mode == 1) {

        // Here should be regB

        if (tokTyp == T_IDENTIFIER) {

            if (checkGenReg() == TRUE) {

                if (DBG_PARSER) {
                    printf("regR %s ", token);
                }
                operandType = OT_REGISTER;
                ASTop2 = createASTnode(NODE_OPERAND, token, 0);
                addASTchild(ASTinstruction, ASTop2);
                strcpy(tokenSave, token);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
                processError(errmsg);
                return;
            }
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }

        // Here should be a comma

        fetchToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

    }
    else if (mode == 2) {

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }

        // Here should be a comma

        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }
    }
    else if (mode == 3) {

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }

        // Here should be a comma

        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            processError(errmsg);
            return;
        }
        fetchToken();

        // Here should be a value

        value = parseExpression();
        if (varType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
    }
    
    operandType = OT_NOTHING;
    ASTmode = createASTnode(NODE_MODE, "", mode);
    addASTchild(ASTinstruction, ASTmode);
}

// ============================================================================
//  Parse DIAG
// ============================================================================

/// @brief Parse the `DIAG` instruction.
/// @details
/// Syntax:
///   - `DIAG regR, regA, regB, info`
///
/// Description:
///   - `regR`, `regA`, and `regB` must be valid general-purpose registers.  
///   - `info` is an immediate value (parsed with `parseExpression()`).  
///   - The instruction is primarily used for diagnostics or debugging output.
///
/// The function enforces the required operand sequence and builds AST nodes
/// for each operand.

void parseDIAG() {

    // Here should be regR 

    if (checkGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }

    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (checkGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    // Here should be a comma

    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();
    value = parseExpression();
    if (varType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop4 = createASTnode(NODE_OPERAND, ">", value);
        addASTchild(ASTinstruction, ASTop4);
        strcpy(tokenSave, token);
    }
}

// -----------------------------------------------------------------------

// =====================================================================================
//  Parse Arithmetic/Logic Instructions
// =====================================================================================

/// @brief Parses arithmetic and logical instructions with multiple addressing modes.
///
/// Supported instructions include:
/// - ADD, ADC, AND, CMP, CMPU, OR, SBC, SUB, XOR
///
/// ### Supported addressing modes:
/// - **Mode 0**: `OP<.XX> regR, value (17bitS)`  
/// - **Mode 1**: `OP<.XX> regR, regA, regB`  
/// - **Mode 2**: `OP[W | H | B]<.XX> regR, regA(regB)`  
/// - **Mode 3**: `OP[W | H | B]<.XX> regR, ofs(regB)`  
///
/// The parser extracts options, validates registers and operands, and
/// attaches them to the AST. Invalid syntax triggers error reporting.

void parseModInstr() {

    char        option[MAX_WORD_LENGTH];        // option value 

    // ----------------------------------------------------
    // Step 1: Parse first optional modifier (e.g., .XX)
    // ----------------------------------------------------
    if (tokTyp == T_DOT) {
        fetchToken();
        if (tokTyp == T_IDENTIFIER) {
            if (DBG_PARSER) printf("OPT1 %s ", token);
            operandType = OT_NOTHING;
            ASTopt1 = createASTnode(NODE_OPTION, token, 1);
            addASTchild(ASTinstruction, ASTopt1);
        }
        fetchToken();
    }

    // ----------------------------------------------------
    // Step 2: Parse second optional modifier
    // ----------------------------------------------------
    if (tokTyp == T_DOT) {
        fetchToken();
        if (tokTyp == T_IDENTIFIER) {
            if (DBG_PARSER) printf("OPT2 %s ", token);
            operandType = OT_NOTHING;
            ASTopt2 = createASTnode(NODE_OPTION, token, 2);
            addASTchild(ASTinstruction, ASTopt2);
        }
        fetchToken();
    }

    // ----------------------------------------------------
    // Step 3: Parse destination register (regR)
    // ----------------------------------------------------
    if (tokTyp == T_IDENTIFIER) {
        if (checkGenReg() == TRUE) {
            if (DBG_PARSER) printf("regR %s ", token);
            operandType = OT_REGISTER;
            ASTop1 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            processError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }


    // ----------------------------------------------------
    // Step 4: Expect a comma after regR
    // ----------------------------------------------------
    fetchToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        processError(errmsg);
        return;
    }
    fetchToken();

    // ----------------------------------------------------
        // Step 5: Parse operand (regA, immediate, or memory)
        // ----------------------------------------------------
    mode = 0;
    is_negative = FALSE;

    if (tokTyp == T_MINUS) {
        is_negative = TRUE;
        fetchToken();
    }

    // --- Operand could be a register (regA) ---
    if (tokTyp == T_IDENTIFIER && checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regA %s ", token);
        operandType = OT_REGISTER;
        ASTop2 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop2);

        fetchToken();
        if (tokTyp == T_EOL) {  // Mode 1 with implicit regB = regR
            mode = 1;
            if (DBG_PARSER) printf("M-%d\n", mode);
            checkOpcodeMode();

            strcpy(token, tokenSave);
            ASTop3 = createASTnode(NODE_OPERAND, token, 0);
            addASTchild(ASTinstruction, ASTop3);

            ASTmode = createASTnode(NODE_MODE, "", mode);
            addASTchild(ASTinstruction, ASTmode);
            return;
        }
        else if (tokTyp == T_COMMA) {
            mode = 1;
        }
        else if (tokTyp == T_LPAREN) {
            mode = 2;
        }
    }

    // --- Operand could be an immediate or memory reference ---
    if (mode == 0) {
        value = parseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;

        if (varType == V_VALUE) {
            if (DBG_PARSER) printf("[%" PRId64 "]", value);
            operandType = OT_VALUE;
            ASTop2 = createASTnode(NODE_OPERAND, ">", value);
            addASTchild(ASTinstruction, ASTop2);
        }
        else {
            if (DBG_PARSER) printf("Variable %s global/local %d\n", varName, varType);
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = createASTnode(NODE_OPERAND, varName, value);
            addASTchild(ASTinstruction, ASTop2);

            ASTmode = createASTnode(NODE_MODE, "", mode);
            addASTchild(ASTinstruction, ASTmode);
            return;
        }

        if (tokTyp == T_EOL) {
            mode = 0;
            if (DBG_PARSER) printf("M-%d\n", mode);
            checkOpcodeMode();
            ASTmode = createASTnode(NODE_MODE, "", mode);
            addASTchild(ASTinstruction, ASTmode);
            return;
        }
        else if (tokTyp == T_LPAREN) {
            mode = 3;
        }
    }

    // ----------------------------------------------------
    // Step 6: Parse regB inside parentheses (if present)
    // ----------------------------------------------------
    fetchToken();
    if (tokTyp == T_EOL) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected EOL ");
        processError(errmsg);
        return;
    }
    else if (tokTyp == T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Missing Argument between brackets ");
        processError(errmsg);
        return;
    }

    if (checkGenReg() == TRUE) {
        if (DBG_PARSER) printf("regB %s ", token);
        ASTop3 = createASTnode(NODE_OPERAND, token, 0);
        addASTchild(ASTinstruction, ASTop3);

        if (mode == 1) {
            fetchToken();
            if (tokTyp != T_EOL) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
                processError(errmsg);
                return;
            }
        }
        else if (mode == 2 || mode == 3) {
            fetchToken();
            if (tokTyp != T_RPAREN) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
                processError(errmsg);
                return;
            }
        }

        if (DBG_PARSER) printf("M-%d\n", mode);
        checkOpcodeMode();

        ASTmode = createASTnode(NODE_MODE, "", mode);
        addASTchild(ASTinstruction, ASTmode);
        return;
    }
    return;
}

// =====================================================================================
//  Parse General Instruction Dispatcher
// =====================================================================================

/// @brief Main dispatcher for parsing instructions.
///
/// This function:
/// - Validates the opcode against the opcode table.
/// - Builds the AST nodes for instructions, labels, and operations.
/// - Routes parsing to the appropriate specialized function depending on the
///   instruction type (e.g., `parseModInstr()`, `parseEXTR()`, `parseLD_ST()`, etc.).
/// - Handles symbol table updates for labels.
/// - Ensures proper error handling and recovery on invalid instructions.
///
/// @note Updates `codeAdr` to advance the program counter by 4 bytes after
///       successful parsing.
void parseInstruction() {

    int i = 0;
    lineERR = FALSE;
    varType = V_VALUE;

    if (codeExist == FALSE) {
        snprintf(errmsg, sizeof(errmsg), ".CODE directive missing");
        processError(errmsg);
        skipToEOL();
        return;
    }

    strcpy(opCode, tokenSave);
    strToUpper(opCode);
    if (DBG_PARSER) printf("%03d I %s ", lineNr, opCode);

    // ----------------------------------------------------
    // Step 1: Validate opcode against table
    // ----------------------------------------------------
    int num_Opcode = sizeof(opCodeTab) / sizeof(opCodeTab[0]);
    bool opCode_found = FALSE;
    for (i = 0; i < num_Opcode; i++) {
        if (strcmp(opCode, opCodeTab[i].mnemonic) == 0) {
            opCode_found = TRUE;
            break;
        }
    }
    if (!opCode_found) {
        snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s ", opCode);
        processError(errmsg);
        skipToEOL();
        return;
    }

    binInstr = opCodeTab[i].binInstr;
    opInstrType = opCodeTab[i].instrType;

    // ----------------------------------------------------
    // Step 2: Build AST nodes for instruction and labels
    // ----------------------------------------------------
    ASTinstruction = createASTnode(NODE_INSTRUCTION, "", binInstr);
    addASTchild(ASTprogram, ASTinstruction);

    if (strcmp(func_entry, "") != 0) {
        ASTlabel = createASTnode(NODE_LABEL, func_entry, 0);
        addASTchild(ASTinstruction, ASTlabel);
        strcpy(func_entry, "");
    }
    if (strcmp(label, "") != 0) {
        symFound = FALSE;
        searchScopeLevel = currentScopeLevel;
        searchSymLevel(scopeTab[searchScopeLevel], label, 0);

        if (!symFound) {
            strcpy(dirCode, "LABEL");
            directive = createSYMnode(SCOPE_DIRECT, label, dirCode, "", lineNr);
            addSYMchild(scopeTab[currentScopeLevel], directive);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Label %s already defined ", label);
            processError(errmsg);
            strcpy(label, "");
            skipToEOL();
            return;
        }
        ASTlabel = createASTnode(NODE_LABEL, label, 0);
        addASTchild(ASTinstruction, ASTlabel);
        strcpy(label, "");
    }

    // ----------------------------------------------------
    // Step 3: Build AST node for operation
    // ----------------------------------------------------
    ASToperation = createASTnode(NODE_OPERATION, opCode, opInstrType);
    addASTchild(ASTinstruction, ASToperation);

    // ----------------------------------------------------
    // Step 4: Dispatch to specific parser based on instruction type
    // ----------------------------------------------------
    switch (opInstrType) {
    case ADD: case ADC: case SUB: case SBC:
    case AND: case OR:  case XOR: case CMP: case CMPU:
        parseModInstr(); break;
    case ADDIL: case LDIL: parseADDIL_LDIL(); break;
    case B: parseB(); break;
    case GATE: parseGATE(); break;
    case BR: case BV: parseBR_BV(); break;
    case CBR: case CBRU: parseCBR_CBRU(); break;
    case BVE: parseBVE(); break;
    case EXTR: parseEXTR(); break;
    case DEP: parseDEP(); break;
    case LDR: case STC: parseLDR_STC(); break;
    case BE: parseBE(); break;
    case BRK: parseBRK(); break;
    case DSR: parseDSR(); break;
    case SHLA: parseSHLA(); break;
    case PCA: case PTLB: parsePCA_PTLB(); break;
    case CMR: parseCMR(); break;
    case DIAG: parseDIAG(); break;
    case ITLB: parseITLB(); break;
    case LDO: parseLDO(); break;
    case LSID: parseLSID(); break;
    case PRB: parsePRB(); break;
    case LDPA: parseLDPA(); break;
    case MR: parseMR(); break;
    case MST: parseMST(); break;
    case RFI: break; // No operands
    case LD: case ST: parseLD_ST(); break;
    case LDA: case STA: parseLDA_STA(); break;
    default:
        printf("not yet implemented\n");
        break;
    }

    // ----------------------------------------------------
    // Step 5: Finalize instruction parsing
    // ----------------------------------------------------
    if (lineERR) {
        skipToEOL();
        return;
    }
    codeAdr += 4;
    skipToEOL();
    return;
}

// =================================================================================
// ParseDirective
// =================================================================================

/// @brief Parse assembler directives.
/// @details
/// This function identifies and processes directives encountered in the
/// assembly source. Directives control program organization, memory layout,
/// symbol definitions, and data initialization.
///
/// The function performs the following tasks:
/// - Validates the directive against a directive table (`dirCodeTab`).
/// - Ensures directives are allowed in the current program type context.
/// - Updates symbol tables and scope information.
/// - Writes values or data into the ELF data section with proper alignment.
/// - Reports errors and warnings for invalid usage.
///
/// **Supported Directives:**
/// - `GLOBAL`   : Marks program as standalone.
/// - `MODULE`   : Marks program as module.
/// - `CODE`     : Defines code section attributes (ADDR, ALIGN, ENTRY).
/// - `DATA`     : Defines data section attributes (ADDR, ALIGN).
/// - `ALIGN`    : Adjusts current data address to alignment boundary.
/// - `EQU`/`REG`: Defines constants or registers in the symbol table.
/// - `BUFFER`   : Allocates and initializes a memory buffer.
/// - `BYTE`     : Allocates 1-byte data values.
/// - `HALF`     : Allocates 2-byte data values (aligned).
/// - `WORD`     : Allocates 4-byte data values (aligned).
/// - `DOUBLE`   : Allocates 8-byte data values (aligned).
/// - `STRING`   : Allocates null-terminated string data.
/// - `END`      : Marks the end of assembly input.
///
/// @note
/// All data-writing directives ensure proper alignment and padding
/// in the ELF section. Errors are raised if alignment or usage rules
/// are violated.
void ParseDirective() {
    int i = 0;                  ///< General-purpose loop counter
    int align;                  ///< Alignment value for .ALIGN or section alignment
    int dataAdrOld;             ///< Stores previous data address before alignment
    const char* ptr = elfData;  ///< Pointer to ELF data buffer (unused directly here)

    int buf_size;               ///< Size of buffer for .BUFFER directive
    int buf_init;               ///< Initialization value for buffer data

    varType = V_VALUE;          ///< Default variable type set to "value"

    // Normalize directive name
    strcpy(dirCode, tokenSave);
    strToUpper(dirCode);

    // Lookup directive in table
    int num_dircode = (sizeof(dirCodeTab) / sizeof(dirCodeTab[0]));
    bool dirCode_found = FALSE;
    for (i = 0; i < num_dircode; i++) {
        if (strcmp(dirCode, dirCodeTab[i].directive) == 0) {
            dirCode_found = TRUE;
            directiveType = dirCodeTab[i].directNum;
            break;
        }
    }

    // Handle unknown directives
    if (dirCode_found == FALSE) {
        snprintf(errmsg, sizeof(errmsg), "Invalid directive %s", token);
        processError(errmsg);
        skipToEOL();
        return;
    }

    // Process recognized directive
    if (dirCode_found == TRUE) {
        switch (directiveType) {

        // ---------------------------------------------------------------------
        // Program structure directives
        // ---------------------------------------------------------------------

        case D_GLOBAL:
            /// Marks the program as standalone (cannot coexist with MODULE).
            if (prgType == P_UNDEFINED) {
                prgType = P_STANDALONE;
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                processError(errmsg);
                skipToEOL();
                return;
            }
            break;

        case D_MODULE:
            /// Marks the program as a module (cannot coexist with GLOBAL).
            if (prgType == P_UNDEFINED) {
                prgType = P_MODULE;
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Standalone program", token);
                processError(errmsg);
                skipToEOL();
                return;
            }
            break;



        // ---------------------------------------------------------------------
        // Section definitions
        // ---------------------------------------------------------------------

        case D_CODE:

            codeExist = TRUE;

            /// Defines code section attributes: ADDR, ALIGN, ENTRY.
            if (prgType == P_STANDALONE) {

                // Build AST nodes for .CODE
                ASTcode = createASTnode(NODE_CODE, "CODE", 0);
                addASTchild(ASTprogram, ASTcode);
                if (strcmp(label, "") != 0) {
                    symFound = FALSE;
                    searchScopeLevel = currentScopeLevel;
                    searchSymLevel(scopeTab[searchScopeLevel], label, 0);

                    if (!symFound) {
                        strcpy(dirCode, "LABEL");
                        directive = createSYMnode(SCOPE_DIRECT, label, dirCode, "", lineNr);
                        addSYMchild(scopeTab[currentScopeLevel], directive);
                    }
                    else {
                        snprintf(errmsg, sizeof(errmsg), "Label %s already defined ", label);
                        processError(errmsg);
                        strcpy(label, "");
                        skipToEOL();
                        return;
                    }
                    ASTlabel = createASTnode(NODE_LABEL, label, 0);
                    addASTchild(ASTcode, ASTlabel);
                    strcpy(label, "");
                }

                // Parse optional parameters (ADDR, ALIGN, ENTRY)
                fetchToken();
                strToUpper(token);
                do {
                    if (strcmp(token, "ADDR") == 0) {
                        fetchToken();
                        elfCodeAddr = parseExpression();
                        strToUpper(token);
                        ASTaddr = createASTnode(NODE_ADDR, "", elfCodeAddr);
                        addASTchild(ASTcode, ASTaddr);
                    }
                    else if (strcmp(token, "ALIGN") == 0) {
                        fetchToken();
                        elfCodeAlign = parseExpression();
                        strToUpper(token);
                        if (elfCodeAlign == 0) {
                            snprintf(errmsg, sizeof(errmsg), "Alignment of 0 not allowed");
                            processError(errmsg);
                            skipToEOL();
                            return;
                        }
                        elfDataAlign = elfCodeAlign; // Default data alignment matches code
                        if ((elfCodeAddr % elfCodeAlign) != 0) {
                            snprintf(errmsg, sizeof(errmsg), "Address %x not aligned by %x", elfCodeAddr, elfCodeAlign);
                            processError(errmsg);
                            skipToEOL();
                            return;
                        }
                        ASTalign = createASTnode(NODE_ALIGN, "", elfCodeAlign);
                        addASTchild(ASTcode, ASTalign);
                    }
                    else if (strcmp(token, "ENTRY") == 0) {
                        if (elfEntryPointStatus == FALSE) {
                            elfEntryPointStatus = TRUE;
                        }
                        else {
                            snprintf(errmsg, sizeof(errmsg), "Entry point already set");
                            processError(errmsg);
                            skipToEOL();
                            return;
                        }
                        ASTentry = createASTnode(NODE_ENTRY, "", 0);
                        addASTchild(ASTcode, ASTentry);
                        fetchToken();
                    }
                    else {
                        snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                        processError(errmsg);
                        skipToEOL();
                        return;
                    }
                    if (tokTyp == T_COMMA) {
                        fetchToken();
                        strToUpper(token);
                    }
                } while (tokTyp != T_EOL);

                if (elfEntryPointStatus == TRUE) {
                    elfEntryPoint = elfCodeAddr;
                }
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                processError(errmsg);
                skipToEOL();
                return;
            }
            break;

        case D_DATA:

            if (dataExist == TRUE) {
                // add datasegment address to segment table
                addSegmentEntry(numSegment, labelDataOld, 'D', elfDataAddrOld, numOfData);
                numSegment++;
                numOfData = 0;
            }


            dataExist = TRUE;

            /// Defines data section attributes: ADDR, ALIGN.
            if (prgType == P_STANDALONE) {

                fetchToken();
                strToUpper(token);
                do {
                    if (strcmp(token, "ADDR") == 0) {
                        fetchToken();
                        elfDataAddr = parseExpression();
                        strToUpper(token);

                    }
                    else if (strcmp(token, "ALIGN") == 0) {
                        fetchToken();
                        elfDataAlign = parseExpression();
                        strToUpper(token);
                        if (elfDataAlign == 0) {
                            snprintf(errmsg, sizeof(errmsg), "Alignment of 0 not allowed");
                            processError(errmsg);
                            skipToEOL();
                            return;
                        }
                        if ((elfDataAddr % elfDataAlign) != 0) {
                            snprintf(errmsg, sizeof(errmsg), "Address %x not aligned by %x", elfDataAddr, elfDataAlign);
                            processError(errmsg);
                            skipToEOL();
                            return;
                        }

                    }
                    else {
                        snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                        processError(errmsg);
                        skipToEOL();
                        return;
                    }
                    if (tokTyp == T_COMMA) {
                        fetchToken();
                        strToUpper(token);
                    }
                } while (tokTyp != T_EOL);

                // create ELF structures for DATA
                createDataSegment();
                strcpy(buffer, ".data.");
                strcat(buffer, label);
                createDataSection(buffer);
                addDataSectionToSegment();
            }
            else {

                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                processError(errmsg);
                skipToEOL();
                return;
            }
            strcpy(labelDataOld, label);
            elfDataAddrOld = elfDataAddr;


            break;

        // ---------------------------------------------------------------------
        // Alignment directives
        // ---------------------------------------------------------------------

        case D_ALIGN:
            /// Aligns data address to specified boundary (optionally with 'K' suffix).
            fetchToken();
            align = atoi(token);
            fetchToken();
            strToUpper(token);
            if (strcmp(token, "K") == 0) {
                align = align * 1024;
            }
            dataAdr = ((dataAdr / align) * align) + align;
            if ((align % 2) != 0) {
                snprintf(errmsg, sizeof(errmsg), "alignment %s not a power of 2", token);
                processWarning(errmsg);
            }
            break;

        // ---------------------------------------------------------------------
        // Symbol definition directives
        // ---------------------------------------------------------------------

        case D_EQU:
        case D_REG:
            /// Defines constants or registers in the current scope.
            fetchToken();
            if (checkReservedWord()) {
                addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "Symbol %s is a reserved word", label);
                processError(errmsg);
                skipToEOL();
                return;
            }
            break;

        // ---------------------------------------------------------------------
        // Memory allocation and initialization
        // ---------------------------------------------------------------------



            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }


        case D_BUFFER:
            /// Allocates and initializes a buffer in the ELF data section.
            
            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }
            
            fetchToken();
            strToUpper(token);
            do {
                if (strcmp(token, "SIZE") == 0) {
                    fetchToken();
                    buf_size = parseExpression();
                    strToUpper(token);
                }
                else if (strcmp(token, "INIT") == 0) {
                    fetchToken();
                    buf_init = parseExpression();
                    strToUpper(token);
                }
                else {
                    snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                    processError(errmsg);
                    skipToEOL();
                    return;
                }
                if (tokTyp == T_COMMA) {
                    fetchToken();
                    strToUpper(token);
                }
            } while (tokTyp != T_EOL);

            // Align to word boundary
            dataAdrOld = dataAdr;
            if ((dataAdr % 4) != 0) {
                dataAdr = ((dataAdr / 4) * 4) + 4;
            }
            if ((dataAdrOld - dataAdr) != 0) {
                strcpy(elfData, "\0\0\0\0\0\0\0\0");
                addDataSectionData(elfData, dataAdr - dataAdrOld);
            }

            // Write symbol table entry
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Allocate memory for buffer
            elfBuffer = (char*)malloc(buf_size);
            elfDataLength = 0;

            // Initialize buffer
            do {
                elfBuffer[elfDataLength] = buf_init & 0xFF;
                elfDataLength++;
                buf_size--;
            } while (buf_size > 0);

            addDataSectionData(elfBuffer, elfDataLength);
            dataAdr = (dataAdr + elfDataLength);
            numOfData += elfDataLength;
            break;



        // ---------------------------------------------------------------------
        // Data definition directives
        // ---------------------------------------------------------------------
        // (BYTE, HALF, WORD, DOUBLE, STRING)
        // These allocate data of fixed sizes, align addresses, and write values
        // into the ELF data section in big-endian format.

        case D_BYTE:

            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                varType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                varType = V_MEMLOCAL;
            }
            fetchToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                fetchToken();
            }

            // expression calculation 
            value = parseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);


            // Write in SYMTAB
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format
            elfData[0] = value & 0xFF;
            elfDataLength = 1;

            addDataSectionData(elfData,elfDataLength);

            // adjust data address
            if ((dataAdr % 1) == 0) {
                dataAdr = (dataAdr + 1);
            }
            numOfData += 1;
            break;


        case D_HALF:

            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                varType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                varType = V_MEMLOCAL;
            }
            fetchToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                fetchToken();
            }

            // expression calculation 
            value = parseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on half word boundary
            dataAdrOld = dataAdr;
            if ((dataAdr % 2) != 0) {
                dataAdr = ((dataAdr / 2) * 2) + 2;
            }
            if ((dataAdrOld - dataAdr) != 0) {
                strcpy(elfData, "\0\0\0\0\0\0\0\0");
                addDataSectionData(elfData, dataAdr - dataAdrOld);
            }

            // Write in SYMTAB
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format
            elfData[0] = (value >> 8) & 0xFF;
            elfData[1] = value & 0xFF;
            elfDataLength = 2;

            addDataSectionData(elfData, elfDataLength);

            // adjust data address
            if ((dataAdr % 2) == 0) {
                dataAdr = (dataAdr + 2);
            }
            numOfData += 2;
            break;

        case D_WORD:

            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                varType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                varType = V_MEMLOCAL;
            }
            fetchToken();

            // Check if negative value
    
            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                fetchToken();
            }
    
            // expression calculation 
            value = parseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on word boundary
            dataAdrOld = dataAdr;
            if ((dataAdr % 4) != 0) {
                dataAdr = ((dataAdr / 4) * 4) + 4;
            }
            if ((dataAdrOld - dataAdr) != 0) {
                strcpy(elfData, "\0\0\0\0\0\0\0\0");
                addDataSectionData(elfData, dataAdr - dataAdrOld);
            }

            // Write in SYMTAB
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format

            elfData[0] = (value >> 24) & 0xFF;
            elfData[1] = (value >> 16) & 0xFF;
            elfData[2] = (value >> 8) & 0xFF;
            elfData[3] = value & 0xFF;
            elfDataLength = 4;

            addDataSectionData(elfData, elfDataLength);

            // adjust data address
            if ((dataAdr % 4) == 0) {
                dataAdr = (dataAdr + 4);
            }
            numOfData += 4;
            break;

        case D_DOUBLE:

            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                varType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                varType = V_MEMLOCAL;
            }
            fetchToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                fetchToken();
            }

            // expression calculation 
            value = parseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on double word boundary
            dataAdrOld = dataAdr;
            if ((dataAdr % 8) != 0) {
                dataAdr = ((dataAdr / 8) * 8) + 8;
            }
            if ((dataAdrOld - dataAdr) != 0) {
                strcpy(elfData, "\0\0\0\0\0\0\0\0");
                addDataSectionData(elfData, dataAdr - dataAdrOld);
            }

            // Write in SYMTAB
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format

            elfData[0] = (value >> 56) & 0xFF;
            elfData[1] = (value >> 48) & 0xFF;
            elfData[2] = (value >> 40) & 0xFF;
            elfData[3] = (value >> 32) & 0xFF;
            elfData[4] = (value >> 24) & 0xFF;
            elfData[5] = (value >> 16) & 0xFF;
            elfData[6] = (value >> 8) & 0xFF;
            elfData[7] = value & 0xFF;
            elfDataLength = 8;

            addDataSectionData(elfData, elfDataLength);

            // adjust data address
            if ((dataAdr % 8) == 0) {
                dataAdr = (dataAdr + 8);
            }
            numOfData += 8;
            break;

        case D_STRING:
            
            if (dataExist == FALSE) {
                snprintf(errmsg, sizeof(errmsg), ".DATA directive missing");
                processError(errmsg);
                skipToEOL();
                return;
            }

            fetchToken();

            // Align on word boundary
            dataAdrOld = dataAdr;
            if ((dataAdr % 4) != 0) {
                dataAdr = ((dataAdr / 4) * 4) + 4;
            }
            strcpy(elfData, "\0\0\0\0\0\0\0\0");
            addDataSectionData(elfData, dataAdr - dataAdrOld);

            // Write in SYMTAB
            addDirectiveToScope(SCOPE_DIRECT, label, dirCode, token, lineNr);

            strcpy(elfData,token) ;
            elfDataLength = strlen(token) + 1;

            
            addDataSectionData(elfData, elfDataLength);

            // adjust data address
            
            dataAdr = (dataAdr + elfDataLength);
            numOfData += elfDataLength;
            break;

        case D_END:

            break;

        } // end switch
    strcpy(label, "");
    varType = V_VALUE;

    } // end directive found
}