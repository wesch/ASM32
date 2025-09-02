
#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief contains all parser functions 
/// builds the symbol table and the abstract syntax tree (AST)

// Function to create a new AST node
// value is alphnumeric value
// valnum -> numeric value
// valtype -> type of value 0=num, 1=alpha

ASTNode* Create_ASTnode(AST_NodeType type, const char* value, int valnum) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (node == NULL) {
        FatalError("malloc failed");
    }
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    node->type = type;
    node->value = strdup(value);
    node->valnum = valnum;
    node->linenr = lineNr;
    node->column = column;
    node->scopeLevel = currentScopeLevel;
    node->scopeName = strdup(currentScopeName);
    node->symNodeAdr = scopeTab[currentScopeLevel];
    node->children = NULL;
    node->codeAdr = codeAdr;    
    node->operandType = operandType;
    node->child_count = 0;
    return node;
}

// Function to add a child node
void Add_ASTchild(ASTNode* parent, ASTNode* child) {
    parent->children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*) * (parent->child_count + 1));
    if (parent->children == NULL) {
        FatalError("realloc failed");
    }
    parent->children[parent->child_count++] = child;
}

// Function to free an AST node
void Free_ASTnode(ASTNode* node) {
    if (node) {
        free(node->value);
        for (int i = 0; i < node->child_count; i++) {
            Free_ASTnode(node->children[i]);
        }
        free(node->children);
        free(node);
    }
}

// Function to create a new Sym_node

SymNode* Create_SYMnode(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {

    SymNode* node = (SymNode*)malloc(sizeof(SymNode));

    if (node == NULL) {
        FatalError("malloc failed");
    }
    node->type = type;
    node->scopeLevel = currentScopeLevel;
    strcpy(node->scopeName, currentScopeName);
    strcpy(node->label, label);
    strcpy(node->func, func);
    strcpy(node->value, value);
    node->vartype = VarType;
    node->linenr = lineNr;
    node->codeAdr = codeAdr;
    node->dataAdr = dataAdr;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

// Function to add a child node

void Add_SYMchild(SymNode* parent, SymNode* child) {

    parent->children = (SymNode**)realloc(parent->children, sizeof(SymNode*) * (parent->child_count + 1));
    if (parent->children == NULL) {
        FatalError("realloc failed");
    }
    parent->children[parent->child_count++] = child;
}


// Add new Scope

void AddScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {
    currentScopeLevel++;
    if (currentScopeLevel > maxScopeLevel) {
        printf("Too many Scope Levels\n");
        return;
    }
    strcpy(currentScopeName, label);
    strcpy(scopeNameTab[currentScopeLevel], currentScopeName);
    scopeTab[currentScopeLevel] = Create_SYMnode(type, label, func, value, linenr);
    Add_SYMchild(scopeTab[currentScopeLevel - 1], scopeTab[currentScopeLevel]);
}

// Add directive and link to scope

void AddDirective(SYM_ScopeType type, char* label, char* func, const char* value, int linenr) {

    directive = Create_SYMnode(type, label, func, value, linenr);
    Add_SYMchild(scopeTab[currentScopeLevel], directive);

}

// Function search from actual Scopelevel up
void SearchSymAll(SymNode* node, char* label, int depth) {

    if (!node) return;
    if (strcmp(node->label, label) == 0 &&
        node->scopeLevel <= searchScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        //       printf("Val %s Label %s Line %d\n",node->value,label, node->linenr);
        strcpy(symFunc, node->func);
        strcpy(symValue, node->value);
        dataAdr = node->dataAdr;
        symcodeAdr = node->codeAdr;
        symFound = TRUE;
        return;
    }
    else {
        for (int i = 0; i < node->child_count; i++) {
            SearchSymAll(node->children[i], label, depth + 1);
        }
    }

}

// Function search on actual Scopelevel

void SearchSymLevel(SymNode* node, char* label, int depth) {

    if (!node) return;
    if (strcmp(node->label, label) == 0 &&
        node->scopeLevel == searchScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        symcodeAdr = node->codeAdr;
        symFound = TRUE;
        return;
    }
    else {
        for (int i = 0; i < node->child_count; i++) {
            SearchSymLevel(node->children[i], label, depth + 1);

        }

    }
}

// Funktion Symbol Suche in Symboltabelle

bool SearchSymbol(SymNode* node, char* label) {
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
    symFound = FALSE;

    while (symFound == FALSE) {
        SearchSymAll(scopeTab[searchScopeLevel], label, 0);
        if (symFound == TRUE) {
            return TRUE;
        }
        searchScopeLevel--;
        strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
        if (searchScopeLevel == 0) break;
    }
    if (symFound == FALSE) {
        // printf("Symbol %s not found\n", label);
    }
    if (symFound == TRUE) {
       // printf("Symbol %s Value %s\n", symFunc, symValue);
    }
    return symFound;
}

// Function to print the Symtab
void PrintSYM(SymNode* node, int depth) {

    if (DBG_SYMTAB == FALSE) {

        printf("======== SUPPRESSED =========\n");
        return;
    }

    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    printf("%s:\t%d\t%04x\t%04x\t%s\t%.5s\t%.5s\t%d\t%d\t%s\n", 
        (node->type == SCOPE_PROGRAM) ? "P" :
        (node->type == SCOPE_MODULE) ? "M" :
        (node->type == SCOPE_FUNCTION) ? "F" :
        (node->type == SCOPE_DIRECT) ? "D" :
        "Unknown", node->scopeLevel,node->codeAdr, node->dataAdr, node->label, node->func, node->value, node->vartype, node->linenr,node->scopeName);
    if (node->type == SCOPE_PROGRAM) {
        printf("\t\tSource File: %s\n", node->value);
    }

    for (int i = 0; i < node->child_count; i++) {
        PrintSYM(node->children[i], depth + 1);
    }
}


void SkipToEOL() {
    while (tokTyp != T_EOL) {
        GetNextToken();
    }
    lineERR = FALSE;
}

// --------------------------------------------------------------------------------
// Check reserved word
//  checks if token is a reserved word
// --------------------------------------------------------------------------------

bool CheckReservedWord() {

    // Check Opcodes

    StrToUpper(label);

    int num_Opcode = (sizeof(opCodeTab) / sizeof(opCodeTab[0]));
    int i;

    for (i = 0; i < num_Opcode; i++) {

        if (strcmp(label, opCodeTab[i].mnemonic) == 0) {		// opCode found

            return FALSE;
        }
    }

    // Check Dirctives

    int num_dircode = (sizeof(dirCodeTab) / sizeof(dirCodeTab[0]));

    for (i = 0; i < num_dircode; i++) {

        if (strcmp(label, dirCodeTab[i].directive) == 0) {		// dircode found

            return FALSE;
        }

    }

    // Check other reserved words

    int num_resWord = (sizeof(resWordTab) / sizeof(resWordTab[0]));

    for (i = 0; i < num_resWord; i++) {

        if (strcmp(label, resWordTab[i].resWord) == 0) {		// reserved word found

            return FALSE;
        }

    }
    return TRUE;
}

// --------------------------------------------------------------------------------
// Check OpCode vs. Mode
// --------------------------------------------------------------------------------

void CheckOpcodeMode() {
    if (strcmp(opCode, "SUB") == 0) {
        return;
    }
    int x = opCode[strlen(opCode) - 1];
    if (mode == 0 || mode == 1) {


        if (x == 'H' || x == 'W' || x == 'B') {
            snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s mode %d combination ", opCode,mode);
            ProcessError(errmsg);
        }
    }
}

// --------------------------------------------------------------------------------
// Check General Register
//  checks if global token contains a valid general register
// --------------------------------------------------------------------------------

bool CheckGenReg() {

    int reg = 0;                            // register number from checkreg = 0;
   
    StrToUpper(token);
    
    if ((token[0] == 'R' && token[1] != 'L' )  && (token[0] == 'R' && token[1] != 'E')){

        if (strlen(token) == 3) {

            if (isdigit(token[1]) && isdigit(token[2])) {

                reg = ((token[1] - 48) * 10 + token[2] - 48);
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else if (strlen(token) == 2) {

            if (isdigit(token[1])) {

                reg = token[1] - 48;
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else {

            return FALSE;
        }
    }
    else {
        searchScopeLevel = currentScopeLevel;

        if (SearchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {

                strcpy(token, symValue);

                if (CheckGenReg()) {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
}

// --------------------------------------------------------------------------------
// Check Segment Register
//  checks if global token contains a valid Segment register
// --------------------------------------------------------------------------------

bool CheckSegReg() {

    int reg = 0;                            // register number from checkreg = 0;

    StrToUpper(token);

    if (token[0] == 'S') {

        if (strlen(token) == 3) {

            if (isdigit(token[1]) && isdigit(token[2])) {

                reg = ((token[1] - 48) * 10 + token[2] - 48);
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else if (strlen(token) == 2) {

            if (isdigit(token[1])) {

                reg = token[1] - 48;
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else {

            return FALSE;
        }
    }
    else {
        searchScopeLevel = currentScopeLevel;

        if (SearchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {

                strcpy(token, symValue);

                if (CheckGenReg()) {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
}

// --------------------------------------------------------------------------------
// Check Control Register
//  checks if global token contains a valid Control register
// --------------------------------------------------------------------------------

bool CheckCtrlReg() {

    int reg = 0;                            // register number from checkreg = 0;

    StrToUpper(token);

    if (token[0] == 'C') {

        if (strlen(token) == 3) {

            if (isdigit(token[1]) && isdigit(token[2])) {

                reg = ((token[1] - 48) * 10 + token[2] - 48);
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else if (strlen(token) == 2) {

            if (isdigit(token[1])) {

                reg = token[1] - 48;
                return TRUE;
            }
            else {
                return  FALSE;
            }
        }
        else {

            return FALSE;
        }
    }
    else {
        searchScopeLevel = currentScopeLevel;

        if (SearchSymbol(scopeTab[searchScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == 0) {

                strcpy(token, symValue);

                if (CheckGenReg()) {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
}




int64_t  ParseFactor() {
    int64_t  n = 0;
    intmax_t tmp;

    char* endptr;
    errno = 0;

    if (tokTyp == T_LPAREN) {

        GetNextToken(); 
        n = ParseExpression(); 

        if (tokTyp == T_RPAREN) {

            GetNextToken(); 
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
            if (SearchSymbol(scopeTab[searchScopeLevel], token)) {
                printf("");
                if (strcmp(symFunc, "EQU") == 0) {

                    strcpy(token, symValue);
                    VarType = V_VALUE;
                }
                else if (strcmp(symFunc, "WORD") == 0 ||
                    strcmp(symFunc, "HALF") == 0 ||
                    strcmp(symFunc, "BYTE") == 0) {
                    
                    // depending on module (global) or function (local) level
                    // global ofs(DP)
                    // local ofs(SP)
                    strcpy(Variable, token);
                    if (currentScopeType == SCOPE_MODULE) {

                        VarType = V_MEMGLOBAL;
                    }
                    if (currentScopeType == SCOPE_FUNCTION) {

                        VarType = V_MEMLOCAL;
                    }

                }
                else {
                    snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s ", token);
                    ProcessError(errmsg);
                    return 0;
                }
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s ", token);
                ProcessError(errmsg);
                return 0;
            }

            if (VarType == V_VALUE) {
                n = atoll(token);
            }
            else if (VarType == V_MEMGLOBAL) {
                ///< ??? CHeck 
                /// werden HALF und BYTE auch je in enem Wort  als Parameter �bergeben?
                
                n = dataAdr;
            }
            else if (VarType == V_MEMLOCAL) {
                
            }
        }

        GetNextToken();
    }
    return n; 
}

int64_t  ParseTerm() {
    int64_t  first, second;

    first = ParseFactor(); 
    if (lineERR) return 0;
    for (;;) {
        if (tokTyp == T_MUL) {

            GetNextToken(); 
            second = ParseFactor(); 
            first *= second; 
        }
        else if (tokTyp == T_DIV) {

            GetNextToken(); 
            second = ParseFactor(); 
            first /= second; 
        }
        else if (tokTyp == T_MOD) {

            GetNextToken(); 
            second = ParseFactor(); 
            first %= second; 
        }
        else if (tokTyp == T_AND) {

            GetNextToken();
            second = ParseExpression();
            first &= second;
        }

        else {
            return first; 
        }
    }
}

/// @par Process expression
/// 
int64_t  ParseExpression() {
    int64_t  first, second;

    first = ParseTerm(); 
    if (lineERR) return 0;
    if (is_negative == TRUE) {
        first = -first;
    }

    for (;;) {
        if (tokTyp == T_PLUS) {

            GetNextToken(); 
            second = ParseTerm(); 
            first += second; 
        }
        else if (tokTyp == T_MINUS) {

            GetNextToken(); 
            second = ParseTerm(); 
            first -= second; 
        }
        else if (tokTyp == T_OR) {

            GetNextToken();
            second = ParseExpression();
            first |= second;
        }
        else if (tokTyp == T_XOR) {

            GetNextToken();
            second = ParseExpression();
            first ^= second;
        }
        else {
            return first; 
        }
    }
}

/// @par Hole n�chstes Token aus der Liste
/// 
void GetNextToken() {
    ptr_t = ptr_t->next;
    strcpy(token, ptr_t->token);
    tokTyp = ptr_t->tokTyp;
    lineNr = ptr_t->lineNumber;
    column = ptr_t->column;
}

/// @par Function to print the AST
/// 
void PrintAST(ASTNode* node, int depth) {
   
    if (DBG_AST == FALSE) {

        printf("======== SUPPRESSED =========\n");
        return;
    }

    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    value = node->valnum;
    if (node->type == NODE_INSTRUCTION) {
        value = 0;
    }

    printf("%s:\t%4d\t%04x %1d   %.6s\t%4d\t%d\t%s\n", 
        (node->type == NODE_PROGRAM) ?     "Prg" :
        (node->type == NODE_INSTRUCTION) ? "Ins" :
        (node->type == NODE_DIRECTIVE) ?   "Dir" :
        (node->type == NODE_OPERATION) ?   "OpC" :
        (node->type == NODE_OPERAND) ?     "Op " :
        (node->type == NODE_OPTION) ?      "Opt" :
        (node->type == NODE_MODE) ?        "Mod" :
        (node->type == NODE_LABEL) ?       "Lab" :
        "Unknown", node->linenr,node->codeAdr,node->operandType,node->value, value, node->scopeLevel,node->scopeName);

    for (int i = 0; i < node->child_count; i++) {
        PrintAST(node->children[i], depth + 1);
    }
}

// --------------------------------------------------------------------------------
//      PrintTokenCode (code) --> string
//          This function prints the given code in human readable form.
// --------------------------------------------------------------------------------

void PrintTokenCode(int i) {

    switch (i) {

    case NONE:          printf("NONE"); break;
    case T_IDENTIFIER:  printf("%.10s","IDENTIFIER"); break;
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
    case T_EOL:		    printf("%.10s", "EOL"); break;
    case EOF:		    printf("EOF"); break;

    default:		    printf("-----  unknown symbol  -----");
    }
}

// --------------------------------------------------------------------------------
//      PrintExpressTokenCode (code) --> string
//          This function prints the given Expression code in human readable form.
// --------------------------------------------------------------------------------

void PrintSymbolTokenCode(int i) {

    switch (i) {

    case T_LPAREN:        printf("("); break;
    case T_RPAREN:        printf(")"); break;
    case T_MINUS:         printf("-"); break;
    case T_PLUS:          printf("+"); break;
    case T_MUL:           printf("*"); break;
    case T_DIV:           printf("/"); break;
    case T_NEG:           printf("~"); break;
    case T_MOD:           printf("%%"); break;
    case T_OR:            printf("|"); break;
    case T_AND:           printf("&"); break;
    case T_XOR:           printf("^"); break;
    case T_EOL:		      printf("EOL"); break;
    case EOF:		      printf("EOF"); break;
    case T_IDENTIFIER:    printf("ID"); break;

    default:		    printf("-----  unknown symbol  -----");
    }
}

// -------------------------------------------------------------------------------- 
//          Parser ProcessLabel
// --------------------------------------------------------------------------------

void ParseLabel() {
    strcpy(label, tokenSave);
    StrToUpper(label);

}

/// @par Parse ADDIL, LDIL
///     - OP regR, value (22bitS)   

void ParseADDIL_LDIL() {
    
    // Here should be regR 
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    GetNextToken();

    value = ParseExpression();
    
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
    if (lineERR) return;
    if (VarType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", Variable, VarType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
        Add_ASTchild(ASTinstruction, ASTop2);
        operandType = OT_NOTHING;
        ASTmode = Create_ASTnode(NODE_MODE, "", mode);
        Add_ASTchild(ASTinstruction, ASTmode);
        return;
    }

 
}

/// @par Parse CBR, CBRU
///     - OP[.<opt1>] regA,regB,ofs

void ParseCBR_CBRU() {

    if (tokTyp == T_DOT) {                                   // check for  option

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }
    // Here should be regA
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regA %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
        if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }


    // Here should be a comma
    
    GetNextToken();
        if (tokTyp != T_COMMA) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    if (tokTyp == T_NUM) {                  // operand is a number

        value = StrToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else if (tokTyp == T_IDENTIFIER) {        // operand is a label 

        operandType = OT_LABEL;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    

   
}

/// @par Parse B
///     - OP ofs [,regR]

void ParseB() {
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    if (tokTyp == T_NUM) {                  // operand is a number

        value = StrToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop1 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop1);
    }
    else if (tokTyp == T_IDENTIFIER) {        // operand is a label 

        operandType = OT_LABEL;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp == T_COMMA) {
        GetNextToken();
        // Here should be regR 
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else if (tokTyp != T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }


}

/// @par Parse BRK
///     - BRK info1,info2

void ParseBRK() {
    if (tokTyp == T_NUM) {                  // operand is a number

        value = StrToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop1 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop1);
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    if (tokTyp == T_NUM) {                  // operand is a number

        value = StrToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
}



/// @par Parse BR, BV
///     - OP (regB)[ ,regR ]

void ParseBR_BV() {

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    if (tokTyp != T_EOL) {
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
}

/// @par Parse BVE
///     - BVE regA (regB) [ ,regR ]

void ParseBVE() {

    // Here should be regA 
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regA %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be LPAREN
    GetNextToken();
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    // Here should be regB 
    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma
 
    GetNextToken();
    if (tokTyp == T_COMMA) {
        GetNextToken();
        // Here should be regR 
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else if (tokTyp != T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
}

/// @par Parse BE
///     - BE ofs(regA,regB)[,regR]

void ParseBE() {

    value = ParseExpression();
    strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);

    if (lineERR) return;

    if (VarType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop1 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop1);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", Variable, VarType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop1 = Create_ASTnode(NODE_OPERAND, Variable, value);
        Add_ASTchild(ASTinstruction, ASTop1);
        return;
    }

    // Here should be LPAREN

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    // Here should be regA 
    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid 1register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    // Here should be regB 
    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid 2register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma
 
    GetNextToken();
    if (tokTyp == T_COMMA) {
        GetNextToken();
        // Here should be regR 
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid 3register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else if (tokTyp != T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }


}


/// @par Parse PCA, PTLB
///     - PCA[.<TM>] regA([regS,]regB)
///     - PTLB[.<TM>] regA([regS,]regB)

void ParsePCA_PTLB() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regA

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        GetNextToken();
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else if (CheckSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
}

/// @par Parse MR
///     - MR regR,<regS|regC>
///     - MR <regS|regC>,regR


void ParseMR() {

    // check if general register

    if (CheckGenReg() == TRUE) {

        mode = 0;
    }
    else if (CheckSegReg() == TRUE) {
        mode = 1;
    }
    else if (CheckCtrlReg() == TRUE) {
        mode = 2;
    } 
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    operandType = OT_REGISTER;
    ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
    Add_ASTchild(ASTinstruction, ASTop1);



    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    char MRopt[10];
    strcpy(MRopt, "  ");

    // first register is a general register


    if (mode == 0) {
        if (CheckSegReg() == TRUE) {
            strcpy(MRopt, "");

        }
        else if (CheckCtrlReg() == TRUE) {
            strcpy(MRopt, "M");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else if (mode == 1) {
        if (CheckGenReg() == TRUE) {
            strcpy(MRopt, "D");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else if (mode == 2) {
        if (CheckGenReg() == TRUE) {
            strcpy(MRopt, "DM");
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    operandType = OT_REGISTER;
    ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
    Add_ASTchild(ASTinstruction, ASTop2);

    operandType = OT_NOTHING;
    ASTopt1 = Create_ASTnode(NODE_OPTION, MRopt, 1);
    Add_ASTchild(ASTinstruction, ASTopt1);

 //   printf("MR %d    %s\n",  lineNr,  MRopt);


}

/// @par Parse MST
///     - MST   regR,regB
///     - MST.<S|C> regR, val

void ParseMST() {

    mode = 0;                       // no option found

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
            mode = 1;
        }
        GetNextToken();
    }
    else {
        strcpy(buffer, " ");
        operandType = OT_NOTHING;
        ASTopt1 = Create_ASTnode(NODE_OPTION, buffer, 1);
        Add_ASTchild(ASTinstruction, ASTopt1);
    }
    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    if (mode == 1) {
        value = ParseExpression();

        strcpy(currentScopeName, scopeNameTab[searchScopeLevel]);
        if (lineERR) return;
        if (VarType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%" PRId64 "]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
        }

    }
    else {
        // Here should be regB 

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
}


/// @par Parse PRB
///     - PRB[.<WI>] regR,([regS],regB)[,regA]

void ParsePRB() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

 
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else if (CheckSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    // check for  a comma

    GetNextToken();
    if (tokTyp == T_COMMA) {
        GetNextToken();
        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }


}

/// @par Parse LSID
///     - LSID  regR, regB

void ParseLSID() {
    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

}

/// @par Parse ITLB
///     - ITLB[.T] regR,(regS,regB)

void ParseITLB() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }
    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a LPAREN

    GetNextToken();
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regS

    if (tokTyp == T_IDENTIFIER) {

        if (CheckSegReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regS %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
}

/// @par Parse SHLA
///     - SHLA[.<LO> regR, regA, regB, shamt

void ParseSHLA() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    value = ParseExpression();
    if (VarType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop4);
        strcpy(tokenSave, token);
    }

}

/// @par Parse LDPA
///     - LDPA   regR,ofs([regs],regB)

void ParseLDPA() {

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    // Here should be regA

    if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            GetNextToken();
    }
  
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else if (CheckSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

}

/// @par Parse LDR und STC
///     - OP   regR,ofs([regs],regB)

void ParseLDR_STC() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    value = ParseExpression();
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    if (lineERR) return;
    if (VarType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", Variable, VarType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
        Add_ASTchild(ASTinstruction, ASTop2);
        return;
    }
    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else if (CheckSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

}

/// @par Parse LDO
///     - LDO   regR,ofs(regB)

void ParseLDO() {

    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    value = ParseExpression();
    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    if (lineERR) return;
    if (VarType == V_VALUE) {

        if (DBG_PARSER) {
            printf("[%" PRId64 "]", value);
        }
        operandType = OT_VALUE;
        ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else {

        if (DBG_PARSER) {
            printf("Variable %s global/local %d\n", Variable, VarType);
        }
        operandType = OT_MEMGLOB;
        mode = 3;
        ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
        Add_ASTchild(ASTinstruction, ASTop2);
        operandType = OT_NOTHING;
        ASTmode = Create_ASTnode(NODE_MODE, "", mode);
        Add_ASTchild(ASTinstruction, ASTmode);
        return;
    }
    GetNextToken();
    if (tokTyp == T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected EOL ");
        ProcessError(errmsg);
        return;
    }
    else if (tokTyp == T_RPAREN) {

        snprintf(errmsg, sizeof(errmsg), "Missing Argument between brackets ");
        ProcessError(errmsg);
        return;
    }

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);

    }
    
}


/// @par Parse DSR
///     - DSR   regR,regB,regA
///     - DSR.A regR,regB,regA,shAmt

void ParseDSR() {

    bool was_option = FALSE;

    // check for option 

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);

            was_option = TRUE;
        }
        GetNextToken();
    }

    // Here should be regR 
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regA

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }


    
    if (was_option == TRUE) {
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }
    }

}


/// @par Parse EXTR
///     - EXTR   regR,regB,pos,len
///     - EXTR.A regR,regB,len

void ParseEXTR() {

    bool was_option = FALSE;

    // check for option 

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);

            was_option = TRUE;
        }
        GetNextToken();
    }

    // Here should be regR 
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    // Here should be a value

    value = ParseExpression();
    if (VarType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop3);
        strcpy(tokenSave, token);
    }

    if (was_option == FALSE) 
    {
        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }

    }

}

/// @par Parse GATE
///     - OP regR,ofs

void ParseGATE() {

    // Here should be regR 
    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    if (tokTyp == T_NUM) {                  // operand is a number

        value = StrToNum(token);
        if (is_negative == TRUE) {

            value = 0 - value;
        }
        operandType = OT_VALUE;
        ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
    else if (tokTyp == T_IDENTIFIER) {        // operand is a label 

        operandType = OT_LABEL;
        ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop2);
    }
}

/// @par Parse LD, ST
///     - OP[.M] regR, ofs([regS],regB)
///     - OP[.M] regR, regA([regS],regB)

void ParseLD_ST() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    // Either Register
    
    mode = 0;

    if (tokTyp == T_IDENTIFIER) {
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            mode = 1;
            GetNextToken();
        }
    }
    if (mode == 0) {

        value = ParseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;
        if (VarType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%" PRId64 "]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
        }
        else {

            if (DBG_PARSER) {
                printf("Variable %s global/local %d\n", Variable, VarType);
            }
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
            Add_ASTchild(ASTinstruction, ASTop2);
            return;
        }
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else if (CheckSegReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop4 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop4);
        }
        else {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
}

/// @par Parse LDA, STA
///     - OP[.M] regR, ofs(regB)
///     - OP[.M] regR, regA(regB)

void ParseLDA_STA() {

    // Check Option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    is_negative = FALSE;
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }

    // Either Register

    mode = 0;

    if (tokTyp == T_IDENTIFIER) {
        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            mode = 1;
            GetNextToken();
        }
    }
    if (mode == 0) {

        value = ParseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;
        if (VarType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%d]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
        }
        else {

            if (DBG_PARSER) {
                printf("Variable %s global/local %d\n", Variable, VarType);
            }
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
            Add_ASTchild(ASTinstruction, ASTop2);
            return;
        }
    }

    if (tokTyp != T_LPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB or regS 1-3

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);
    }
    else {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a RPAREN
    GetNextToken();
    if (tokTyp != T_RPAREN) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
}

// -----------------------------------------------------------------------

/// @par Parse CMR
/// CMR.<opt3> regR, regA, regB
///  EQ ( b == 0 ) 												
///  LT(b < 0, Signed)
///  NE(b != 0)
///  LE(b <= 0, Signed)
///  GT(b > 0, Signed)
///  GE(b >= 0, Signed)
///  HI(b > 0, Unsigned)
///  HE(b >= 0, Unsigned)

void ParseCMR() {


    // check for option

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }


}

/// @par Parse DEP
/// DEP.[Z] regR, regB, pos, len
/// DEP.[A[Z]] regR, regB, len                  mode 1
/// DEP.[I[Z]] regR, value, pos, len            mode 2
/// DEP.[IA[Z]] regR, value, len                mode 3
/// 
void ParseDEP() {

    // check for option

    mode = 0;

    if (tokTyp == T_DOT) {

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);

            for ( j = 0; j < strlen(token); j++) {

                switch (token[j]) {

                case 'A':

                    mode = mode + 1;
                    break;

                case 'I':

                    mode = mode + 2;
                    break;

                case 'Z':
                    bool Z = TRUE;
                    break;
                }
            }
        }
        GetNextToken();
    }   



    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    if (mode == 0) {

        // Here should be regB

        if (tokTyp == T_IDENTIFIER) {

            if (CheckGenReg() == TRUE) {

                if (DBG_PARSER) {
                    printf("regR %s ", token);
                }
                operandType = OT_REGISTER;
                ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
                Add_ASTchild(ASTinstruction, ASTop2);
                strcpy(tokenSave, token);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
                ProcessError(errmsg);
                return;
            }
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }

        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }


    }
    else if (mode == 1) {

        // Here should be regB

        if (tokTyp == T_IDENTIFIER) {

            if (CheckGenReg() == TRUE) {

                if (DBG_PARSER) {
                    printf("regR %s ", token);
                }
                operandType = OT_REGISTER;
                ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
                Add_ASTchild(ASTinstruction, ASTop2);
                strcpy(tokenSave, token);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
                ProcessError(errmsg);
                return;
            }
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }

        // Here should be a comma

        GetNextToken();
        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

    }
    else if (mode == 2) {

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }

        // Here should be a comma

        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }

        // Here should be a comma


        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop4);
            strcpy(tokenSave, token);
        }
    }
    else if (mode == 3) {

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }

        // Here should be a comma

        if (tokTyp != T_COMMA) {
            snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
            ProcessError(errmsg);
            return;
        }
        GetNextToken();

        // Here should be a value

        value = ParseExpression();
        if (VarType == V_VALUE) {

            operandType = OT_VALUE;
            ASTop3 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
    }
    
    operandType = OT_NOTHING;
    ASTmode = Create_ASTnode(NODE_MODE, "", mode);
    Add_ASTchild(ASTinstruction, ASTmode);
}

/// @par Parse DIAG
///     DIAG regR,regA,regB,info

void ParseDIAG() {

    // Here should be regR 

    if (CheckGenReg() == TRUE) {

        if (DBG_PARSER) {
            printf("regR %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop1);
        strcpy(tokenSave, token);
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regA

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();

    // Here should be regB

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regB %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop3);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }
    GetNextToken();
    value = ParseExpression();
    if (VarType == V_VALUE) {

        operandType = OT_VALUE;
        ASTop4 = Create_ASTnode(NODE_OPERAND, ">", value);
        Add_ASTchild(ASTinstruction, ASTop4);
        strcpy(tokenSave, token);
    }
}

// -----------------------------------------------------------------------

/// @par Parse ADD,ADC,AND,CMP,CMPU,OR,SBC,SUB,XOR
/// OP<.XX> regR, value (17bitS)          ->  mode 0
/// OP<.XX> regR, regA, regB              ->  mode 1
/// OP[W | H | B]<.XX> regR, regA(regB)   ->  mode 2
/// OP[W | H | B]<.XX> regR, ofs(regB)    ->  mode 3

void ParseModInstr() {

    char        option[MAX_WORD_LENGTH];        // option value 

    // check for 1. option

    if (tokTyp == T_DOT) {                                  

        GetNextToken();
        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT1 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt1 = Create_ASTnode(NODE_OPTION, token, 1);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        GetNextToken();
    }

    // check for 2. option

    if (tokTyp == T_DOT) {                                  


        GetNextToken();

        if (tokTyp == T_IDENTIFIER) {

            if (DBG_PARSER) {
                printf("OPT2 %s ", token);
            }
            operandType = OT_NOTHING;
            ASTopt2 = Create_ASTnode(NODE_OPTION, token, 2);
            Add_ASTchild(ASTinstruction, ASTopt2);
        }
        GetNextToken();
    }

    // Here should be regR 

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            if (DBG_PARSER) {
                printf("regR %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
            strcpy(tokenSave, token);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s ", token);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // Here should be a comma

    GetNextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
        ProcessError(errmsg);
        return;
    }

    // this is the common area for all modes

    GetNextToken();

    mode = 0;
    is_negative = FALSE;
    
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        GetNextToken();
    }


    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {
            if (DBG_PARSER) {
                printf("regA %s ", token);
            }
            operandType = OT_REGISTER;
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);

            GetNextToken();
            if (tokTyp == T_EOL) {
                mode = 1;
                if (DBG_PARSER) {
                    printf("M-%d\n", mode);
                }

                CheckOpcodeMode();

                strcpy(token, tokenSave);
                operandType = OT_REGISTER;
                ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
                Add_ASTchild(ASTinstruction, ASTop3);            
                operandType = OT_NOTHING;
                ASTmode = Create_ASTnode(NODE_MODE, "", mode);
                Add_ASTchild(ASTinstruction, ASTmode);
                return;
            }
            else if (tokTyp == T_COMMA) {
                mode = 1;
            }
            else if (tokTyp == T_LPAREN) {
                mode = 2;
            }
        }
    }
    if (mode == 0) {

        value = ParseExpression();
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (lineERR) return;
        if (VarType == V_VALUE) {

            if (DBG_PARSER) {
                printf("[%" PRId64 "]", value);
            }
            operandType = OT_VALUE;
            ASTop2 = Create_ASTnode(NODE_OPERAND, ">", value);
            Add_ASTchild(ASTinstruction, ASTop2);
        }
        else {

            if (DBG_PARSER) {
                printf("Variable %s global/local %d\n",Variable,VarType);
            }
            operandType = OT_MEMGLOB;
            mode = 3;
            ASTop2 = Create_ASTnode(NODE_OPERAND, Variable, value);
            Add_ASTchild(ASTinstruction, ASTop2);
            operandType = OT_NOTHING;
            ASTmode = Create_ASTnode(NODE_MODE, "", mode);
            Add_ASTchild(ASTinstruction, ASTmode);
            return;
        }
        if (tokTyp == T_EOL) {

            mode = 0;
            if (DBG_PARSER) {
                printf("M-%d\n", mode);
            }
            CheckOpcodeMode();
            if (lineERR == TRUE) {
 //               return;
            }
            operandType = OT_NOTHING;
            ASTmode = Create_ASTnode(NODE_MODE, "", mode);
            Add_ASTchild(ASTinstruction, ASTmode);
        
            return;
        }
        else if (tokTyp == T_LPAREN) {

            mode = 3;
        }
    }

    GetNextToken();
    if (tokTyp == T_EOL) {

        snprintf(errmsg, sizeof(errmsg), "Unexpected EOL ");
        ProcessError(errmsg);
        return;
    }
    else if (tokTyp == T_RPAREN) {

        snprintf(errmsg, sizeof(errmsg), "Missing Argument between brackets ");
        ProcessError(errmsg);
        return;
    }

    if (CheckGenReg() == TRUE) {
        if (DBG_PARSER) {
            printf("regB %s ", token);
        }
        operandType = OT_REGISTER;
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);

        if (mode == 1) {
            GetNextToken();
            if (tokTyp != T_EOL) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
                ProcessError(errmsg);
                return;
            }
        }
        else if (mode == 2 || mode == 3) {
            GetNextToken();
            if (tokTyp != T_RPAREN) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s ", token);
                ProcessError(errmsg);
                return;
            }
        }
        if (DBG_PARSER) {
            printf("M-%d\n", mode);
        }
        CheckOpcodeMode();
        if (lineERR) {
 //           return;
        }
        
        ASTmode = Create_ASTnode(NODE_MODE, "", mode);
        Add_ASTchild(ASTinstruction, ASTmode);
        return;

    }
    return;

} // END ADD,SUB,.....

// -------------------------------------------------------------------------------- 
//          Parser ProcessInstruction
// --------------------------------------------------------------------------------

void ParseInstruction() {

    int i = 0;
    lineERR = FALSE;
    VarType = V_VALUE;

    strcpy(opCode, tokenSave);
    StrToUpper(opCode);
    
    if (DBG_PARSER) {

        printf("%03d I %s ", lineNr, opCode);
    }

    int num_Opcode = (sizeof(opCodeTab) / sizeof(opCodeTab[0]));
    bool opCode_found = FALSE;

    for (i = 0; i < num_Opcode; i++) {

        if (strcmp(opCode, opCodeTab[i].mnemonic) == 0) {		// opCode found

            opCode_found = TRUE;
            break;
        }
    }
    if (opCode_found == FALSE) {

        snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s ", opCode);
        ProcessError(errmsg);
        SkipToEOL();
        return; 
    }

    binInstr = opCodeTab[i].binInstr;
    OpType = opCodeTab[i].instrType;

    // build AST NODE INSTRUCTION

    operandType = 0;
    ASTinstruction = Create_ASTnode(NODE_INSTRUCTION, "", binInstr);
    Add_ASTchild(ASTprogram, ASTinstruction);

    if (strcmp(func_entry, "") != 0) {

        // build AST NODE LABEL

        ASTlabel = Create_ASTnode(NODE_LABEL, func_entry, 0);
        Add_ASTchild(ASTinstruction, ASTlabel);
        strcpy(func_entry, "");
    }
    if (strcmp(label, "") != 0) {

        symFound = FALSE;
        searchScopeLevel = currentScopeLevel;
        SearchSymLevel(scopeTab[searchScopeLevel], label, 0);

        if (symFound == FALSE) {

            // insert Label in SYMBOLTABLE

            strcpy(dirCode, "LABEL");
            operandType = OT_LABEL;
            VarType = V_VALUE;
            directive = Create_SYMnode(SCOPE_DIRECT, label, dirCode, "", lineNr);
            Add_SYMchild(scopeTab[currentScopeLevel], directive);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Label %s already defined ", label);
            ProcessError(errmsg);
            strcpy(label, "");
            SkipToEOL();
            return;
        }

        ASTlabel = Create_ASTnode(NODE_LABEL, label, 0);
        Add_ASTchild(ASTinstruction, ASTlabel);
        strcpy(label, "");
    }

    // build AST NODE OPERATION

    operandType = 0;
    ASToperation = Create_ASTnode(NODE_OPERATION, opCode, OpType);
    Add_ASTchild(ASTinstruction, ASToperation);

    switch (OpType) {

    case ADD:
    case ADC:
    case SUB:
    case SBC:
    case AND:
    case OR:
    case XOR:
    case CMP:
    case CMPU:      

        ParseModInstr(); 
        break;
    
    case ADDIL:
    case LDIL:

        ParseADDIL_LDIL();
        break;

    case B:

        ParseB();
        break;

    case GATE:

        ParseGATE();
        break;

    case BR:
    case BV:

        ParseBR_BV();
        break;

    case CBR:
    case CBRU:

        ParseCBR_CBRU();
        break;

    case BVE:

        ParseBVE();
        break;

    case EXTR:

        ParseEXTR();
        break;

    case DEP:

        ParseDEP();
        break;

    case LDR:
    case STC:

        ParseLDR_STC();
        break;

    case BE:

        ParseBE();
        break;

    case BRK:

        ParseBRK();
        break;

    case DSR:

        ParseDSR();
        break;

    case SHLA:

        ParseSHLA();
        break;

    case PCA:
    case PTLB:

        ParsePCA_PTLB();
        break;

    case CMR:

        ParseCMR();
        break;

    case DIAG:

        ParseDIAG();
        break;

    case ITLB:

        ParseITLB();
        break;

    case LDO:

        ParseLDO();
        break;

    case LSID:

        ParseLSID();
        break;

    case PRB:

        ParsePRB();
        break;

    case LDPA:

        ParseLDPA();
        break;

    case MR:

        ParseMR();
        break;

    case MST:

        ParseMST();
        break;

    case RFI:

        break;

    case LD:
    case ST:

        ParseLD_ST();
        break;

    case LDA:
    case STA:

        ParseLDA_STA();
        break;

    default: 
        printf("not yet implemented\n");
        break;

    }
    if (lineERR) {
        SkipToEOL();
        return;
    }

    codeAdr = codeAdr + 4;

    SkipToEOL();
    return;
}

/// @par Parser ProcessDirective

void ParseDirective() {
    // printf("DIRECTIVE %s\t", token_old);
         // Opcode

    int i = 0;
    int align;

    const char* ptr = O_DATA;

    int buf_size;
    int buf_init;

    VarType = V_VALUE;

    strcpy(dirCode, tokenSave);

    StrToUpper(dirCode);

    int num_dircode = (sizeof(dirCodeTab) / sizeof(dirCodeTab[0]));
    bool dirCode_found = FALSE;

    for (i = 0; i < num_dircode; i++) {

        if (strcmp(dirCode, dirCodeTab[i].directive) == 0) {		// opCode found

            dirCode_found = TRUE;
            DirType = dirCodeTab[i].directNum;
            break;
        }
    }

    if (dirCode_found == FALSE) {

        snprintf(errmsg, sizeof(errmsg), "Invalid directive %s", token);
        ProcessError(errmsg);
        SkipToEOL();
        return;

        while (TRUE) {

            GetNextToken();

            if (tokTyp == T_EOL) {
                return;
            }
        }
    }
    if (dirCode_found == TRUE) {

        switch (DirType) {

        case D_GLOBAL:

            if (prgType == P_UNDEFINED) {
                prgType = P_STANDALONE;
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                ProcessError(errmsg);
                SkipToEOL();
                return;
            }

            break;

        case D_MODULE:

            if (prgType == P_UNDEFINED) {
                prgType = P_MODULE;
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Standalone program", token);
                ProcessError(errmsg);
                SkipToEOL();
                return;
            }
            break;


        case D_CODE:

            if (prgType == P_STANDALONE) {
                GetNextToken();
                StrToUpper(token);
                do
                {
                    if (strcmp(token, "ADDR") == 0) {
                        GetNextToken();
                        O_CODE_ADDR = ParseExpression();
                        StrToUpper(token);
                    }
                    else if (strcmp(token, "ALIGN") == 0) {
                        
                        GetNextToken();
                        O_CODE_ALIGN = ParseExpression();
                        StrToUpper(token);
                        if (O_CODE_ALIGN == 0) {
                            snprintf(errmsg, sizeof(errmsg), "Alignment of 0 not allowed");
                            ProcessError(errmsg);
                            SkipToEOL();
                            return;
                        }
                        O_DATA_ALIGN = O_CODE_ALIGN;        // default data aligment follows code alignment
                        if ((O_CODE_ADDR % O_CODE_ALIGN) != 0) {
                            snprintf(errmsg, sizeof(errmsg), "Address %x not aligned by %x", O_CODE_ADDR, O_CODE_ALIGN);
                            ProcessError(errmsg);
                            SkipToEOL();
                            return;
                        }
                    }
                    else if (strcmp(token, "ENTRY") == 0) {
                        if (O_ENTRY_SET == FALSE) {
                            O_ENTRY_SET = TRUE;
                        }
                        else {
                            snprintf(errmsg, sizeof(errmsg), "Entry point already set");
                            ProcessError(errmsg);
                            SkipToEOL();
                            return;
                        }
                        GetNextToken();
                    }

                    else {
                        snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                        ProcessError(errmsg);
                        SkipToEOL();
                        return;
                    }
                    if (tokTyp == T_COMMA) {
                        GetNextToken();
                        StrToUpper(token);
                    }
                } while (tokTyp != T_EOL);

                if (O_ENTRY_SET == TRUE) {
                    O_ENTRY = O_CODE_ADDR;
                }
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                ProcessError(errmsg);
                SkipToEOL();
                return;
            }
            break;


        case D_DATA:

            if (prgType == P_STANDALONE) {
                GetNextToken();
                StrToUpper(token);
                do
                {
                    if (strcmp(token, "ADDR") == 0) {
                        GetNextToken();
                        O_DATA_ADDR = ParseExpression();
                        StrToUpper(token);
                    }
                    else if (strcmp(token, "ALIGN") == 0) {

                        GetNextToken();
                        O_DATA_ALIGN = ParseExpression();
                        StrToUpper(token);
                        if (O_DATA_ALIGN == 0) {
                            snprintf(errmsg, sizeof(errmsg), "Alignment of 0 not allowed");
                            ProcessError(errmsg);
                            SkipToEOL();
                            return;
                        }
                        if ((O_DATA_ADDR % O_DATA_ALIGN) != 0) {
                            snprintf(errmsg, sizeof(errmsg), "Address %x not aligned by %x", O_DATA_ADDR, O_DATA_ALIGN);
                            ProcessError(errmsg);
                            SkipToEOL();
                            return;
                        }
                    }
                    else {
                        snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                        ProcessError(errmsg);
                        SkipToEOL();
                        return;
                    }
                    if (tokTyp == T_COMMA) {
                        GetNextToken();
                        StrToUpper(token);
                    }
                } while (tokTyp != T_EOL);
            }
            else {
                snprintf(errmsg, sizeof(errmsg), "invalid %s in Module program", token);
                ProcessError(errmsg);
                SkipToEOL();
                return;
            }
            break;

        case D_ALIGN:

            GetNextToken();
            align = atoi(token);
            GetNextToken();
            StrToUpper(token);
            if (strcmp(token,"K") == 0) {
                align = align * 1024;
            }
            dataAdr = ((dataAdr / align) * align) + align;
            if ((align % 2) != 0) {

            }

            break;

        case D_EQU:
        case D_REG:

            GetNextToken();
            if (CheckReservedWord()) {

                AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Symbol %s is a reserved word", label);
                ProcessError(errmsg);
                SkipToEOL();
                return;
            }
            break;


        case D_BUFFER:
            GetNextToken();
            StrToUpper(token);
            do
            {
                if (strcmp(token, "SIZE") == 0) {
                    GetNextToken();
                    buf_size = ParseExpression();
                    StrToUpper(token);
                }
                else if (strcmp(token, "INIT") == 0) {

                    GetNextToken();
                    buf_init = ParseExpression();
                    StrToUpper(token);
                }
                else {
                    snprintf(errmsg, sizeof(errmsg), "Invalid Parameter %s", token);
                    ProcessError(errmsg);
                    SkipToEOL();
                    return;
                }
                if (tokTyp == T_COMMA) {
                    GetNextToken();
                    StrToUpper(token);
                }
            } while (tokTyp != T_EOL);

            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);



            // allocate memory area for data
            p_data = (char*)malloc(buf_size);
            O_dataLen = 0;

            /// init data buffer
            
            do
            {
                p_data[0 + O_dataLen] = buf_init & 0xFF;
                O_dataLen++;
                buf_size--;
            } while (buf_size > 0);
            
            memcpy(O_DATA, p_data, O_dataLen);

            addDataSectionData();

            break;

        case D_END:
            break;

        case D_BYTE:

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                VarType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                VarType = V_MEMLOCAL;
            }
            GetNextToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                GetNextToken();
            }

            // expression calculation 
            value = ParseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);


            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format
            O_DATA[0] = value & 0xFF;
            O_dataLen = 1;

            addDataSectionData();

            // adjust data address
            if ((dataAdr % 1) == 0) {
                dataAdr = (dataAdr + 1);
            }
            break;


        case D_HALF:

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                VarType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                VarType = V_MEMLOCAL;
            }
            GetNextToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                GetNextToken();
            }

            // expression calculation 
            value = ParseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on half word boundary
            if ((dataAdr % 2) != 0) {
                dataAdr = ((dataAdr / 2) * 2) + 2;
            }

            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format
            O_DATA[0] = (value >> 8) & 0xFF;
            O_DATA[1] = value & 0xFF;
            O_dataLen = 2;

            addDataSectionData();

            // adjust data address
            if ((dataAdr % 2) == 0) {
                dataAdr = (dataAdr + 2);
            }
            break;

        case D_WORD:

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                VarType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                VarType = V_MEMLOCAL;
            }
            GetNextToken();

            // Check if negative value
    
            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                GetNextToken();
            }
    
            // expression calculation 
            value = ParseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on word boundary
            if ((dataAdr % 4) != 0) {
                dataAdr = ((dataAdr / 4) * 4) + 4;
            }

            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format

            O_DATA[0] = (value >> 24) & 0xFF;
            O_DATA[1] = (value >> 16) & 0xFF;
            O_DATA[2] = (value >> 8) & 0xFF;
            O_DATA[3] = value & 0xFF;
            O_dataLen = 4;

            addDataSectionData();

            // adjust data address
            if ((dataAdr % 4) == 0) {
                dataAdr = (dataAdr + 4);
            }
            break;

        case D_DOUBLE:

            if ((currentScopeLevel == SCOPE_MODULE) || (currentScopeLevel == SCOPE_PROGRAM)) {
                VarType = V_MEMGLOBAL;
            }
            if (currentScopeLevel == SCOPE_FUNCTION) {
                VarType = V_MEMLOCAL;
            }
            GetNextToken();

            // Check if negative value

            is_negative = FALSE;
            if (tokTyp == T_MINUS) {

                is_negative = TRUE;
                GetNextToken();
            }

            // expression calculation 
            value = ParseExpression();
            strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);

            // Align on word boundary
            if ((dataAdr % 8) != 0) {
                dataAdr = ((dataAdr / 8) * 8) + 8;
            }

            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

            // Write in DATA SECTION of ELF File in BIG Endian format

            O_DATA[0] = (value >> 56) & 0xFF;
            O_DATA[1] = (value >> 48) & 0xFF;
            O_DATA[2] = (value >> 40) & 0xFF;
            O_DATA[3] = (value >> 32) & 0xFF;
            O_DATA[4] = (value >> 24) & 0xFF;
            O_DATA[5] = (value >> 16) & 0xFF;
            O_DATA[6] = (value >> 8) & 0xFF;
            O_DATA[7] = value & 0xFF;
            O_dataLen = 8;

            addDataSectionData();

            // adjust data address
            if ((dataAdr % 8) == 0) {
                dataAdr = (dataAdr + 8);
            }
            break;

        case D_STRING:
            
            GetNextToken();

            // Write in SYMTAB
            AddDirective(SCOPE_DIRECT, label, dirCode, token, lineNr);

            strcpy(O_DATA,token) ;
            O_dataLen = strlen(token) + 1;
            
            addDataSectionData();

            break;

        } // end switch
        strcpy(label, "");
        VarType = V_VALUE;

    } // end directive found
}