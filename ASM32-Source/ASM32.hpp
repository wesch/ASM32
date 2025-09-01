#ifndef ASM32_H
#define ASM32_H


/// @file
/// \brief contains global structures

extern char  SourceFileName[255];            // The .s filename
extern FILE* inputFile;                      // The .s input file
extern int         lineNr;                     // line number source file
extern int         column;                         // column of token in sourceline
extern char        sl[MAX_LINE_LENGTH];            // Line from source file
extern int         prgType;                        // Programmtyp 0=undefined, 1=standalone mode, 2= Modul mode
extern char        token[MAX_WORD_LENGTH];         // token value
extern char        tokenSave[MAX_WORD_LENGTH];     // save previous token
extern int         tokTyp;                         // type of token see enum
extern int         tokTyp_old;
extern int         numToken;                       // numeric value of token
extern int         mode;
extern int64_t     value;
extern int         align_val;                      ///< alignemnt value
extern bool        lineERR;

extern char        label[MAX_WORD_LENGTH];
extern int         ind;                        // index in sourceline for tokenizing
extern int         j;                          // inhdex to fill token
extern char        errmsg[MAX_ERROR_LENGTH];       // Error 
extern char        infmsg[MAX_ERROR_LENGTH];
extern bool        is_label;
extern bool        is_instruction;
extern bool        is_directive;
extern bool        is_negative;
extern char        opCode[MAX_WORD_LENGTH];
extern int         OpType;                         // contains opCodetab.instr_type
extern int         operandType;
extern int         operandTyp[10];                 ///< Type of Operand for codegen
extern int         DirType;
extern char        dirCode[MAX_WORD_LENGTH];
extern int         VarType;
extern char        Variable[MAX_WORD_LENGTH];
extern char        buffer[255];

extern int         binInstr;                       // binaer instruction
extern int         binInstrSave;
extern char        opt1[MAX_WORD_LENGTH];       // option for AST
extern char        opt2[MAX_WORD_LENGTH];       // option for AST





extern int         codeAdr;                        // adress counter code
extern int         dataAdr;                        // adress counter data



// ELF Format output

using namespace ELFIO;
// extern elfio writer;
extern section* text_sec;
extern segment* text_seg;
extern section* data_sec;
extern segment* data_seg;
extern section* note_sec;


extern uint32_t    O_CODE_ADDR;
extern uint32_t    O_DATA_ADDR;
extern uint32_t    O_ENTRY;                        ///< Entry Point
extern bool        O_ENTRY_SET;
extern uint32_t    O_CODE_ALIGN;
extern uint32_t    O_DATA_ALIGN;

extern char        O_DATA[MAX_WORD_LENGTH];
extern char        O_TEXT[MAX_WORD_LENGTH];
extern int         O_dataLen;                      ///< offset in data memory area
extern bool        O_DataSectionData;
extern bool        O_TextSectionData;
extern char        *p_data;                         ///< pointer data memory area
extern int         p_dataSize;              ///< size of data area per malloc


extern char        func_entry[MAX_WORD_LENGTH];
extern bool        main_func_detected;
extern char opchar[5][10];                  
extern int  opnum[5];
extern char option[2][10];
extern int  opCount;
extern int  optCount;
extern int  bin_status;

extern struct SymNode* scopeTab[10];
extern char scopeNameTab[50][10];
extern int currentScopeLevel;
extern int searchScopeLevel;               // Level symtab for search
extern int maxScopeLevel;
extern char currentScopeName[50];
extern char currentScopeNameSave[50];
extern SYM_ScopeType currentScopeType;
extern bool symFound;
extern struct SymNode* GlobalSYM;
extern struct SymNode* program;
extern struct SymNode* module;
extern struct SymNode* function;
extern struct SymNode* block;
extern struct SymNode* directive;
extern SymNode* currentSym;
extern SymNode* currentSymSave;
extern char symFunc[50];                // Directive in Symtab 
extern char symValue[50];             // Wert in Symtab
extern int  symcodeAdr;

extern struct ASTNode* ASTprogram;
extern struct ASTNode* ASTinstruction;
extern struct ASTNode* ASToperation;
extern struct ASTNode* ASTop1;
extern struct ASTNode* ASTop2;
extern struct ASTNode* ASTop3;
extern struct ASTNode* ASTop4;
extern struct ASTNode* ASTlabel;
extern struct ASTNode* ASTmode;
extern struct ASTNode* ASTopt1;
extern struct ASTNode* ASTopt2;

extern SRC_NodeType currentSRC_type;
extern struct SRCNode* GlobalSRC;
extern struct SRCNode* SRCprogram;
extern struct SRCNode* SRCsource;
extern struct SRCNode* SRCbin;
extern struct SRCNode* SRCerror;
extern struct SRCNode* SRCcurrent;


extern bool DBG_TOKEN;
extern bool DBG_PARSER;
extern bool DBG_GENBIN;
extern bool DBG_SYMTAB;
extern bool DBG_AST;

// --------------------------------------------------------------------------------
/// \brief  token list
/// Einfach verkettete Liste mit gelesenenen Token
// --------------------------------------------------------------------------------

struct tokenList {
    int     lineNumber;                         ///< Zeilenummer sourcecode
    int     column;                             ///< Spalte sourcecode
    int     tokTyp;                             ///< Typ token
    char    token[MAX_WORD_LENGTH];             ///< Token Wert als Character
    struct  tokenList* next;                    ///< Nächstes token in Liste
};
extern struct tokenList* next_t;
extern struct tokenList* start_t;
extern struct tokenList* ptr_t;                 ///< regular pointer


// --------------------------------------------------------------------------------
/// \brief    Symboltable
// --------------------------------------------------------------------------------

struct SymNode {
    SYM_ScopeType type;         ///< Art des Scopes
    int scopeLevel;             ///< Verschachtelungstiefe des Scopes
    char scopeName[50];         ///< Name des Scope
    char label[50];             ///< Name des Symbols (Label)
    char func[50];              ///< Funktion
    char value[50];             ///< Value des Symbols
    int vartype;                ///< Type function
    int linenr;                 ///< Zeilenummer sourcecode
    int codeAdr;                ///< CodeAdresse 
    int dataAdr;                ///< DatenAdresse
    struct SymNode** children;  ///< Zeiger auf die Kinder
    int child_count;            ///< Anzahl Kinder
} ;

// --------------------------------------------------------------------------------
/// \brief      Abstract Syntax Tree
/// 
/// This structure contains the parsed tokens.
// --------------------------------------------------------------------------------

struct ASTNode {
    AST_NodeType type;
    char* value;                ///< Wert als Character              
    int valnum;                 ///< Numerischer Wert
    int linenr;                 ///< Zeilenummer sourcecode
    int column;                 ///< Spalte sourcecode
    int scopeLevel;             ///< Verschachtelungstiefe des Scopes
    char* scopeName;            ///< Scopename
    SymNode* symNodeAdr;        ///< Adresse aktueller Node Symboladresse
    int codeAdr;                ///< CodeAdresse
    int operandType;            ///< Type of operand 1=REGISTER, 2=MEMORY,3=LABEL
    uint32_t _binInstr;         ///< Instruction Wort
    struct ASTNode** children;  ///< Zeiger auf die Kinder
    int child_count;            ///< Anzahl Kinder
} ;

// --------------------------------------------------------------------------------
/// \brief    Source, Errormessages, binary output
// --------------------------------------------------------------------------------
struct SRCNode {
    SRC_NodeType type;
    int linenr;
    int codeAdr;
    uint32_t binInstr;
    int binstatus;              ///< bininstr 0=no bininstr, 1=bininstr, 2=bininstr in child
    char* text;
    int scopeLevel;             ///< Verschachtelungstiefe des Scopes
    struct SRCNode** children;
    int child_count;
};

// --------------------------------------------------------------------------------
//      Function Prototypes 
// --------------------------------------------------------------------------------


// -- main

SRCNode* Create_SRCnode(SRC_NodeType type, const char* text, int lineNr);
void Add_SRCchild(SRCNode* parent, SRCNode* child);
void PrintSRC(SRCNode* node, int depth);
void Search_SRC(SRCNode* node, int depth);
void InsertMC_SRC(SRCNode* node, int depth);

// -- utils.cpp
void    CloseSourceFile();
void    FatalError(const char* msg);
int     isunderline(char ch);
void    OpenSourceFile();
void    ProcessError(const char* msg);
void    PrintDebug(const char* msg);
void    StrToUpper(char* _str);
int StrToNum(char* _str);

// -- lexer.cpp
void    NewTokenList();
void    PrintTokenList();
void    GetToken();
bool SearchLabel(SymNode* node, char* label);
void SearchLabelAll(SymNode* node, char* label, int depth);

// -- parser.cpp
bool    CheckGenReg();
void    GetNextToken();
void    PrintSymbolTokenCode(int i);
void    PrintTokenCode(int i);
void    ParseDirective();
void    ParseInstruction();
void    ParseLabel();
void    ParseModInstr();
void    SkipToEOL();
int64_t      ParseExpression();
int64_t      ParseTerm();
int64_t      ParseFactor();
void    AddDirective(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    AddScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    Add_SYMchild(SymNode* parent, SymNode* child);
SymNode* Create_SYMnode(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    PrintSYM(SymNode* node, int depth);
void    SearchSymAll(SymNode* node, char* label, int depth);
void    SearchSymLevel(SymNode* node, char* label, int depth);
bool    SearchSymbol(SymNode* node, char* label);
ASTNode* Create_ASTnode(AST_NodeType type, const char* value, int valnum);
void    Add_ASTchild(ASTNode* parent, ASTNode* child);
void    PrintAST(ASTNode* node, int depth);


// -- codegen.cpp
void GenBinInstruction();
void GenBinOption();
void CodeGenAST(ASTNode* node, int depth);
void SetBit(int pos, int x, int num);
void SetOffset(int pos, int x, int num);
void SetGenRegister(int reg, char* regname);
void WriteBinary();

// -- ELF writer
int createELF();
int createTextSection();
int addTextSectionData();
int createTextSegment();
int addTextSectionToSegment();
int createDataSection();
int addDataSectionData();
int createDataSegment();
int addDataSectionToSegment();
int addNote();
int writeElfFile();




#endif