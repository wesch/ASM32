
#include "constants.hpp"
#include "ASM32.hpp"

// --------------------------------------------------------------------------------
//      new tokenList entry    
// --------------------------------------------------------------------------------

void NewTokenList() {

    if (start_t == NULL) {                                            // insert 1. element

        start_t = (struct tokenList*)malloc(sizeof(struct tokenList));

        if (start_t == NULL) {

            printf("Memory allocation failed for Token List\n");
            return;
        }
        start_t->next = NULL;
        ptr_t = start_t;
    }
    else {                                                          // at least 1 elemnt exists

        ptr_t = start_t;

        while (ptr_t->next != NULL) {

            ptr_t = ptr_t->next;
        }
        ptr_t->next = (struct tokenList*)malloc(sizeof(struct tokenList));

        if (ptr_t->next == NULL) {

            printf("Memory allocation failed for Token List\n");
            return;
        }
        ptr_t = ptr_t->next;
        ptr_t->next = NULL;
    }
}

// --------------------------------------------------------------------------------
//      print tokenList entries
// --------------------------------------------------------------------------------

void PrintTokenList() {

    // return;

    struct tokenList* ptr_t = start_t;

    printf("\n\Token List\n");
    printf("----------------------------\n");

    while (ptr_t != NULL) {

        printf("%d\t%d\t%s\t", ptr_t->lineNumber, ptr_t->column, ptr_t->token);
        PrintTokenCode(ptr_t->tokTyp);
        printf("\n");
        ptr_t = ptr_t->next;
    }
}

// --------------------------------------------------------------------------------
//      Get Token
// --------------------------------------------------------------------------------

void GetToken() {
    int         ch;                             // character for tokenizer
    strcpy(token,"\0");
    j = 0;
    tokTyp = NONE;

    while (TRUE) {

        ch = sl[ind];
        column = ind;

        if (ch == '\n') {
            tokTyp = T_EOL;
            strcpy(token, "EOL");
            break;
        }
        else if (ch == ';') {
            tokTyp = T_EOL;
            strcpy(token, "EOL");
            break;
        }
        else if (ch == '.') {
            tokTyp = T_DOT;
            strcpy(token, ".");
            break;
        }
        else if (ch == ',') {
            tokTyp = T_COMMA;
            strcpy(token, ",");
            break;
        }
        else if (ch == ':') {
            tokTyp = T_COLON;
            strcpy(token, ":");
            break;
        }
        else if (ch == '-') {
            tokTyp = T_MINUS;
            strcpy(token, "-");
            break;
        }
        else if (ch == '+') {
            tokTyp = T_PLUS;
            strcpy(token, "+");
            break;
        }
        else if (ch == '*') {
            tokTyp = T_MUL;
            strcpy(token, "*");
            break;
        }
        else if (ch == '/') {
            tokTyp = T_DIV;
            strcpy(token, "/");
            break;
        }
        else if (ch == '~') {
            tokTyp = T_NEG;
            strcpy(token, "~");
            break;
        }
        else if (ch == '%') {
            tokTyp = T_MOD;
            strcpy(token, "%");
            break;
        }
        else if (ch == '|') {
            tokTyp = T_OR;
            strcpy(token, "|");
            break;
        }
        else if (ch == '&') {
            tokTyp = T_AND;
            strcpy(token, "%");
            break;
        }
        else if (ch == '^') {
            tokTyp = T_XOR;
            strcpy(token, "^");
            break;
        }
        else if (ch == '(') {
            tokTyp = T_LPAREN;
            strcpy(token, "(");
            break;
        }
        else if (ch == ')') {
            tokTyp = T_RPAREN;
            strcpy(token, ")");
            break;
        }
        else if (isalpha(ch)) {                         // alphanumeric 

            column = ind;
            tokTyp = T_IDENTIFIER;
            while (isalpha(ch) || isdigit(ch)) {

                token[j] = ch;
                j++;
                ind++;
                ch = sl[ind];
            }
            token[j] = '\0';
            ind--;
            break;
        }
        else if (isdigit(ch)) {                         // numeric value

            column = ind;
            tokTyp = T_NUM;
            if (ch == '0' && sl[ind + 1] == 'x') {
                token[j] = ch;
                token[j + 1] = sl[ind + 1];
                j = 2;
                ind = ind + 2;
                ch = sl[ind];
                while (HexCharToInt(ch) >= 0) {
                    token[j] = ch;
                    j++;
                    ind++;
                    ch = sl[ind];
                }
            }
            else {
                while (isdigit(ch)) {

                    token[j] = ch;
                    j++;
                    ind++;
                    ch = sl[ind];
                }
            }
            token[j] = '\0';
            ind--;
            numToken = (int)strtol(token, NULL, 0);

            break;
        }
        ind++;
    }
    ind++;
}