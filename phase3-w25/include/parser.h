/* parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

#define MAX_ERRORS 256 // Can be adjusted later

// Basic node types for AST
typedef enum {
    AST_PROGRAM,        // Program node
    AST_VARDECL,        // Variable declaration (int x)
    AST_ASSIGN,         // Assignment (x = 5)
    AST_PRINT,          // Print statement
    AST_NUMBER,         // Number literal
    AST_IDENTIFIER,     // Variable name
    AST_IF,             // If statement 
    AST_WHILE,          // While statement
    AST_REPEAT,         // Repeat statement
    AST_BLOCK,          // Code block?
    AST_BINOP,          // Binary operator
    AST_FACTORIAL,      // Factorial function
    AST_ARRAYDECL,
    AST_ARRAYACCESS
} ASTNodeType;

typedef enum {
    PARSE_ERROR_NONE,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISSING_SEMICOLON,
    PARSE_ERROR_MISSING_IDENTIFIER,
    PARSE_ERROR_MISSING_EQUALS,
    PARSE_ERROR_INVALID_EXPRESSION,
    PARSE_ERROR_MISSING_PARENTHESES,
    PARSE_ERROR_MISSING_CONDITION_STATEMENT,
    PARSE_ERROR_MISSING_BLOCK_BRACES,
    PARSE_ERROR_INVALID_OPERATOR,
    PARSE_ERROR_FUNCTION_CALL,
    PARSE_ERROR_INVALID_ARRAY_SIZE,
    PARSE_ERROR_INVALID_ARRAY_INDEX
} ParseError;

// AST Node structure
typedef struct ASTNode {
    ASTNodeType type;           // Type of node
    Token token;               // Token associated with this node
    struct ASTNode* left;      // Left child
    struct ASTNode* right;     // Right child
    struct ASTNode* next; //for linking statements together
    // TODO: Add more fields if needed
} ASTNode;

typedef struct {
    int line;
    int column;
} SourcePosition;

typedef struct {
    ParseError type;
    char message[256];
    SourcePosition position;
} ParseErrorInfo;


// Parser functions
void parser_init(const char* input);
ASTNode* parse(void);
void print_ast(ASTNode* node, int level);
void free_ast(ASTNode* node);

#endif /* PARSER_H */