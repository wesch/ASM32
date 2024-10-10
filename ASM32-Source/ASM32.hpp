#ifndef ASM32_H
#define ASM32_H



extern char  SourceFileName[255];            // The .s filename
extern FILE* inputFile;                      // The .s input file
extern int         lineNr;                     // line number source file
extern int         column;                         // column of token in sourceline
extern char        sl[MAX_LINE_LENGTH];            // Line from source file
extern char        token[MAX_WORD_LENGTH];         // token value
extern char        token_old[MAX_WORD_LENGTH];     // save previous token
extern int         tokTyp;                         // type of token see enum
extern int         tokTyp_old;
extern int         numToken;                       // numeric value of token
extern int         mode;
extern int         value;
extern char        label[MAX_WORD_LENGTH];
extern int         ind;                        // index in sourceline for tokenizing
extern int         j;                          // inhdex to fill token
extern char        errmsg[MAX_ERROR_LENGTH];       // Error 
extern bool        is_label;
extern bool        is_instruction;
extern bool        is_directive;
extern bool        is_negative;


extern struct SymNode* scopeTab[10];
extern char scopeNameTab[50][10];
extern int currentScopeLevel;
extern int maxScopeLevel;
extern char currentScopeName[50];
extern ScopeType currentScopeType;
extern bool symFound;
extern int scopeLevelSave;
extern struct SymNode* global;
extern struct SymNode* program;
extern struct SymNode* module;
extern struct SymNode* function;
extern struct SymNode* block;
extern struct SymNode* directive;
extern char symFunc[50];           // Directive in Symtab 
extern char symValue[50];             // Wert in Symtab

// --------------------------------------------------------------------------------
//      token list
// --------------------------------------------------------------------------------

struct tokenList {
    int     lineNumber;
    int     column;
    int     tokTyp;                                 // type of token
    char    token[MAX_WORD_LENGTH];                 // token value
    struct  tokenList* next;
};
extern struct tokenList* next_t;
extern struct tokenList* start_t;
extern struct tokenList* ptr_t;                    // regular pointer


// --------------------------------------------------------------------------------
//      Symboltable
// --------------------------------------------------------------------------------

struct SymNode {
    ScopeType type;             // Art des Scopes
    int scopeLevel;             // Verschachtelungstiefe des Scopes
    char scopeName[50];         // Name des Scope

    char label[50];             // Name des Symbols (Label)
    char func[50];              // Funktion
    char value[50];             // Value des Symbols
    int linenr;                 // zeilenummer sourcecode

    struct SymNode** children;  // Zeiger auf die Kinder
    int child_count;            // Anzahl Kinder
} ;

// --------------------------------------------------------------------------------
//      Abstract Syntax Tree
// --------------------------------------------------------------------------------

 struct ASTNode {
    NodeType type;
    char* value;
    int linenr;
    int column;
    struct ASTNode** children;
    int child_count;
} ;

// --------------------------------------------------------------------------------
//      Function Prototypes 
// --------------------------------------------------------------------------------

// -- utils.cpp
void    CloseSourceFile();
void    FatalError(const char* msg);
int     HexCharToInt(char ch);
void    OpenSourceFile();
void    ProcessError(const char* msg);
void    PrintDebug(const char* msg);
void    StrToUpper(char* _str);

// -- lexer.cpp
void    NewTokenList();
void    PrintTokenList();
void    GetToken();

// -- parser.cpp
bool    checkGenReg();
void    Evaluate();
void    NextToken();
void    PrintSymbolTokenCode(int i);
void    PrintTokenCode(int i);
void    ProcessDirective();
void    ProcessInstruction();
void    ProcessLabel();
void    ParseModInstr();
void    SkipToEOL();
int     ParseExpression();
int     ParseLogicalExpression();
int     ParseTerm();
int     ParseFactor();

void addDirective(ScopeType type, char* label, char* func, const char* value, int linenr);
void addScope(ScopeType type, char* label, char* func, const char* value, int linenr);
void add_child(SymNode* parent, SymNode* child);
SymNode* create_node(ScopeType type, char* label, char* func, const char* value, int linenr);
void print_sym(SymNode* node, int depth);
void searchSymAll(SymNode* node, char* label, int depth);
void searchSymLevel(SymNode* node, char* label, int depth);
bool searchSymbol(SymNode* node, char* label);


#endif