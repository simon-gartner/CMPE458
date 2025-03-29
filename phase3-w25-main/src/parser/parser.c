/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"

/* 
   Assumption: The ASTNode structure is updated to include a 'next' pointer,
   for example:

   typedef struct ASTNode {
       ASTNodeType type;
       Token token;
       struct ASTNode *left;
       struct ASTNode *right;
       struct ASTNode *next;  // For linking statements in a program
   } ASTNode;
*/

/* Function declarations for statement parsing */
static ASTNode* parse_if_statement();
static ASTNode* parse_while_statement();
static ASTNode* parse_repeat_statement();
static ASTNode* parse_print_statement();
static ASTNode* parse_block_statement();
static ASTNode* parse_factorial();
static ASTNode* parse_declaration();
static ASTNode* parse_assignment();
static ASTNode* parse_statement();

//ADDED NEW PARSING FUNCTIONS IN ADDITION TO OLDER ONES, might break stuff
static ASTNode *parse_expression(void);
static ASTNode *parse_equality(void);
static ASTNode *parse_comparison(void);
static ASTNode *parse_additive(void);
static ASTNode *parse_multiplicative(void);
static ASTNode *parse_primary(void);

/* Global variables for token management */
static Token current_token;
static Token previous_token;
static int position = 0;
static const char *source;

/* Error handling */
static ParseErrorInfo errors[MAX_ERRORS];
static int error_count = 0;

static void parse_error(ParseError error, Token token) {
    if (error_count >= MAX_ERRORS) return;

    int column = token.column;
    if (error == PARSE_ERROR_MISSING_SEMICOLON) {
        column += strlen(token.lexeme);
    }

    errors[error_count] = (ParseErrorInfo){
        .type = error,
        .position = {token.line, token.column},
        .message = ""
    };

    switch (error) {
        case PARSE_ERROR_MISSING_SEMICOLON:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing semicolon after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing identifier after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Unexpected '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_EQUALS:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Expected '=' after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_EXPRESSION:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Invalid expression starting with '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_PARENTHESES:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing parentheses for '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_CONDITION_STATEMENT:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Expected condition after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_BLOCK_BRACES:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Expected '{}' block after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_OPERATOR:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Invalid operator '%s'", token.lexeme);
            break;
        case PARSE_ERROR_FUNCTION_CALL:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Invalid function call '%s'", token.lexeme);
            break;
        default:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Unknown error at %d:%d", token.line, token.column);
    }
    error_count++;
}

void print_errors(void) {
    for (int i = 0; i < error_count; i++) {
        printf("Error %d:%d: %s\n", 
               errors[i].position.line,
               errors[i].position.column,
               errors[i].message);
    }
}

/* Token management functions */
static void advance(void) {
    previous_token = current_token;
    current_token = get_next_token(source, &position);
}

static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        node->token = current_token;
        node->left = NULL;
        node->right = NULL;
        node->next = NULL;
    }
    return node;
}

static int match(TokenType type) {
    return current_token.type == type;
}

static void synchronize(void) {
    while (!match(TOKEN_SEMICOLON) &&
           !match(TOKEN_RBRACE) &&
           !match(TOKEN_LBRACE) &&
           !match(TOKEN_IF) &&
           !match(TOKEN_WHILE) &&
           !match(TOKEN_REPEAT) &&
           !match(TOKEN_INT) &&
           !match(TOKEN_PRINT) &&
           !match(TOKEN_EOF)) {
        advance();
    }
    if (match(TOKEN_SEMICOLON)) advance();
}

static void expect(TokenType type) {
    if (!match(type)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
    }
    advance();
}

//Newly added parsing functions, hopefully works


static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    node->token = current_token;
    advance();

    if (match(TOKEN_NUMBER)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        Token number_token = current_token;
        advance(); 
        
        if (!match(TOKEN_SEMICOLON)) {
            parse_error(PARSE_ERROR_MISSING_SEMICOLON, number_token);
        } else {
            advance();
        }
        return node;
    }

    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, previous_token);
        synchronize();
        return node;
    }

    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, previous_token);
    } else {
        advance();
    }
    return node;
}



static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_ASSIGN);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance(); // not sure if needed, increments pointer

    if (!match(TOKEN_EQUALS)) {
        parse_error(PARSE_ERROR_MISSING_EQUALS, previous_token);
        free_ast(node);
        synchronize();
        return NULL;
    }
    advance(); 

    node->right = parse_expression();
    if (!node->right) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        free_ast(node);
        synchronize();
        return NULL;
    }

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, previous_token);
    } else {
        advance();
    }
    return node;
}

//parsing primary tokens 
static ASTNode *parse_primary(void) {
    if (match(TOKEN_NUMBER)) {
        ASTNode *node = create_node(AST_NUMBER);
        node->token = current_token;
        advance();
        return node;
    } else if (match(TOKEN_IDENTIFIER)) {
        ASTNode *node = create_node(AST_IDENTIFIER);
        node->token = current_token;
        advance();
        return node;
    } else if (match(TOKEN_LPAREN)) {
        advance(); // consume '('
        ASTNode *node = parse_expression();
        if (!match(TOKEN_RPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        } else {
            advance(); // consume ')'
        }
        return node;
    } else {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        return NULL;
    }
}

// parsing the multiplication, since the lexer doesent specify operators
static ASTNode *parse_multiplicative(void) {
    ASTNode *node = parse_primary();
    while (match(TOKEN_OPERATOR) && 
           (strcmp(current_token.lexeme, "*") == 0 || strcmp(current_token.lexeme, "/") == 0)) {
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = current_token;  // operator token ("*" or "/")
        new_node->left = node;
        advance(); // consume operator
        new_node->right = parse_primary();
        node = new_node;
    }
    return node;
}

// parsing for addition and subtraction
static ASTNode *parse_additive(void) {
    ASTNode *node = parse_multiplicative();
    while (match(TOKEN_OPERATOR) &&
           (strcmp(current_token.lexeme, "+") == 0 || strcmp(current_token.lexeme, "-") == 0)) {
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = current_token;  // operator token ("+" or "-")
        new_node->left = node;
        advance(); // consume operator
        new_node->right = parse_multiplicative();
        node = new_node;
    }
    return node;
}

//parsing for greater than and less than
static ASTNode *parse_comparison(void) {
    ASTNode *node = parse_additive();
    while (match(TOKEN_LESS) || match(TOKEN_GREATER)) {
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = current_token;  // token is TOKEN_LESS or TOKEN_GREATER
        new_node->left = node;
        advance(); // consume operator
        new_node->right = parse_additive();
        node = new_node;
    }
    return node;
}

// parsing multiple operators
static ASTNode *parse_equality(void) {
    ASTNode *node = parse_comparison();
    while (match(TOKEN_EQUAL_EQUAL) || match(TOKEN_NOT_EQUAL)) {
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = current_token;  // token is TOKEN_EQUAL_EQUAL or TOKEN_NOT_EQUAL
        new_node->left = node;
        advance(); // consume operator
        new_node->right = parse_comparison();
        node = new_node;
    }
    return node;
}


static ASTNode *parse_expression(void) {
    return parse_equality();
}
//if statement parsing fixes
static ASTNode *parse_if_statement(void) {
    ASTNode *node = create_node(AST_IF);
    advance(); 

    if (match(TOKEN_LPAREN)) { //ensuring if has parenthesizes 
        advance(); 
        node->left = parse_expression();
        if (!match(TOKEN_RPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, previous_token);
        } else {
            advance(); 
        }
    } else {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }

    if (match(TOKEN_LBRACE)) {
        node->right = parse_block_statement();
    } else {
        node->right = parse_statement();
    }
    
    return node;
}

//doing same thing for if with while
static ASTNode *parse_while_statement(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance();

    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();
    node->left = parse_expression();
    if (node->left == NULL) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
    }
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();

    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance();
    node->right = parse_block_statement();
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance();
    return node;
}

// repeat stuff
static ASTNode *parse_repeat_statement(void) {
    ASTNode *node = create_node(AST_REPEAT);
    advance(); // consume 'repeat'

    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance(); // consume '{'
    node->right = parse_block_statement();
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance(); // consume '}'

    if (!match(TOKEN_UNTIL)) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
    }
    advance(); // consume 'until'

    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance(); // consume '('
    node->left = parse_expression();
    if (node->left == NULL) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
    }
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance(); // consume ')'
    return node;
}

// parsing for print statements
static ASTNode* parse_print_statement(void) {
    ASTNode *node = create_node(AST_PRINT);
    advance(); // consume 'print'

    if (!match(TOKEN_IDENTIFIER) && !match(TOKEN_NUMBER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, previous_token);
        synchronize();
        return node;
    }
    node->left = parse_expression();
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
    } else {
        advance(); // consume ';'
    }
    return node;
}

// block statement parsing
static ASTNode* parse_block_statement(void) {
    ASTNode *node = create_node(AST_BLOCK);
    /* Use the 'next' pointer for linking statements in a block */
    ASTNode **current = &node->next;
    
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    } else {
        advance(); // consume '{'
    }
    
    while (!match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            *current = stmt;
            current = &stmt->next;
        }
        if (match(TOKEN_SEMICOLON)) {
            advance();
        }
    }
    
    if (match(TOKEN_RBRACE)) {
        advance(); // consume '}'
    }
    
    return node;
}

//factorial parsing
static ASTNode* parse_factorial(void) {
    ASTNode *node = create_node(AST_FACTORIAL);
    advance(); // consume 'factorial'
    
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance(); // consume '('
    
    if (!match(TOKEN_NUMBER) && !match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
    }
    advance();
    
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance(); // consume ')'
    
    return node;
}

// statement parsing, putting it all together
static ASTNode *parse_statement(void) {
    ASTNode *stmt = NULL;
    if (match(TOKEN_INT)) {
        stmt = parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        stmt = parse_assignment();
    } else if (match(TOKEN_IF)) {
        stmt = parse_if_statement();
    } else if (match(TOKEN_WHILE)) {
        stmt = parse_while_statement();
    } else if (match(TOKEN_REPEAT)) {
        stmt = parse_repeat_statement();
    } else if (match(TOKEN_PRINT)) {
        stmt = parse_print_statement();
    } else if (match(TOKEN_FACTORIAL)) {
        stmt = parse_factorial();
    } else {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        advance(); // consume invalid token
    }
    return stmt;
}

// putting together putting together lol
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    /* Link statements using the 'next' pointer in the program node */
    ASTNode **current = &program->next;
    
    while (!match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            *current = stmt;
            current = &stmt->next;
        } else {
            while (!match(TOKEN_SEMICOLON) && 
                   !match(TOKEN_RBRACE) && 
                   !match(TOKEN_EOF)) {
                advance();
            }
            if (match(TOKEN_SEMICOLON)) advance();
        }
    }
    
    return program;
}

void parser_init(const char *input) {
    source = input;
    position = 0;
    advance(); 
}

// run parse function
{
    return parse_program();
}

// print parse tree
void print_ast(ASTNode *node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("  ");
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->token.lexeme);
            break;
        case AST_ASSIGN:
            printf("Assign\n");
            break;
        case AST_NUMBER:
            printf("Number: %s\n", node->token.lexeme);
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->token.lexeme);
            break;
        case AST_IF:
            printf("If\n");
            break;
        case AST_WHILE:
            printf("While\n");
            break;
        case AST_BLOCK:
            printf("Block\n");
            break;
        case AST_BINOP:
            printf("BinaryOp: %s\n", node->token.lexeme);
            break;
        case AST_PRINT:
            printf("Print\n");
            break;
        case AST_REPEAT:
            printf("Repeat\n");
            break;
        case AST_FACTORIAL:
            printf("Factorial\n");
            break;
        default:
            printf("Unknown node type\n");
    }
    
    // Print children one level deeper
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
    // Print the next statement at the same level
    print_ast(node->next, level);
}

// freeing mem incase of errors
void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->next);
    free(node);
}

//changed testing function for operator precidense.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <textfile>\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Could not open file '%s'\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = malloc(file_size + 1);
    if (!source) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t bytes_read = fread(source, 1, file_size, file);
    source[bytes_read] = '\0';
    fclose(file);
    
    printf("Parsing file: %s\n", argv[1]);
    parser_init(source);
    ASTNode *ast = parse();
    
    if (error_count > 0) {
        printf("\n%d errors found:\n", error_count);
        print_errors();
    } else {
        printf("\nFile parsed successfully!\n");
        printf("Abstract Syntax Tree:\n");
        print_ast(ast, 0);
    }
    
    free_ast(ast);
    free(source);
    
    return 0;
}

