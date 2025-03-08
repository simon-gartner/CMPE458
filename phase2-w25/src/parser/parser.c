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
static ASTNode *parse_term(void);
static ASTNode *parse_comparison(void);


// Current token being processed
static Token current_token;
static Token previous_token;

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
        case PARSE_ERROR_MISSING_SEMICOLON:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing semicolon after '%s'", previous_token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Missing identifier after '%s'", token.lexeme);
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
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            snprintf(errors[error_count].message, sizeof(errors[error_count].message), "Unexpected '%s'", token.lexeme);
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
    previous_token = current_token;
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
    while (!match(TOKEN_SEMICOLON) && 
           !match(TOKEN_RBRACE) && 
           !match(TOKEN_LBRACE) &&
           !match(TOKEN_IF) &&
           !match(TOKEN_WHILE) &&
           !match(TOKEN_REPEAT) &&
           !match(TOKEN_PRINT) &&
           !match(TOKEN_EOF)) {
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
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, previous_token);
        free_ast(node);
        synchronize();
        return NULL;
    }

    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, node->left->token);
        synchronize();
    } else {
        advance();
    }
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
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
    } else {
        advance();
    }
    return node;
}

static ASTNode *parse_comparison(void) {
    ASTNode *left = parse_term();
    
    if (match(TOKEN_EQUAL_EQUAL)) {
        ASTNode *node = create_node(AST_BINOP);
        node->token = current_token;
        node->left = left;
        advance();
        node->right = parse_term();
        return node;
    }
    
    return left;
}

static ASTNode *parse_term(void) {
    ASTNode *node;
    if (match(TOKEN_NUMBER)) {
        node = create_node(AST_NUMBER);
        node->token = current_token;
        advance();
    } else if (match(TOKEN_IDENTIFIER)) {
        node = create_node(AST_IDENTIFIER);
        node->token = current_token;
        advance();
    } else {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        return NULL;
    }
    return node;
}


// Parse if statements: if (condition) { ... }
static ASTNode *parse_if_statement(void) {
    ASTNode *node = create_node(AST_IF);
    advance(); 

    if (match(TOKEN_LPAREN)) {
        advance();
        node->left = parse_expression();
        if (!match(TOKEN_RPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        } else {
            advance();
        }
    } else {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
    }

    // Body parsing
    if (match(TOKEN_LBRACE)) {
        node->right = parse_block_statement();
    } else {
        node->right = parse_statement();
    }
    
    return node; // Always return node even with errors
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
    advance(); // consume 'print'

    if (!match(TOKEN_IDENTIFIER) && !match(TOKEN_NUMBER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        synchronize();
        return node;
    }

    node->left = parse_expression();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
    } else {
        advance();
    }
    return node;
}

// Parse block statements: { statement1; statement2; }
static ASTNode* parse_block_statement(void) {
    ASTNode *node = create_node(AST_BLOCK);
    ASTNode **current = &node->left;  // Initialize current pointer
    
    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
    } else {
        advance(); // Consume '{'
    }
    
    // Parse statements until closing brace
    while (!match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            *current = stmt;
            current = &(*current)->right;  // Move to next slot
        }
        
        if (match(TOKEN_SEMICOLON)) {
            advance();
        }
    }
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
    ASTNode *left = parse_term();
    
    if (match(TOKEN_EQUAL_EQUAL)) {
        ASTNode *node = create_node(AST_BINOP);
        node->token = current_token;
        node->left = left;
        advance();
        node->right = parse_term();
        return node;
    }
    
    return left;
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
            "if ( x == 42 ) { x = 41; }\n"
            "print x;"; // Valid if statement;
    // TODO 8: Add more test cases and read from a file:
    const char *invalid_input = "int x\n"
                                "x = 42;\n"
                                "int ; \n"
                                "if (x == 42) { x = 41 }\n"
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
