/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"


// TODO 1: Add more parsing function declarations for:
static ASTNode* parse_if_statement(); // - if statements: if (condition) { ... }
static ASTNode* parse_while_statement(); // - while loops: while (condition) { ... }
static ASTNode* parse_repeat_statement(); // - repeat-until: repeat { ... } until (condition)
static ASTNode* parse_print_statement(); // - print statements: print x;
static ASTNode* parse_block(); // - blocks: { statement1; statement2; }
static ASTNode* parse_factorial(); // - factorial function: factorial(x)
static ASTNode* parse_block_statement();


// Current token being processed
static Token current_token;
static int position = 0;
static const char *source;


static ParseErrorInfo errors[MAX_ERRORS];

static int error_count = 0;

static void parse_error(ParseError error, Token token) {
    if (error_count >= MAX_ERRORS) return;

    errors[error_count] = (ParseErrorInfo){
        .type = error,
        .position = {token.line, token.column},
        .message = ""
    };

    switch (error) {
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Unexpected '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_SEMICOLON:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing semicolon after '%s'", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Expected identifier after '%s'", token.lexeme);
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
    for(int i = 0; i < error_count; i++) {
        printf("Error %d:%d: %s\n", 
               errors[i].position.line,
               errors[i].position.column,
               errors[i].message);
    }
}

// Get next token
static void advance(void) {
    current_token = get_next_token(source, &position);
}

// Create a new AST node
static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        node->token = current_token;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Match current token with expected type
static int match(TokenType type) {
    return current_token.type == type;
}

static void synchronize(void) {
    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        advance();
    }
    if (match(TOKEN_SEMICOLON)) advance();
}


// Expect a token type or error
static void expect(TokenType type) {
    if (!match(type)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
    }
    advance();
}

// Forward declarations
static ASTNode *parse_statement(void);

// TODO 3: Add parsing functions for each new statement type
// DONE static ASTNode* parse_if_statement(void) { ... }
// DONE static ASTNode* parse_while_statement(void) { ... }
// DONE static ASTNode* parse_repeat_statement(void) { ... }
// DONE static ASTNode* parse_print_statement(void) { ... }
// DONE static ASTNode* parse_block(void) { ... }
// DONE static ASTNode* parse_factorial(void) { ... }

static ASTNode *parse_expression(void);

// Parse variable declaration: int x;
static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    node->token = current_token;
    advance();

    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, node->token);
        synchronize();
        return NULL;
    }

    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, node->left->token);
        synchronize();
        return NULL;
    }
    advance();
    return node;
}

// Parse assignment: x = 5;
static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_ASSIGN);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;

    advance();

    if (!match(TOKEN_EQUALS)) {
        parse_error(PARSE_ERROR_MISSING_EQUALS, node->left->token);
        synchronize();
        free_ast(node);
        return NULL;
    }

    advance();

    if (!(node->right = parse_expression())) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        synchronize();
        free_ast(node);
        return NULL;
    }

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, node->token);
        synchronize();
        free_ast(node);
        return NULL;
    }

    advance();

    return node;
}

// Parse if statements: if (condition) { ... }
static ASTNode *parse_if_statement(void) {
    ASTNode *node = create_node(AST_IF);
    node->token = current_token;
    advance(); // consume 'if'

    // Check for '('
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES,  node->token);
        synchronize();
    } else {
        advance();
    }   
    
    // Check for conditional statement
    node->left = parse_expression();

    if (!(node->left)) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, node->token);
        synchronize();
    }

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, node->token);
        synchronize();
    } else {
        advance();
    }

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES,  node->token);
        synchronize();
    } else {
        advance();
        node->right = parse_block_statement();
    }

    // TODO ELSE HANDLING

    return node;
}

// Parse while statements: while (condition) { ... }
static ASTNode *parse_while_statement(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance(); // consume 'while'

    // Check for '('
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();
    
    // Check for conditional statement
    node->left = parse_expression();

    if (node->left == NULL) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
    }

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance();

    // Parse code block
    node->right = parse_expression();

    // Check for '}'
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    
    advance();
    return node;
}

// Parse repeat-until statements: repeat { ... } until (condition)
static ASTNode *parse_repeat_statement(void) {
    ASTNode *node = create_node(AST_REPEAT);
    advance(); // consume 'repeat'

        // Check for '{'
        if (!match(TOKEN_LBRACE)) {
            parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        }
        advance();
    
        // Parse code block
        node->right = parse_expression();

        // Check for '}'
        if (!match(TOKEN_RBRACE)) {
            parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        }
        advance();

        // Check for until keyword
        if (!match(TOKEN_UNTIL)) {
            parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        }
        advance();

        // Check for '('
        if (!match(TOKEN_LPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        }
        advance();

        // Check for conditional statement
        node->left = parse_expression();

        if (node->left == NULL) {
            parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
        }
        
        // Check for ')'
        if (!match(TOKEN_RPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        }
        
        advance();
        return node;
}

// Parse print statements: print x
static ASTNode* parse_print_statement(void) {
    ASTNode *node = create_node(AST_PRINT);
    node->token = current_token; 
    advance(); // consume 'print'

    // Check for variable to print
    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, node->token);
        synchronize();
        return node;
    }

    node->token = current_token;
    advance();

    // check for ';'
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, node->token);
        synchronize();
    } else {
        advance();
    }
    return node;
}

// Parse block statements: { statement1; statement2; }
static ASTNode* parse_block_statement(void) {
    ASTNode *node = create_node(AST_BLOCK);

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    }
    advance();

    // Check for expressions until '}' is found
    while (match(TOKEN_RBRACE)) {
        
        node->left = parse_expression();
        if (node->left == NULL) {
            parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        }
        advance();

        // Check for ';'
        if (!match(TOKEN_SEMICOLON)) {
            parse_error(PARSE_ERROR_MISSING_SEMICOLON, node->token);
        }
        advance();
    }
    // if (!match(TOKEN_RBRACE)) {
    //     parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    //     exit(1);
    // }
    advance(); 
    return node;
}

// Parse factorial statements factorial(x)
static ASTNode* parse_factorial(void){
    ASTNode *node = create_node(AST_FACTORIAL);
    advance();
     // Check for '('
     if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();
    
    // Check for number or variable
    if (!match(TOKEN_NUMBER) || !match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
    }
    advance();

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }
    advance();
}

// Parse statement
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
    }
    // TODO 4: Add cases for new statement types
    // DONE else if (match(TOKEN_IF)) return parse_if_statement();
    // DONE else if (match(TOKEN_WHILE)) return parse_while_statement();
    // DONE else if (match(TOKEN_REPEAT)) return parse_repeat_statement();
    // DONE else if (match(TOKEN_PRINT)) return parse_print_statement();
    // ...

    if (!stmt) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        // Consume invalid token to prevent infinite loop
        advance();
    }
    return stmt;
}

// Parse expression (currently only handles numbers and identifiers)

// TODO 5: Implement expression parsing
// Current expression parsing is basic. Need to implement:
// - Binary operations (+-*/)
// - Comparison operators (<, >, ==, etc.)
// - Operator precedence
// - Parentheses grouping
// - Function calls

static ASTNode *parse_expression(void) {
    ASTNode *node;

    if (match(TOKEN_NUMBER)) {
        node = create_node(AST_NUMBER);
        advance();
    } else if (match(TOKEN_IDENTIFIER)) {
        node = create_node(AST_IDENTIFIER);
        advance();
    } else {
        printf("Syntax Error: Expected expression\n");
    }

    return node;
}

// Parse program (multiple statements)
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    ASTNode **current = &program;

    while (!match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            *current = stmt;
            current = &(*current)->right;
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

// Initialize parser
void parser_init(const char *input) {
    source = input;
    position = 0;
    advance(); // Get first token
}

// Main parse function
ASTNode *parse(void) {
    return parse_program();
}

// Print AST (for debugging)
void print_ast(ASTNode *node, int level) {
    if (!node) return;

    // Indent based on level
    for (int i = 0; i < level; i++) printf("  ");

    // Print node info
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
        default:
            printf("Unknown node type\n");
    }

    // Print children
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

// Free AST memory
void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

// Main function for testing
int main() {
    // Test with both valid and invalid inputs
    const char *input = 
            "int x;\n" // Valid declaration
            "x = 42;\n" // Valid assignment;
            "if ( x == 42 ) { x = 41 }\n"
            "print x;"; // Valid if statement;
    // TODO 8: Add more test cases and read from a file:
    const char *invalid_input = "int x\n"
                                "x = 42;\n"
                                "int ; \n"
                                "if (x == 42) x = 41 }\n"
                                "print x";

    printf("Parsing input:\n%s\n", invalid_input);
    parser_init(invalid_input);

    ASTNode *ast = parse();

    if (error_count > 0) {
        printf("\n%d WARNING: Errors Found:\n", error_count);
        print_errors();
        free_ast(ast);
        return 1;
    }

    printf("\nAbstract Syntax Tree:\n");
    print_ast(ast, 0);

    free_ast(ast);
    return 0;
}
