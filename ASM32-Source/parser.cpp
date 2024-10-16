
#include "constants.hpp"
#include "ASM32.hpp"

// Function to create a new AST node
ASTNode* Create_ASTnode(NodeType type, const char* value, int valnum) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = _strdup(value);
    node->valnum = valnum;
    node->linenr = lineNr;
    node->column = column;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

// Function to add a child node
void Add_ASTchild(ASTNode* parent, ASTNode* child) {
    parent->children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*) * (parent->child_count + 1));
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

SymNode* Create_SYMnode(ScopeType type, char* label, char* func, const char* value, int linenr) {

    SymNode* node = (SymNode*)malloc(sizeof(SymNode));

    node->type = type;
    node->scopeLevel = currentScopeLevel;
    strcpy(node->scopeName, currentScopeName);

    strcpy(node->label, label);
    strcpy(node->func, func);
    strcpy(node->value, value);
    node->linenr = linenr;

    node->children = NULL;
    node->child_count = 0;
    return node;
}

// Function to add a child node

void Add_SYMchild(SymNode* parent, SymNode* child) {

    parent->children = (SymNode**)realloc(parent->children, sizeof(SymNode*) * (parent->child_count + 1));
    parent->children[parent->child_count++] = child;
}


// Add new Scope

void AddScope(ScopeType type, char* label, char* func, const char* value, int linenr) {
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

void AddDirective(ScopeType type, char* label, char* func, const char* value, int linenr) {

    directive = Create_SYMnode(type, label, func, value, linenr);
    Add_SYMchild(scopeTab[currentScopeLevel], directive);

}

// Function search from actual Scopelevel up
void SearchSymAll(SymNode* node, char* label, int depth) {

    if (!node) return;
    if (strcmp(node->label, label) == 0 &&
        node->scopeLevel <= currentScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        //       printf("Val %s Label %s Line %d\n",node->value,label, node->linenr);
        strcpy(symFunc, node->func);
        strcpy(symValue, node->value);
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
        node->scopeLevel == currentScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == 0) {
        
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

    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    symFound = FALSE;
    scopeLevelSave = currentScopeLevel;
    while (symFound == FALSE) {
        SearchSymAll(scopeTab[currentScopeLevel], label, 0);
        if (symFound == TRUE) {
            return TRUE;
        }
        currentScopeLevel--;
        strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
        if (currentScopeLevel == 0) break;
    }
    if (symFound == FALSE) {
        printf("Symbol %s not found\n", label);
    }
    currentScopeLevel = scopeLevelSave;
    if (symFound == TRUE) {
       printf("Symbol %s Value %s\n", symFunc, symValue);
    }
    return symFound;
}

// Function to print the Symtab
void PrintSYM(SymNode* node, int depth) {

    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    printf("%s:\t%d\t%s\t%s\t%s\tLine: %d\n", (node->type == GLOBAL) ? "G" :
        (node->type == PROGRAM) ? "P" :
        (node->type == MODULE) ? "M" :
        (node->type == FUNCTION) ? "F" :
        (node->type == BLOCK) ? "B" :
        (node->type == DIRECT) ? "D" :
        "Unknown", node->scopeLevel,node->label, node->func, node->value, node->linenr);

    for (int i = 0; i < node->child_count; i++) {
        PrintSYM(node->children[i], depth + 1);
    }
}


void SkipToEOL() {
    while (tokTyp != T_EOL) {
        NextToken();
    }

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
    return;
    int x = opCode[strlen(opCode) - 1];
    if (mode == 0 || mode == 1) {


        if (x == 'H' || x == 'W' || x == 'B') {
            snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s mode %d combination at line %d position %d", opCode,mode, lineNr, column);
            ProcessError(errmsg);
        }
    }
}

// --------------------------------------------------------------------------------
// Check Register
//  checks if global token contains a valid general register
// --------------------------------------------------------------------------------

bool CheckGenReg() {

    int reg = 0;                            // register number from checkreg = 0;
   
    StrToUpper(token);
    
    if (token[0] == 'R') {

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
        if (SearchSymbol(scopeTab[currentScopeLevel], token)) {
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
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }


}
int ParseFactor() {
    int n = 0;

    if (tokTyp == T_LPAREN) {

        NextToken(); 
        n = ParseExpression(); 

        if (tokTyp == T_RPAREN) {

            NextToken(); 
            return n; 
        }
    }
    else if (tokTyp == T_NEG) {

        n = ~n;
    }
    else {
        
        if (tokTyp == T_NUM) {
 
        }
        else if (tokTyp == T_IDENTIFIER) {

            if (SearchSymbol(scopeTab[currentScopeLevel], token)) {

                if (strcmp(symFunc, "EQU") == 0) {

                    strcpy(token, symValue);
                }
                else
                {
                    snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s at line %d position %d", token, lineNr, column);
                    ProcessError(errmsg);
                }
            }
            else
            {
                snprintf(errmsg, sizeof(errmsg), "Invalid symbol name %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }

        }
        n = atoi(token);
 
        NextToken();
    }
    return n; 
}

int ParseTerm() {
    int first, second;

    first = ParseFactor(); 

    for (;;) {
        if (tokTyp == T_MUL) {

            NextToken(); 
            second = ParseFactor(); 
            first *= second; 
        }
        else if (tokTyp == T_DIV) {

            NextToken(); 
            second = ParseFactor(); 
            first /= second; 
        }
        else if (tokTyp == T_MOD) {

            NextToken(); 
            second = ParseFactor(); 
            first %= second; 
        }
        else if (tokTyp == T_AND) {

            NextToken();
            second = ParseExpression();
            first &= second;
        }

        else {
            return first; 
        }
    }
}

int ParseExpression() {
    int first, second;

    first = ParseTerm(); 
    if (is_negative == TRUE) {
        first = -first;
    }

    for (;;) {
        if (tokTyp == T_PLUS) {

            NextToken(); 
            second = ParseTerm(); 
            first += second; 
        }
        else if (tokTyp == T_MINUS) {

            NextToken(); 
            second = ParseTerm(); 
            first -= second; 
        }
        else if (tokTyp == T_OR) {

            NextToken();
            second = ParseExpression();
            first |= second;
        }
        else if (tokTyp == T_XOR) {

            NextToken();
            second = ParseExpression();
            first ^= second;
        }
        else {
            return first; 
        }
    }
}






// --------------------------------------------------------------------------------
//      NextToken
//          get next token from tokenList
// --------------------------------------------------------------------------------

void NextToken() {
    ptr_t = ptr_t->next;
    strcpy(token, ptr_t->token);
    tokTyp = ptr_t->tokTyp;
    lineNr = ptr_t->lineNumber;
    column = ptr_t->column;
}

// Function to print the AST
void PrintAST(ASTNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("%s: %s %d %x\n", (node->type == NODE_PROGRAM) ? "Prog     " :
        (node->type == NODE_INSTRUCTION) ? "Instr   " :
        (node->type == NODE_DIRECTIVE) ?   "Direct" :
        (node->type == NODE_OPERATION) ?   "OpCode" :
        (node->type == NODE_OPERAND) ?     "Oper  " :
        (node->type == NODE_OPTION) ?      "Option" :
        (node->type == NODE_MODE) ?        "Mode  " :
        (node->type == NODE_LABEL) ?       "Label " :
        "Unknown", node->value, node->valnum, node->valnum);

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
    case T_IDENTIFIER:  printf("IDENTIFIER"); break;
    case T_NUM:         printf("NUM"); break;
    case T_COMMA:       printf("COMMA"); break;
    case T_COLON:       printf("COLON"); break;
    case T_DOT:         printf("DOT"); break;
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
    case T_EOL:		    printf("EOL"); break;
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
//    printf("LABEL %s\t", token_old);
    strcpy(label, token_old);
}

// --------------------------------------------------------------------------------
//  Opcode ADD,ADC,SUB,SBC,AND,OR,XOR,CMP,CMPU
//  Format:
//      OP<.XX> regR,value
//      OP<.XX> regR, regA, regB
//      OP[W | H | B]<.XX> regR, regA(regB)
//      OP[W | H | B]<.XX> regR, ofs(regB)
// 
// --------------------------------------------------------------------------------
void ParseModInstr() {

    char        option[MAX_WORD_LENGTH];        // option value 



    if (tokTyp == T_DOT) {                                   // check for 1st option

        memset(option, 0, MAX_WORD_LENGTH);
        NextToken();
        if (tokTyp == T_IDENTIFIER) {

            printf("OPT1 %s ", token);
            ASTopt1 = Create_ASTnode(NODE_OPTION, token,0);
            Add_ASTchild(ASTinstruction, ASTopt1);
        }
        NextToken();
    }
    if (tokTyp == T_DOT) {                                   // check for 2nd option

        memset(option, 0, MAX_WORD_LENGTH);
        NextToken();

        if (tokTyp == T_IDENTIFIER) {

            printf("OPT2 %s ", token);
            ASTopt2 = Create_ASTnode(NODE_OPTION, token, 0);
            Add_ASTchild(ASTinstruction, ASTopt2);
        }
        NextToken();
    }

    // Here is always regR 

    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            printf("regR %s ", token);
            ASTop1 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop1);
        }
        else
        {
            snprintf(errmsg, sizeof(errmsg), "Invalid register name %s at line %d position %d", token, lineNr, column);
            ProcessError(errmsg);
            return;
        }
    }
    else
    {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s at line %d position %d", token, lineNr, column);
        ProcessError(errmsg);
        return;
    }

    // Here is always a comma


    NextToken();
    if (tokTyp != T_COMMA) {
        snprintf(errmsg, sizeof(errmsg), "Unexpected token %s at line %d position %d", token, lineNr, column);
        ProcessError(errmsg);
    }

    // this is the common area for all modes

    NextToken();

    mode = 0;
    is_negative = FALSE;
    
    if (tokTyp == T_MINUS) {

        is_negative = TRUE;
        NextToken();
    }


    if (tokTyp == T_IDENTIFIER) {

        if (CheckGenReg() == TRUE) {

            printf("regA %s ", token);
            ASTop2 = Create_ASTnode(NODE_OPERAND, token, 0);
            Add_ASTchild(ASTinstruction, ASTop2);
            NextToken();
            if (tokTyp == T_EOL) {
                mode = 1;
                printf("M-%d\n", mode);

                CheckOpcodeMode();

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
           
        printf("[%d]", value);
        ASTop2 = Create_ASTnode(NODE_OPERAND, "", value);
        Add_ASTchild(ASTinstruction, ASTop2);
        if (tokTyp == T_EOL) {

            mode = 0;
            printf("M-%d\n", mode);

            CheckOpcodeMode();

            ASTmode = Create_ASTnode(NODE_MODE, "", mode);
            Add_ASTchild(ASTinstruction, ASTmode);
            return;
        }
        else if (tokTyp == T_LPAREN) {

            mode = 3;
        }
    }
    NextToken();

    if (CheckGenReg() == TRUE) {

        printf("regB %s ", token);
        ASTop3 = Create_ASTnode(NODE_OPERAND, token, 0);
        Add_ASTchild(ASTinstruction, ASTop3);

        if (mode == 1) {
            NextToken();
            if (tokTyp != T_EOL) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }
        }
        else if (mode == 2 || mode == 3) {
            NextToken();
            if (tokTyp != T_RPAREN) {
                snprintf(errmsg, sizeof(errmsg), "Unexpected token %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }
        }

        printf("M-%d\n", mode);

        CheckOpcodeMode();
        
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
    
    int         OpType;                         // contains opCodetab.instr_type

    strcpy(opCode, token_old);

    StrToUpper(opCode);

    printf("%03d I %s ", lineNr, opCode);

    int num_Opcode = (sizeof(opCodeTab) / sizeof(opCodeTab[0]));
    bool opCode_found = FALSE;

    for (i = 0; i < num_Opcode; i++) {

        if (strcmp(opCode, opCodeTab[i].mnemonic) == 0) {		// opCode found

            opCode_found = TRUE;
            break;
        }
    }
    if (opCode_found == FALSE) {

        snprintf(errmsg, sizeof(errmsg), "Invalid Opcode %s at line %d position %d", opCode, lineNr, column );
        ProcessError(errmsg);

        return; 
    }

    ASTinstruction = Create_ASTnode(NODE_INSTRUCTION, "", opCodeTab[i].binInstr);
    Add_ASTchild(ASTprogram, ASTinstruction);

    if (strcmp(label, "") != 0) {
        ASTlabel = Create_ASTnode(NODE_LABEL, label, 0);
        Add_ASTchild(ASTinstruction, ASTlabel);
        strcpy(label, "");
    }

    ASToperation = Create_ASTnode(NODE_OPERATION, opCode,0);
    Add_ASTchild(ASTinstruction, ASToperation);

    OpType = opCodeTab[i].instrType;

    switch (OpType) {

    case ADD:
    case ADC:
    case SUB:
    case SBC:
    case AND:
    case OR:
    case XOR:
    case CMP:
    case CMPU:      ParseModInstr(); break;

    default: printf("not yet implemented\n");

    }
    SkipToEOL();
    return;
}





// -------------------------------------------------------------------------------- 
//          Parser ProcessDirective
// --------------------------------------------------------------------------------

void ParseDirective() {
    // printf("DIRECTIVE %s\t", token_old);
    char        dirCode[MAX_WORD_LENGTH];        // Opcode
 
    int i = 0;


    strcpy(dirCode, token_old);

    StrToUpper(dirCode);

    int num_dircode = (sizeof(dirCodeTab) / sizeof(dirCodeTab[0]));
    bool dirCode_found = FALSE;

    for (i = 0; i < num_dircode; i++) {

        if (strcmp(dirCode, dirCodeTab[i].directive) == 0) {		// opCode found

            dirCode_found = TRUE;
 //           printf("DIRECTIVE %s is valid\n", dirCode);
            break;
        }
    }
   
    if (dirCode_found == FALSE) {



        snprintf(errmsg, sizeof(errmsg), "Invalid directive %s", token);
        ProcessError(errmsg);

        while (TRUE) {

            NextToken();

            if (tokTyp == T_EOL) {
                return;
            }
        }
    }
    if (dirCode_found == TRUE) {

        if (strcmp(dirCode, "ENDMODULE") == 0 ||
            strcmp(dirCode, "ENDFUNCTION") == 0 ||
            strcmp(dirCode, "ENDBLOCK") == 0) {
            if (currentScopeLevel > 1) {
                scopeTab[currentScopeLevel--] = NULL;
            }
            else
            {
                strcpy(errmsg, "Scopelevel can not be reduced below program level");
                ProcessError(errmsg);

            }
        }
        else
        {
            symFound = FALSE;
            SearchSymLevel(scopeTab[currentScopeLevel], label, 0);

            if (symFound == FALSE) {
            //   printf("%s label not found\n", label);
                strcpy(symValue, "");

                if (strcmp(dirCode, "MODULE") == 0) {
                    currentScopeType = MODULE;
                    AddScope(currentScopeType, label, dirCode, symValue, lineNr);

                }
                else if (strcmp(dirCode, "FUNCTION") == 0) {
                    currentScopeType = FUNCTION;
                    AddScope(currentScopeType, label, dirCode, symValue, lineNr);
                }
                else if (strcmp(dirCode, "BLOCK") == 0) {
                    currentScopeType = BLOCK;
                    AddScope(currentScopeType, label, dirCode, symValue, lineNr);
                }
                if (strcmp(dirCode, "EQU") == 0 ||
                    strcmp(dirCode, "REG") == 0) {
                    NextToken();
                    if (CheckReservedWord()) {
                        AddDirective(DIRECT, label, dirCode, token, lineNr);
                    }
                    else
                    {
                        snprintf(errmsg, sizeof(errmsg), "Symbol %s is a reserved word", label);
                        ProcessError(errmsg);
                    }

                }



                strcpy(label, "");
 
            }
            else {

                snprintf(errmsg, sizeof(errmsg), "Symbol %s already defined in this scope", label);
                ProcessError(errmsg);
            }
        }
    }

}