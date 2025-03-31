#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,      // e.g., "123", "456"
    TOKEN_OPERATOR,    // +, -, *, /
    TOKEN_IDENTIFIER,  // Variable names
    TOKEN_EQUALS,      // =
    TOKEN_SEMICOLON,   // ;
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_LBRACE,      // {
    TOKEN_RBRACE,      // }
    TOKEN_IF,          // if keyword
    TOKEN_INT,         // int keyword
    TOKEN_FLOAT,       // float keyword
    TOKEN_CHAR,        // char keyword
    TOKEN_PRINT,       // print keyword
    TOKEN_WHILE,       // while keyword
    TOKEN_REPEAT,      // repeat keyword
    TOKEN_UNTIL,       // until keyword
    TOKEN_FACTORIAL,   // factorial keyword

    // Comparison Operators
    TOKEN_LESS,        // <
    TOKEN_GREATER,     // >
    TOKEN_EQUAL_EQUAL, // ==
    TOKEN_NOT_EQUAL,   // !=

    TOKEN_ERROR,

    TOKEN_LBRACKET,
    TOKEN_RBRACKET
} TokenType;

typedef enum {
    ERROR_NONE,
    ERROR_INVALID_CHAR,
    ERROR_INVALID_NUMBER,
    ERROR_CONSECUTIVE_OPERATORS,
    ERROR_INVALID_IDENTIFIER,
    ERROR_UNEXPECTED_TOKEN
} ErrorType;

typedef struct {
    TokenType type;
    char lexeme[100];   // Actual text of the token
    int line; 
    int column;          // Line number in source file
    ErrorType error;    // Error type if any
} Token;

#endif /* TOKENS_H */
