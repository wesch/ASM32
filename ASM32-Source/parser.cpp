
#include "constants.hpp"
#include "ASM32.hpp"



// Function to create a new Sym_node

SymNode* create_node(ScopeType type, char* label, char* func, const char* value, int linenr) {

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

void add_child(SymNode* parent, SymNode* child) {

    parent->children = (SymNode**)realloc(parent->children, sizeof(SymNode*) * (parent->child_count + 1));
    parent->children[parent->child_count++] = child;
}


// Add new Scope

void addScope(ScopeType type, char* label, char* func, const char* value, int linenr) {
    currentScopeLevel++;
    if (currentScopeLevel > maxScopeLevel) {
        printf("Too many Scope Levels\n");
        return;
    }
    strcpy(currentScopeName, label);
    strcpy(scopeNameTab[currentScopeLevel], currentScopeName);
    scopeTab[currentScopeLevel] = create_node(type, label, func, value, linenr);
    add_child(scopeTab[currentScopeLevel - 1], scopeTab[currentScopeLevel]);
}

// Add directive and link to scope

void addDirective(ScopeType type, char* label, char* func, const char* value, int linenr) {

    directive = create_node(type, label, func, value, linenr);
    add_child(scopeTab[currentScopeLevel], directive);

}

// Function search from actual Scopelevel up
void searchSymAll(SymNode* node, char* label, int depth) {

    if (!node) return;
    if (strcmp(node->label, label) == NULL &&
        node->scopeLevel <= currentScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == NULL) {
        //       printf("Val %s Label %s Line %d\n",node->value,label, node->linenr);
        strcpy(symFunc, node->func);
        strcpy(symValue, node->value);
        symFound = TRUE;
        return;
    }
    else {
        for (int i = 0; i < node->child_count; i++) {
            searchSymAll(node->children[i], label, depth + 1);
        }
    }

}

// Function search on actual Scopelevel

void searchSymLevel(SymNode* node, char* label, int depth) {

    if (!node) return;
    if (strcmp(node->label, label) == NULL &&
        node->scopeLevel == currentScopeLevel &&
        strcmp(node->scopeName, currentScopeName) == NULL) {
        
        symFound = TRUE;
        return;
    }
    else {
        for (int i = 0; i < node->child_count; i++) {
            searchSymLevel(node->children[i], label, depth + 1);
        }
    }
}

// Funktion Symbol Suche in Symboltabelle

bool searchSymbol(SymNode* node, char* label) {

    strcpy(currentScopeName, scopeNameTab[currentScopeLevel]);
    symFound = FALSE;
    scopeLevelSave = currentScopeLevel;
    while (symFound == FALSE) {
        searchSymAll(scopeTab[currentScopeLevel], label, 0);
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
void print_sym(SymNode* node, int depth) {

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
        print_sym(node->children[i], depth + 1);
    }
}


// --------------------------------------------------------------------------------
// Check reserved word
//  checks if token is a reserved word
// --------------------------------------------------------------------------------

bool CheckReservedWord() {
    return TRUE;
}



// --------------------------------------------------------------------------------
// Check Register
//  checks if global token contains a valid general register
// --------------------------------------------------------------------------------

bool CheckGenReg() {

    int reg = 0;                            // register number from checkreg = 0;
   
    if (toupper(token[0]) == 'R') {

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
        if (reg > 15) {
            return FALSE;
        }
    }
    else {
        if (searchSymbol(scopeTab[currentScopeLevel], token)) {
            if (strcmp(symFunc, "REG") == NULL) {

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

void GetValue() {

    if (tokTyp == NUM) {

        strcpy(token_old, token);
        tokTyp_old = tokTyp;

        NextToken();                                         // COMMA or EOL

        if (tokTyp == EOL) {                                 // must be immediate

            strcpy(token, token_old);
            tokTyp = tokTyp_old;
            if (tokTyp == NUM)
                tokTyp = NONE;
            return;
        }
    }
}

// --------------------------------------------------------------------------------
//      GetExpression
//          the actual token contains first token of the expression
//          the next token is an oparation +-*/
//          expression ends by EOL
///              or by ( if the token before ( is no operation 
//          expression contains NUM, IDENTIFIER, LPAREN, RPAREN, PLUS, MINUS, MUL, DIV
// 
// --------------------------------------------------------------------------------
void GetExpression() {


    printf("%s ", token_old);
    // PrintSymbolTokenCode(tokTyp);
    printf(" ");

    while (tokTyp != EOL) {

        tokTyp_old = tokTyp;
        strcpy(token_old, token);
        NextToken();
        if (tokTyp == LPAREN || tokTyp == EOL) {
            


            if (tokTyp_old == RPAREN || tokTyp_old == NUM || tokTyp_old == IDENTIFIER) {

                break;
            }
        }

        if (tokTyp == IDENTIFIER) {

            if (searchSymbol(scopeTab[currentScopeLevel], token)) {

                if (strcmp(symFunc, "EQU") == NULL) {
                    printf("%s\t", symValue);
                }
            }
        }
        else if (tokTyp == NUM) {
            printf("%s ", token);
        }
        
        else {

            if (tokTyp == PLUS || 
                tokTyp == MINUS || 
                tokTyp == MUL || 
                tokTyp == DIV ||
                tokTyp == LPAREN ||
                tokTyp == RPAREN) {
                
                PrintSymbolTokenCode(tokTyp); 
            }
            printf(" ");
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

// --------------------------------------------------------------------------------
//      PrintTokenCode (code) --> string
//          This function prints the given code in human readable form.
// --------------------------------------------------------------------------------

void PrintTokenCode(int i) {

    switch (i) {

    case NONE:          printf("NONE"); break;
    case IDENTIFIER:    printf("IDENTIFIER"); break;
    case NUM:           printf("NUM"); break;
    case COMMA:         printf("COMMA"); break;
    case COLON:         printf("COLON"); break;
    case DOT:           printf("DOT"); break;
    case LPAREN:        printf("LPAREN"); break;
    case RPAREN:        printf("RPAREN"); break;
    case COMMENT:       printf("COMMENT"); break;
    case DIRECTIVE:     printf("DIRECTIVE"); break;
    case OPCODE:        printf("OPCODE"); break;
    case LABEL:         printf("LABEL"); break;
    case MINUS:         printf("MINUS"); break;
    case PLUS:          printf("PLUS"); break;
    case MUL:           printf("MUL"); break;
    case DIV:           printf("DIV"); break;
    case EOL:		    printf("EOL"); break;
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

    case LPAREN:        printf("("); break;
    case RPAREN:        printf(")"); break;
    case MINUS:         printf("-"); break;
    case PLUS:          printf("+"); break;
    case MUL:           printf("*"); break;
    case DIV:           printf("/"); break;
    case EOL:		    printf("EOL"); break;
    case EOF:		    printf("EOF"); break;
    case IDENTIFIER:    printf("ID"); break;

    default:		    printf("-----  unknown symbol  -----");
    }
}

// -------------------------------------------------------------------------------- 
//          Parser ProcessLabel
// --------------------------------------------------------------------------------

void ProcessLabel() {
//    printf("LABEL %s\t", token_old);
    strcpy(label, token_old);
}

// -------------------------------------------------------------------------------- 
//          Parser ProcessInstruction
// --------------------------------------------------------------------------------

void ProcessInstruction() {
    printf("%d I %s\t", lineNr, token_old);

    int i = 0;
    char        option[MAX_WORD_LENGTH];        // option value 
    char        opCode[MAX_WORD_LENGTH];        // Opcode
    int         OpType;                         // contains opCodetab.instr_type



    strcpy(opCode, token_old);

    StrToUpper(opCode);

    int num_Opcode = (sizeof(opCodeTab) / sizeof(opCodeTab[0]));
    bool opCode_found = FALSE;

    for (i = 0; i < num_Opcode; i++) {
    
        if (strcmp(opCode, opCodeTab[i].mnemonic) == 0) {		// opCode found
        
            opCode_found = TRUE;
            break;
        }
    }
    if (opCode_found == FALSE) {

        printf("E: Line %d Column %d \tInvalid Opcode %s", lineNr, column, opCode);

        while (TRUE) {

            NextToken();

            if (tokTyp == EOL) {
                return;
            }
        }
    }

    OpType = opCodeTab[i].instrType;

// --------------------------------------------------------------------------------
//  Opcode ADD,ADC,SUB,SBC,AND,OR,XOR,CMP,CMPU
//  Format:
//      OP<.XX> regR,value
//      OP<.XX> regR, regA, regB
//      OP[W | H | B]<.XX> regR, regA(regB)
//      OP[W | H | B]<.XX> regR, ofs(regB)
// 
// --------------------------------------------------------------------------------

    if (OpType == ADD ||
        OpType == ADC ||
        OpType == SUB ||
        OpType == SBC ||
        OpType == AND ||
        OpType == OR ||
        OpType == XOR ||
        OpType == CMP ||
        OpType == CMPU) {

        if (tokTyp == DOT) {                                   // check for option

            memset(option, 0, MAX_WORD_LENGTH);
            NextToken();

            if (tokTyp == IDENTIFIER) {

                printf("OPT %s\t", token);
            }
            NextToken();
        }

        // Here is always regR 

        if (tokTyp == IDENTIFIER) {   

            if (CheckGenReg() == TRUE) {

                printf("regR %s\t", token);
            }
            else
            {
                sprintf(errmsg, "Invalid register name %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }

        }
        else
        {
            sprintf(errmsg, "Unexpected token %s at line %d position %d", token, lineNr, column);
            ProcessError(errmsg);
        }

        // Here is always a comma

        NextToken();             
        if (tokTyp != COMMA) {
            sprintf(errmsg, "Unexpected token %s at line %d position %d", token, lineNr, column);
            ProcessError(errmsg);
        }

        NextToken();                                             
        
        // Here is optional a minus sign

        if (tokTyp == MINUS) {
            PrintSymbolTokenCode(tokTyp);
            is_negative = TRUE;
            NextToken();
        }

        // Here is either register or a value (num, symbol, expression)

        mode = 9;                                           // Mode not set

        if (tokTyp == IDENTIFIER) {

            if (CheckGenReg() == TRUE) {

                printf("regA %s\t", token);
                mode = 1;
            }
            else if (searchSymbol(scopeTab[currentScopeLevel], token)) {

                if (strcmp(symFunc, "EQU") == NULL) {
                    printf("%s\t", symValue);
                    strcpy(token, symValue);
                }
            }
            else
            {
                sprintf(errmsg, "Invalid symbol name %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }

        }
        else if (tokTyp == NUM) {

        }
        else if (tokTyp == LPAREN) {
            if (tokTyp == LPAREN) {
                PrintSymbolTokenCode(tokTyp);
            }
        }
        strcpy(token_old, token);
        tokTyp_old = tokTyp;
        
        if (mode != 1) {
         
            GetExpression();
        }
        else {
            NextToken();
        }
        if (tokTyp == EOL) {

            strcpy(token, token_old);
            tokTyp = tokTyp_old;
        //    printf("sym %s\t", token);
            printf("M-0\n");
            return;
        }

        // If mode = 1 we had a register which may be mode 1 or mode 2 else we have mode 3

        if (mode == 1) {

            
            if (tokTyp == COMMA) {
                mode = 1;
            }
            else if (tokTyp == LPAREN) {
                mode = 2;
            }
            else {
                sprintf(errmsg, "Invalid token at line %d position %d", lineNr, column);
                ProcessError(errmsg);
            }
            NextToken();
            if (CheckGenReg() == TRUE) {
                printf("regB %s\t", token);
                printf("M-%d\n", mode);
            }
            else
            {
                sprintf(errmsg, "Unexpected token %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }
            

        }
        else
        {

        }

        // == ABSPRUNG

        return;

        // =======================================================



        if (tokTyp == NUM ) {

            strcpy(token_old, token);
            tokTyp_old = tokTyp;

            NextToken();                                         // COMMA or EOL

            if (tokTyp == EOL) {                                 // must be immediate

                strcpy(token, token_old);
                tokTyp = tokTyp_old;
                if (tokTyp == NUM)
                    tokTyp = NONE;
                printf("%s\t", token);
                printf("M-0\n");
                return;
            }
            else if (tokTyp == LPAREN) {

                strcpy(token, token_old);
                tokTyp = tokTyp_old;

                if (tokTyp == NUM) {

                    printf("%s\t", token);
                    NextToken();
                    printf("regb %s\t", token);
                    printf("M-3\n");
                    NextToken();                                    // RPAREN
                    return;
                }

            }
            else if (tokTyp == PLUS || tokTyp == MINUS || tokTyp == MUL || tokTyp == DIV ) {

                GetExpression();

                if (tokTyp == LPAREN) {

                    NextToken();

                    if (CheckGenReg() == TRUE) {

                        printf("regb %s\t", token);
                        printf("M-3\n");
                        NextToken();                                    // RPAREN
                        return;
                    }
                    else
                    {
                        sprintf(errmsg, "Invalid register name %s at line %d position %d", token, lineNr, column);
                        ProcessError(errmsg);
                    }
                    
 
                }
                else {
                    printf("M-0\n");
                    return;
                }
            }

        }
        else if (tokTyp == IDENTIFIER) {

            if (CheckGenReg() == TRUE) {

                printf("regA %s\t", token);

               
            }
            else if (searchSymbol(scopeTab[currentScopeLevel], token)) {

                if (strcmp(symFunc, "EQU") == NULL) {
                    printf("%s\t", symValue);
                }
            }

            else
            {
                sprintf(errmsg, "Invalid symbol name %s at line %d position %d", token, lineNr, column);
                ProcessError(errmsg);
            }

            strcpy(token_old, token);
            tokTyp_old = tokTyp;
            NextToken();
            
            if (tokTyp == EOL) {
                printf("M-0\n");
                return;
            }

            if (tokTyp == COMMA) {                                  // --> M1 two register

                NextToken();
                if (tokTyp == EOL) {
                    sprintf(errmsg, "Unexpected end of line  %d position %d",  lineNr, column);
                    ProcessError(errmsg);
                }
                if (CheckGenReg() == TRUE) {

                    printf("regB %s\t", token);
                    printf("M-1\n");
                    return;
                }
                else
                {
                    sprintf(errmsg, "Invalid register name %s at line %d position %d", token, lineNr, column);
                    ProcessError(errmsg);
                }

            }
            else if (tokTyp == LPAREN) {                            // --> M2 register indexed

                strcpy(token, token_old);
                tokTyp = tokTyp_old;
                NextToken();

                if (CheckGenReg() == TRUE) {

                    printf("regB %s\t", token);
                    printf("M-2\n");
                    NextToken();                                    // RPAREN
                    return;
                }
                else
                {
                    sprintf(errmsg, "Invalid register name %s at line %d position %d", token, lineNr, column);
                    ProcessError(errmsg);
                }
            }
            else if (tokTyp == EOL) {

                strcpy(token, token_old);
                tokTyp = tokTyp_old;
                printf("sym %s\t", token);
                printf("M-0\n");
                return;
            }
            else if (tokTyp == PLUS || tokTyp == MINUS || tokTyp == MUL || tokTyp == DIV) {

                GetExpression();

                if (tokTyp == LPAREN) {

                    NextToken();

                    if (CheckGenReg() == TRUE) {

                        printf("regb %s\t", token);
                        printf("M-3\n");
                        NextToken();                                    // RPAREN
                        return;
                    }
                    else
                    {
                        sprintf(errmsg, "Invalid register name %s at line %d position %d", token, lineNr, column);
                        ProcessError(errmsg);
                    }
                }
                else {
                    printf("M-0\t");
                    return;
                }
            }
        }
    } // END ADD,SUB,.....


}

// -------------------------------------------------------------------------------- 
//          Parser ProcessDirective
// --------------------------------------------------------------------------------

void ProcessDirective() {
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



        sprintf(errmsg, "Invalid directive %s", token);
        ProcessError(errmsg);

        while (TRUE) {

            NextToken();

            if (tokTyp == EOL) {
                return;
            }
        }
    }
    if (dirCode_found == TRUE) {

        if (strcmp(dirCode, "ENDMODULE") == NULL ||
            strcmp(dirCode, "ENDFUNCTION") == NULL ||
            strcmp(dirCode, "ENDBLOCK") == NULL) {
            if (currentScopeLevel > 1) {
                scopeTab[currentScopeLevel--] = NULL;
            }
            else
            {
                sprintf(errmsg, "Scopelevel can not be reduced below program level", token);
                ProcessError(errmsg);

            }
        }
        else
        {
            symFound = FALSE;
            searchSymLevel(scopeTab[currentScopeLevel], label, 0);

            if (symFound == FALSE) {
            //   printf("%s label not found\n", label);
                strcpy(symValue, "");

                if (strcmp(dirCode, "MODULE") == NULL) {
                    currentScopeType = MODULE;
                    addScope(currentScopeType, label, dirCode, symValue, lineNr);

                }
                else if (strcmp(dirCode, "FUNCTION") == NULL) {
                    currentScopeType = FUNCTION;
                    addScope(currentScopeType, label, dirCode, symValue, lineNr);
                }
                else if (strcmp(dirCode, "BLOCK") == NULL) {
                    currentScopeType = BLOCK;
                    addScope(currentScopeType, label, dirCode, symValue, lineNr);
                }
                if (strcmp(dirCode, "EQU") == NULL ||
                    strcmp(dirCode, "REG") == NULL) {
                    NextToken();
                    if (CheckReservedWord()) {
                        addDirective(DIRECT, label, dirCode, token, lineNr);
                    }
                    else
                    {
                        sprintf(errmsg, "Symbol %s is a reserved word", label);
                        ProcessError(errmsg);
                    }

                }



                strcpy(label, "");
 
            }
            else {

                sprintf(errmsg, "Symbol %s already defined in this scope", label);
                ProcessError(errmsg);
            }
        }
    }

}