/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"

// Line tracking
static int current_line = 1;
static char last_token_type = 'x'; // For checking consecutive operators

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }
    printf("Token: %-10s | Lexeme: %-15s | Line: %d\n", tokenToString(token.type), token.lexeme, token.line);
}

const char* tokenToString(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER:     return "NUMBER";
        case TOKEN_OPERATOR:   return "OPERATOR";
        case TOKEN_KEYWORD:    return "KEYWORD";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_COMMENT:    return "COMMENT";
        case TOKEN_EOF:        return "EOF";
        case TOKEN_STRING:     return "STRING";
        case TOKEN_DELIMITER:  return "DELIMITER";
        case TOKEN_GRAMMAR:    return "PUNC";
        default:               return "UNKNOWN";
    }
}



/* Print error messages for lexical errors */
void print_error(ErrorType error, int line, const char *lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch (error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        default:
            printf("Unknown error\n");
    }
}

// List of keywords
const char *keywords[] = {"if", "for", "do", "while", "int", "return", "repeat", "until", "float", "string"};
const int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

// Function to check if a string is a keyword
int is_keyword(const char *str) {
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/* Get next token from input */
Token get_next_token(const char *input, int *pos) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;

    // Skip whitespace and track line numbers
    while ((c = input[*pos]) != '\0' && (c == ' ' || c == '\n' || c == '\t')) {
        if (c == '\n') {
            current_line++;
        }
        (*pos)++;
    }

    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    c = input[*pos];

    // Comment handling
    if (c == '/') {
        // Single line comment
        if (input[*pos + 1] == '/') {
            (*pos) += 2;

            while (input[*pos] != '\n' && input[*pos] != '\0') (*pos)++;

            if (input[*pos] == '\n') {
                current_line++;
                (*pos)++;
            }
            return get_next_token(input, pos);
        }

        // Multiline comment
        else if (input[*pos + 1] == '*') {
            (*pos) += 2;
            char d = input[*pos];
        
            while (!(c == '*' && d == '/') && c != '\0') {
                if (c == '\n') current_line++;

                (*pos)++;
                c = input[*pos];
                d = input[*pos + 1];
            }

        if (c == '*' && d == '/') (*pos) += 2;

        return get_next_token(input, pos); 
        }
    }
    

    // Handle numbers
    if (isdigit(c)) {
        size_t i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while (isdigit(c) && i < sizeof(token.lexeme) - 1);

        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    // Keyword and identifier handling here
    if (isalpha(c)){
        size_t i = 0;

        // Grab entirety of token
        while (( (isalpha(c) || isalnum(c) || c == '_') && i < sizeof(token.lexeme) - 1)) {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        }

        token.lexeme[i] = '\0';

        // Determine if keyword or identifier
        if (is_keyword(token.lexeme)) {
            token.type = TOKEN_KEYWORD;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        return token;
    }

    // Handle string literals
    if (c == '"') {
        size_t i = 0;
        (*pos)++; // Skip the opening '"' 

        while (input[*pos] != '"' && input[*pos] != '\0') {
            if (input[*pos] == '\\' && input[*pos+1] == '"') {
                token.lexeme[i++] = '"';
                (*pos) += 2;
            } else {
                token.lexeme[i++] = input[*pos];
                (*pos)++;
            }

            if (i >= 247){
                token.error = ERROR_TOO_LONG;
                break;
            }
        } 

        if (input[*pos] == '"') (*pos)++;
        
        token.lexeme[i] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    // Handle operators
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||  c == '<' || c == '>' || c == '!') {
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        (*pos)++;

        if (input[*pos] == '=' && (c == '=' || c == '!' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|')) {
            token.lexeme[1] = '=';
            token.lexeme[2] = '\0';
            (*pos)++;
        }

        if (input[*pos] == '&' && (c == '&')) {
            token.lexeme[1] = '&';
            token.lexeme[2] = '\0';
            (*pos)++;
        }

        if (input[*pos] == '|' && (c == '|')) {
            token.lexeme[1] = '|';
            token.lexeme[2] = '\0';
            (*pos)++;
        }

        if (input[*pos] == '+' && (c == '+')) {
            token.lexeme[1] = '+';
            token.lexeme[2] = '\0';
            (*pos)++;
        }

        token.type = TOKEN_OPERATOR;
        last_token_type = 'o';
        return token;
    }

    // Delimiter handling here
    if (c == ';') {
        token.type = TOKEN_DELIMITER;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'd';
        (*pos)++;
        return token;
    }

    // Grammar handling
    if (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == ',') {
        token.type = TOKEN_GRAMMAR;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'd';
        (*pos)++;
        return token;
    }

    // Handle invalid characters
    token.error = ERROR_INVALID_CHAR;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    (*pos)++;
    return token;
}

// This is a basic lexer that handles numbers (e.g., "123", "456"), basic operators (+ and -), consecutive operator errors, whitespace and newlines, with simple line tracking for error reporting.

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t input_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *input = malloc(input_size + 1);
    if (!input) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return 1;
    }

    fread(input, 1, input_size, file);
    input[input_size] = '\0';
    fclose(file);

    int position = 0;
    Token token;

    printf("Analyzing: \n%s\n\n", input);

    do {
        token = get_next_token(input, &position);
        print_token(token);
    } while (token.type != TOKEN_EOF);

    free(input);
    return 0;
}