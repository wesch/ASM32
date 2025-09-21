#ifndef ASM32_H
#define ASM32_H

/// \file ASM32.h
/// \brief Global definitions, structures, and function prototypes for the assembler.
/// \details
/// This header defines all global variables, structures, and function prototypes
/// used across the assembler modules. It covers:
/// - Source file management
/// - Token and lexical analysis
/// - Symbol table
/// - Abstract Syntax Tree (AST)
/// - Source code representation (SRC)
/// - ELF file generation

// ============================================================================
// Global Variables
// ============================================================================

extern char  SourceFileName[255];             ///< Name of the input source file
extern FILE* inputFile;                       ///< Handle for the opened source file
extern int   lineNr;                          ///< Current line number in source file
extern int   column;                          ///< Current column number in source file
extern char  sl[MAX_LINE_LENGTH];             ///< Current source line buffer
extern int   prgType;                         ///< Program type (e.g., module, program, etc.)
extern char  token[MAX_WORD_LENGTH];          ///< Current token string
extern char  tokenSave[MAX_WORD_LENGTH];      ///< Backup of last token
extern int   tokTyp;                          ///< Current token type
extern int   tokTypSave;                      ///< Backup of token type
extern int   numToken;                        ///< Number of tokens parsed
extern int   mode;                            ///< Current parsing mode
extern int64_t value;                         ///< Numeric value of current token
extern int   align_val;                       ///< Alignment value for directives
extern bool  lineERR;                         ///< Error status of current line
extern int   numOfInstructions;
extern int   numOfData;

extern char  label[MAX_WORD_LENGTH];          ///< Current label name
extern char  labelCodeOld[MAX_WORD_LENGTH];
extern char  labelDataOld[MAX_WORD_LENGTH];
extern int   ind;                             ///< Generic index helper
extern int   j;                               ///< Generic counter helper
extern char  errmsg[MAX_ERROR_LENGTH];        ///< Last error message
extern char  infmsg[MAX_ERROR_LENGTH];        ///< Informational message buffer
extern bool  is_label;                        ///< Flag: current line contains a label
extern bool  is_instruction;                  ///< Flag: current line contains an instruction
extern bool  is_directive;                    ///< Flag: current line contains a directive
extern bool  is_negative;                     ///< Flag: current token is negative
extern char  opCode[MAX_WORD_LENGTH];         ///< Current operation mnemonic
extern int   opInstrType;                     ///< Type of current instruction
extern int   operandType;                     ///< Type of current operand
extern int   operandTyp[10];                  ///< Operand type list
extern int   directiveType;                   ///< Type of current directive
extern char  dirCode[MAX_WORD_LENGTH];        ///< Directive code string
extern int   varType;                         ///< Variable type
extern char  varName[MAX_WORD_LENGTH];        ///< Variable name
extern char  buffer[255];                     ///< General-purpose buffer

extern bool  codeInstrFlag;                   ///< Flag if .CODE or instruction was read in AST
extern int   nodeTypeOld;

extern bool  codeExist;                       ///< flag if a .CODE directive is present before first instruction    
extern bool  dataExist;                       ///< flag if a .DATA directive is present before first definitin of byte,half,word etc. 
extern int   numSegment;


extern int   binInstr;                        ///< Current binary instruction word
extern int   binInstrSave;                    ///< Saved binary instruction word
extern char  opt1[MAX_WORD_LENGTH];           ///< First instruction option
extern char  opt2[MAX_WORD_LENGTH];           ///< Second instruction option

extern int   codeAdr;                         ///< Current code address
extern int   dataAdr;                         ///< Current data address

using namespace ELFIO;
extern section* text_sec;                     ///< ELF .text section
extern segment* text_seg;                     ///< ELF .text segment
extern section* data_sec;                     ///< ELF .data section
extern segment* data_seg;                     ///< ELF .data segment
extern section* note_sec;                     ///< ELF .note section

extern uint32_t elfCodeAddr;                  ///< ELF code section base address
extern uint32_t elfCodeAddrOld;
extern uint32_t elfDataAddr;                  ///< ELF data section base address
extern uint32_t elfDataAddrOld;
extern uint32_t elfEntryPoint;                ///< ELF entry point address
extern bool     elfEntryPointStatus;          ///< Status: entry point defined
extern uint32_t elfCodeAlign;                 ///< Code section alignment
extern uint32_t elfDataAlign;                 ///< Data section alignment

extern char     elfData[MAX_WORD_LENGTH];     ///< ELF data section identifier
extern char     elfCode[MAX_WORD_LENGTH];     ///< ELF code section identifier
extern int      elfDataLength;                ///< ELF data section length
extern bool     elfDataSectionStatus;         ///< Flag: ELF data section defined
extern bool     elfCodeSectionStatus;         ///< Flag: ELF code section defined
extern char* elfBuffer;                    ///< ELF write buffer
extern int      elfBufferSize;                ///< ELF buffer size

extern char     func_entry[MAX_WORD_LENGTH];  ///< Function entry symbol
extern bool     main_func_detected;           ///< Flag: main() detected
extern char     opchar[5][10];                ///< Operator characters
extern int      opnum[5];                     ///< Operator numbers
extern char     option[2][10];                ///< Instruction options
extern int      opCount;                      ///< Operand count
extern int      optCount;                     ///< Option count
extern int      bin_status;                   ///< Binary generation status

// ============================================================================
// Symbol Table Globals
// ============================================================================

extern struct SymNode* scopeTab[10];          ///< Scope stack
extern char            scopeNameTab[50][10];  ///< Names of scopes
extern int             currentScopeLevel;     ///< Current scope nesting level
extern int             searchScopeLevel;      ///< Scope level for search
extern int             maxScopeLevel;         ///< Maximum scope level
extern char            currentScopeName[50];  ///< Current scope name
extern char            currentScopeNameSave[50]; ///< Saved scope name
extern SYM_ScopeType   currentScopeType;      ///< Current scope type
extern bool            symFound;              ///< Symbol search status
extern struct SymNode* GlobalSYM;             ///< Global symbol table root
extern struct SymNode* program;               ///< Program symbol node
extern struct SymNode* module;                ///< Module symbol node
extern struct SymNode* function;              ///< Function symbol node
extern struct SymNode* block;                 ///< Block symbol node
extern struct SymNode* directive;             ///< Directive symbol node
extern SymNode* currentSym;            ///< Current symbol
extern SymNode* currentSymSave;        ///< Saved current symbol
extern char            symFunc[50];           ///< Function name of symbol
extern char            symValue[50];          ///< Value of symbol
extern int             symcodeAdr;            ///< Code address of symbol

// ============================================================================
// AST Globals
// ============================================================================

extern struct ASTNode* ASTprogram;            ///< Root node of AST
extern struct ASTNode* ASTinstruction;        ///< Current AST instruction node
extern struct ASTNode* ASTdirective;
extern struct ASTNode* ASTcode;
extern struct ASTNode* ASToperation;          ///< AST operation node
extern struct ASTNode* ASTop1;                ///< AST operand 1
extern struct ASTNode* ASTop2;                ///< AST operand 2
extern struct ASTNode* ASTop3;                ///< AST operand 3
extern struct ASTNode* ASTop4;                ///< AST operand 4
extern struct ASTNode* ASTlabel;              ///< AST label node
extern struct ASTNode* ASTmode;               ///< AST mode node
extern struct ASTNode* ASTopt1;               ///< AST option 1
extern struct ASTNode* ASTopt2;               ///< AST option 2
extern struct ASTNode* ASTaddr;               ///< AST addr
extern struct ASTNode* ASTalign;              ///< AST align
extern struct ASTNode* ASTentry;              ///< AST entry

// ============================================================================
// SRC Globals
// ============================================================================

extern SRC_NodeType    currentSRC_type;       ///< Current SRC node type
extern struct SRCNode* GlobalSRC;             ///< Root of SRC tree
extern struct SRCNode* SRCprogram;            ///< SRC program node
extern struct SRCNode* SRCsource;             ///< SRC source node
extern struct SRCNode* SRCbin;                ///< SRC binary node
extern struct SRCNode* SRCerror;              ///< SRC error node
extern struct SRCNode* SRCcurrent;            ///< Current SRC node

// ============================================================================
// Debug Flags
// ============================================================================

extern bool DBG_TOKEN;   ///< Enable token debug output
extern bool DBG_PARSER;  ///< Enable parser debug output
extern bool DBG_GENBIN;  ///< Enable binary generation debug output
extern bool DBG_SYMTAB;  ///< Enable symbol table debug output
extern bool DBG_AST;     ///< Enable AST debug output

// ============================================================================
// Data Structures
// ============================================================================

/// \brief Token list (linked list of scanned tokens).
struct tokenList {
    int lineNumber;                      ///< Source line number
    int column;                          ///< Source column number
    int tokTyp;                          ///< Token type
    char token[MAX_WORD_LENGTH];         ///< Token value (as string)
    struct tokenList* next;              ///< Pointer to next token
};
extern struct tokenList* next_t;         ///< Next token pointer
extern struct tokenList* start_t;        ///< Start of token list
extern struct tokenList* ptr_t;          ///< General token pointer

/// \brief Symbol table node.
struct SymNode {
    SYM_ScopeType type;         ///< Scope type
    int scopeLevel;             ///< Scope nesting level
    char scopeName[50];         ///< Scope name
    char label[50];             ///< Symbol label
    char func[50];              ///< Function name
    char value[50];             ///< Symbol value
    int varType;                ///< Variable type
    int lineNr;                 ///< Source line number
    int codeAdr;                ///< Code address
    int dataAdr;                ///< Data address
    struct SymNode** children;  ///< Child symbols
    int childCount;             ///< Number of children
};

/// \brief Abstract Syntax Tree (AST) node.
struct ASTNode {
    AST_NodeType type;          ///< Node type
    char* value;                ///< String value
    int valnum;                 ///< Numeric value
    int lineNr;                 ///< Source line number
    int column;                 ///< Source column number
    int scopeLevel;             ///< Scope nesting level
    char* scopeName;            ///< Scope name
    SymNode* symNodeAdr;        ///< Linked symbol table node
    int codeAdr;                ///< Code address
    int operandType;            ///< Operand type (1=REGISTER, 2=MEMORY, 3=LABEL)
    uint32_t _binInstr;         ///< Encoded instruction word
    struct ASTNode** children;  ///< Child nodes
    int childCount;             ///< Number of children
};

/// \brief Source representation node (text, error, binary).
struct SRCNode {
    SRC_NodeType type;          ///< Node type
    int linenr;                 ///< Source line number
    int codeAdr;                ///< Code address
    uint32_t binInstr;          ///< Binary instruction
    int binStatus;              ///< Binary status (0=none, 1=exists, 2=in child)
    char* text;                 ///< Associated text
    int scopeLevel;             ///< Scope nesting level
    struct SRCNode** children;  ///< Child nodes
    int childCount;             ///< Number of children
};

/// \brief Segment Table.
typedef struct {
    char name[32];   // Name (31 chars + null terminator)
    char type;       // Type as a single character
    int addr;        // Address (integer)
    int len;         // Length (integer)
} SegmentTableEntry;

// ============================================================================
// Function Prototypes
// ============================================================================

// -- ASM32.cpp
SRCNode* createSRCnode(SRC_NodeType type, const char* text, int lineNr);
void addSRCchild(SRCNode* parent, SRCNode* child);
void printSourceListing(SRCNode* node, int depth);
void searchSRC(SRCNode* node, int depth);
void insertBinToSRC(SRCNode* node, int depth);
void setDefaultDirectives();
int addSegmentEntry(int index, const char* name, char type, int addr, int len);
int compareByAddr(const void* a, const void* b);
void printSegmentTable(int count);

// -- utils.cpp
void openSourceFile();
void closeSourceFile();
void fatalError(const char* msg);
int  isunderline(char ch);
void processError(const char* msg);
void processWarning(const char* msg);
void printDebug(const char* msg);
void strToUpper(char* _str);
int  strToNum(char* _str);

// -- lexer.cpp
void createTokenEntry();
void printTokenList();
void createToken();

// -- parser.cpp
bool    checkGenReg();
void    fetchToken();
void    printSymbolTokenCode(int i);
void    printTokenCode(int i);
void    ParseDirective();
void    parseInstruction();
void    parseLabel();
void    parseModInstr();
void    skipToEOL();
int64_t parseExpression();
int64_t parseTerm();
int64_t parseFactor();
void    addDirectiveToScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    addScope(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    addSYMchild(SymNode* parent, SymNode* child);
SymNode* createSYMnode(SYM_ScopeType type, char* label, char* func, const char* value, int linenr);
void    printSYM(SymNode* node, int depth);
void    searchSymAll(SymNode* node, char* label, int depth);
void    searchSymLevel(SymNode* node, char* label, int depth);
bool    searchSymbol(SymNode* node, char* label);
ASTNode* createASTnode(AST_NodeType type, const char* value, int valnum);
void    addASTchild(ASTNode* parent, ASTNode* child);
void    printAST(ASTNode* node, int depth);

// -- codegen.cpp
void clrBit(int pos);
void createBinary();
void genBinInstruction();
void genBinOption();
int  getSegRegister(char* regname);
void processAST(ASTNode* node, int depth);
void setBit(int pos, int x, int num);
void setGenRegister(int reg, char* regname);
void setMRRegister(char* regname);
void setOffset(int pos, int x, int num);

// -- ELF writer
int createELF();
int createTextSection(char* name);
int addTextSectionData();
int createTextSegment();
int addTextSectionToSegment();
int createDataSection(char* name);
int addDataSectionData(char* data, int len);
int createDataSegment();
int addDataSectionToSegment();
int addNote();
int writeElfFile();

#endif
