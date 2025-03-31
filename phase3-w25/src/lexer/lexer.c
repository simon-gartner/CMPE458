/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/tokens.h"
#include "../../include/lexer.h"

/* Static lexer state */
static int current_line;
static int current_column;
static char last_token_type;

/* Resets lexer state; call this before lexing a new input */
void lexer_reset(void) {
    current_line = 1;
    current_column = 1;
    last_token_type = 'x';
}

/* Keywords table */
static struct {
    const char* word;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},
    {"int", TOKEN_INT},
    {"print", TOKEN_PRINT},
    {"while", TOKEN_WHILE},
    {"repeat", TOKEN_REPEAT},
    {"until", TOKEN_UNTIL},
    {"factorial", TOKEN_FACTORIAL}
};

static int is_keyword(const char* word) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i].word) == 0) {
            return keywords[i].type;
        }
    }
    return 0;
}

void print_error(ErrorType error, int line, const char* lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch(error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        case ERROR_INVALID_IDENTIFIER:
            printf("Invalid identifier\n");
            break;
        case ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }

    printf("Token: ");
    switch(token.type) {
        case TOKEN_NUMBER:     printf("NUMBER"); break;
        case TOKEN_OPERATOR:   printf("OPERATOR"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
        case TOKEN_EQUALS:     printf("EQUALS"); break;
        case TOKEN_SEMICOLON:  printf("SEMICOLON"); break;
        case TOKEN_LPAREN:     printf("LPAREN"); break;
        case TOKEN_RPAREN:     printf("RPAREN"); break;
        case TOKEN_LBRACE:     printf("LBRACE"); break;
        case TOKEN_RBRACE:     printf("RBRACE"); break;
        case TOKEN_IF:         printf("IF"); break;
        case TOKEN_INT:        printf("INT"); break;
        case TOKEN_PRINT:      printf("PRINT"); break;
        case TOKEN_WHILE:      printf("WHILE"); break;
        case TOKEN_REPEAT:     printf("REPEAT"); break;
        case TOKEN_EOF:        printf("EOF"); break;
        case TOKEN_LESS:       printf("LESS"); break;
        case TOKEN_GREATER:    printf("GREATER"); break;
        default:               printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n", token.lexeme, token.line);
}

Token get_next_token(const char* input, int* pos) {
    char c;
    /* Skip whitespace using isspace() */
    while (input[*pos] != '\0' && isspace(input[*pos])) {
        if (input[*pos] == '\n') {
            current_line++;
            current_column = 1;
        } else {
            current_column++;
        }
        (*pos)++;
    }

    int token_line = current_line;
    int token_column = current_column;
    
    Token token = {TOKEN_ERROR, "", token_line, token_column, ERROR_NONE};

    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    c = input[*pos];

    /* Handle numbers */
    if (isdigit(c)) {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            current_column++;
            c = input[*pos];
        } while (isdigit(c) && i < (int)(sizeof(token.lexeme) - 1));
        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        last_token_type = 'x';
        return token;
    }

    /* Handle identifiers and keywords */
    if (isalpha(c) || c == '_') {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            current_column++;
            c = input[*pos];
        } while ((isalnum(c) || c == '_') && i < (int)(sizeof(token.lexeme) - 1));
        token.lexeme[i] = '\0';
        TokenType keyword_type = is_keyword(token.lexeme);
        if (keyword_type) {
            token.type = keyword_type;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        last_token_type = 'x';
        return token;
    }

    /* Handle multi-character operators */
    if (c == '=' && input[*pos + 1] == '=') {
        token.type = TOKEN_EQUAL_EQUAL;
        strcpy(token.lexeme, "==");
        (*pos) += 2;
        current_column += 2;
        return token;
    }
    
    if (c == '!' && input[*pos + 1] == '=') {
        token.type = TOKEN_NOT_EQUAL;
        strcpy(token.lexeme, "!=");
        (*pos) += 2;
        current_column += 2;
        return token;
    }

    /* Handle single-character tokens */
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    (*pos)++;
    current_column++;

    switch (c) {
        case '+': case '-': case '*': case '/':
            if (last_token_type == 'o') {
                token.error = ERROR_CONSECUTIVE_OPERATORS;
                return token;
            }
            token.type = TOKEN_OPERATOR;
            last_token_type = 'o';
            break;
        case '=':
            token.type = TOKEN_EQUALS;
            last_token_type = 'x';
            break;
        case '<':
            token.type = TOKEN_LESS;
            last_token_type = 'x';
            break;
        case '>':
            token.type = TOKEN_GREATER;
            last_token_type = 'x';
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            last_token_type = 'x';
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            last_token_type = 'x';
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            last_token_type = 'x';
            break;
        case '{':
            token.type = TOKEN_LBRACE;
            last_token_type = 'x';
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            last_token_type = 'x';
            break;
        default:
            token.error = ERROR_INVALID_CHAR;
            last_token_type = 'x';
            break;
    }

    /* If the next character is a newline, update line and column */
    if (input[*pos] == '\n') {
        current_line++;
        current_column = 1;
    }

    return token;
}

/* Uncomment the main function below for standalone testing

int main() {
    const char *input = 
        "int x = 123;\n"
        "test_var = 456;\n"
        "print x;\n"
        "if (y > 10) {\n"
        "    @#$ invalid\n"
        "    x = ++2;\n"
        "}\n"
        "x = 3 + 4 * 5;";
        
    printf("Analyzing input:\n%s\n\n", input);
    lexer_reset();  // Reset lexer state before lexing
    int position = 0;
    Token token;
    
    do {
        token = get_next_token(input, &position);
        print_token(token);
    } while (token.type != TOKEN_EOF);
    
    return 0;
}
*/

