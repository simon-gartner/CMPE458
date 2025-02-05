/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"

// Line tracking
static int current_line = 1;
static char last_token_type = 'x'; // For checking consecutive operators

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

/* Print token information
 *
 *  TODO Update your printing function accordingly
 */

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }

    printf("Token: ");
    switch (token.type) {
        case TOKEN_NUMBER:
            printf("NUMBER");
            break;
        case TOKEN_OPERATOR:
            printf("OPERATOR");
            break;
        case TOKEN_KEYWORD:
            printf("KEYWORD");
            break;
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER");
            break;
        case TOKEN_COMMENT:
            printf("COMMENT");
            break;
        case TOKEN_EOF:
            printf("EOF");
            break;
        case TOKEN_STRING:
            printf("STRING");
            break;
        default:
            printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n",
           token.lexeme, token.line);
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

    // Comment handling here
    if (c == '/') {
        int i = 0;
        // Single line comment checking
        if (input[*pos+1] == '/'){
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            // Scan until newline
            while ( c != '\n') {
                if (c == '\0') {
                    break;
                }
                token.lexeme[i++] = c;
                (*pos)++;
                c = input[*pos];
            }
            token.lexeme[i] = '\0';
            token.type = TOKEN_COMMENT;
            return token;
        }
        // Multi line comment checking
        if (input[*pos+1] == '*'){
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            char d = input[*pos+1];
            // Scan until */ is found
            while (c != '*' || d != '/') {
                token.lexeme[i++] = c;
                (*pos)++;
                c = input[*pos];
                d = input[*pos+1];
            }
            // add closing comment to token
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            token.lexeme[i] = '\0';
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
            token.lexeme[i] = '\0';
            token.type = TOKEN_COMMENT;
            return token;
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

    // TODO: Add keyword and identifier handling here
    // Hint: You'll have to add support for keywords and identifiers, and then string literals
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
            token.lexeme[i++] = input[(*pos)++];
        } 

        if (input[*pos] == '"') (*pos)++;
        
        token.lexeme[i] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    // Handle operators
    if (c == '+' || c == '-') {
        if (last_token_type == 'o') {
            // Check for consecutive operators
            token.error = ERROR_CONSECUTIVE_OPERATORS;
            token.lexeme[0] = c;
            token.lexeme[1] = '\0';
            (*pos)++;
            return token;
        }
        token.type = TOKEN_OPERATOR;
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        last_token_type = 'o';
        (*pos)++;
        return token;
    }

    // TODO: Add delimiter handling here

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
