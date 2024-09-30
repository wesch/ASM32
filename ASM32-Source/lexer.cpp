
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

    struct tokenList* ptr_t = start_t;
    printf("===== Token List ======\n");

    while (ptr_t != NULL) {

        printf("%d\t%d\t%s\t", (size_t*)ptr_t->lineNumber, ptr_t->column, ptr_t->token);
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
    token[0] = '\0';
    j = 0;
    tokTyp = NONE;

    while (TRUE) {

        ch = sl[ind];
        column = ind;

        if (ch == '\n') {

            tokTyp = EOL;
            break;
        }
        else if (ch == ';') {

            tokTyp = EOL;
            break;
        }
        else if (ch == '.') {

            tokTyp = DOT;
            break;
        }
        else if (ch == ',') {

            tokTyp = COMMA;
            break;
        }
        else if (ch == ':') {

            tokTyp = COLON;
            break;
        }
        else if (ch == '-') {

            tokTyp = MINUS;
            break;
        }
        else if (ch == '+') {

            tokTyp = PLUS;
            break;
        }
        else if (ch == '*') {

            tokTyp = MUL;
            break;
        }
        else if (ch == '/') {

            tokTyp = DIV;
            break;
        }

        else if (ch == '(') {

            tokTyp = LPAREN;
            break;
        }
        else if (ch == ')') {
            tokTyp = RPAREN;
            break;
        }
        else if (isalpha(ch)) {                         // alphanumeric 
            column = ind;
            tokTyp = IDENTIFIER;
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
            tokTyp = NUM;
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