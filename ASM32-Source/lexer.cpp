#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief Lexer module for ASM32.
/// \details
/// This file implements the lexer, which reads source lines and
/// produces a linked list of tokens. Tokens represent identifiers,
/// literals, operators, and punctuation that will later be used by
/// the parser. It includes functions for building the token list,
/// printing it for debugging, and extracting tokens from the input.


// --------------------------------------------------------------------------------
//  Token List Management
// --------------------------------------------------------------------------------

/// \brief Create a new token list entry.
/// \details
/// Adds a new node to the global token list.  
/// If the list is empty, the first node is allocated and set as `start_t`.  
/// If the list already exists, the new node is appended at the end.
void createTokenEntry() {
    if (start_t == NULL) {  // Insert first element
        start_t = (struct tokenList*)malloc(sizeof(struct tokenList));

        if (start_t == NULL) {
            fatalError("malloc failed");
        }
        start_t->next = NULL;
        ptr_t = start_t;
    }
    else {  // At least one element already exists
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

/// \brief Print the token list.
/// \details
/// If debugging is enabled (`DBG_TOKEN == TRUE`),  
/// prints the contents of the token list, including line number,
/// column, token type, and token string.
void printTokenList() {
    if (DBG_TOKEN == TRUE) {
        printf("\nToken List\n");
        printf("---------------------------------------------\n");
        printf("Line#\tcol\tToktyp\tToken\n");
        printf("---------------------------------------------\n");

        struct tokenList* ptr_t = start_t;
        while (ptr_t != NULL) {
            printf("%d\t%d\t", ptr_t->lineNumber, ptr_t->column);
            printTokenCode(ptr_t->tokTyp);
            printf("\t%s\n", ptr_t->token);
            ptr_t = ptr_t->next;
        }
    }
}


// --------------------------------------------------------------------------------
//  Token Extraction
// --------------------------------------------------------------------------------

/// \brief Extract the next token from the current line.
/// \details
/// Reads characters from the global source buffer (`sl`) starting
/// at index `ind`, classifies them, and builds the next token string.
/// The token type is stored in `tokTyp`, and the token text in `token`.
///
/// Recognized token types include:
/// - End of line (`\n`, `;`)
/// - Punctuation (`. , : ( )`)
/// - Operators (`+ - * / ~ % | & ^`)
/// - Identifiers (letters, digits, underscores)
/// - Numbers (decimal, hexadecimal, binary-like with underscores)
/// - Special forms (`L%<num>`, `R%<num>`)
///
/// On completion, the global index `ind` is updated to the position
/// following the token.
void createToken() {
    int ch;                     ///< Current character under inspection.
    strcpy(token, "\0");        ///< Reset token buffer.
    int j = 0;                  ///< Token character index.
    tokTyp = NONE;              ///< Default token type.

    while (TRUE) {
        ch = sl[ind];
        column = ind;

        // End of line
        if (ch == '\n' || ch == ';') {
            tokTyp = T_EOL;
            strcpy(token, "EOL");
            break;
        }
        // Punctuation and operators
        else if (ch == '.') { tokTyp = T_DOT; strcpy(token, "."); break; }
        else if (ch == ',') { tokTyp = T_COMMA; strcpy(token, ","); break; }
        else if (ch == '_') { tokTyp = T_UNDERSCORE; strcpy(token, "_"); break; }
        else if (ch == ':') { tokTyp = T_COLON; strcpy(token, ":"); break; }
        else if (ch == '-') { tokTyp = T_MINUS; strcpy(token, "-"); break; }
        else if (ch == '+') { tokTyp = T_PLUS; strcpy(token, "+"); break; }
        else if (ch == '*') { tokTyp = T_MUL; strcpy(token, "*"); break; }
        else if (ch == '/') { tokTyp = T_DIV; strcpy(token, "/"); break; }
        else if (ch == '~') { tokTyp = T_NEG; strcpy(token, "~"); break; }
        else if (ch == '%') { tokTyp = T_MOD; strcpy(token, "%"); break; }
        else if (ch == '|') { tokTyp = T_OR; strcpy(token, "|"); break; }
        else if (ch == '&') { tokTyp = T_AND; strcpy(token, "%"); break; }
        else if (ch == '^') { tokTyp = T_XOR; strcpy(token, "^"); break; }
        else if (ch == '(') { tokTyp = T_LPAREN; strcpy(token, "("); break; }
        else if (ch == ')') { tokTyp = T_RPAREN; strcpy(token, ")"); break; }

        // Quoted string
        else if (ch == '"') {
            column = ind;
            tokTyp = T_IDENTIFIER;
            ind++;
            ch = sl[ind];
            while (ch != '"') {
                token[j++] = ch;
                ind++;
                ch = sl[ind];
            }
            token[j] = '\0';
            ind++;
            tokTyp = T_EOL;  // After a string, nothing else follows on the line
            break;
        }

        // Special forms: L% / R%
        else if (ch == 'L' && sl[ind + 1] == '%') {
            column = ind;
            tokTyp = T_NUM;
            ind += 2;
            ch = sl[ind];
            while (isdigit(ch)) {
                token[j++] = ch;
                ind++;
                ch = sl[ind];
            }
            token[j] = '\0';
            int n = atoi(token);
            n &= 0xFFFFFC00;
            snprintf(token, sizeof(token), "%d", n);
            ind--;
            break;
        }
        else if (ch == 'R' && sl[ind + 1] == '%') {
            column = ind;
            tokTyp = T_NUM;
            ind += 2;
            ch = sl[ind];
            while (isdigit(ch)) {
                token[j++] = ch;
                ind++;
                ch = sl[ind];
            }
            token[j] = '\0';
            int n = atoi(token);
            n &= 0x3FF;
            snprintf(token, sizeof(token), "%d", n);
            ind--;
            break;
        }

        // Identifier (letters, digits, underscores)
        else if (isalpha(ch)) {
            column = ind;
            tokTyp = T_IDENTIFIER;
            while (isalpha(ch) || isdigit(ch) || isunderline(ch)) {
                token[j++] = ch;
                ind++;
                ch = sl[ind];
            }
            token[j] = '\0';
            ind--;
            break;
        }

        // Number literal (decimal or hexadecimal)
        else if (isdigit(ch)) {
            column = ind;
            tokTyp = T_NUM;
            if (ch == '0' && sl[ind + 1] == 'x') {
                // Hexadecimal literal
                token[j] = ch;
                token[j + 1] = sl[ind + 1];
                j = 2;
                ind += 2;
                ch = sl[ind];
                while (isxdigit(ch) || isunderline(ch)) {
                    if (ch != '_') token[j++] = ch;
                    ind++;
                    ch = sl[ind];
                }
            }
            else {
                // Decimal literal
                while (isdigit(ch) || isunderline(ch)) {
                    if (ch != '_') token[j++] = ch;
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
