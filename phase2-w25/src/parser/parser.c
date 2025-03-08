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


// Current token being processed
static Token current_token;
static int position = 0;
static const char *source;


static void parse_error(ParseError error, Token token) {
    // ADDED more error types for:
    // - Missing parentheses
    // - Missing condition
    // - Missing block braces
    // - Invalid operator
    // - Function call errors

    printf("Parse Error at line %d: ", token.line);
    switch (error) {
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_SEMICOLON:
            printf("Missing semicolon after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("Expected identifier after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_EQUALS:
            printf("Expected '=' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_EXPRESSION:
            printf("Invalid expression after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_PARENTHESES:
            printf("Missing parentheses after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_CONDITION_STATEMENT:
            printf("Expected condition statement after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_BLOCK_BRACES:
            printf("Missing block braces after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_OPERATOR:
            printf("Invalid operator after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_FUNCTION_CALL:
            printf("Invalid function call '%s'\n", token.lexeme);
            break;                     
        default:
            printf("Unknown error\n");
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

// Expect a token type or error
static void expect(TokenType type) {
    if (match(type)) {
        advance();
    } else {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        exit(1); // Or implement error recovery
    }
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
    advance(); // consume 'int'

    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        exit(1);
    }

    node->token = current_token;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse assignment: x = 5;
static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_VARDECL);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_EQUALS)) {
        parse_error(PARSE_ERROR_MISSING_EQUALS, current_token);
        exit(1);
    }
    advance();

    node->right = parse_expression();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse if statements: if (condition) { ... }
static ASTNode *parse_if_statement(void) {
    ASTNode *node = create_node(AST_IF);
    advance(); // consume 'if'

    // Check for '('
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        exit(1);
    }
    advance();
    
    // Check for conditional statement
    node->left = parse_expression();

    if (node->left == NULL) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
        exit(1);
    }

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        exit(1);
    }
    advance();

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        exit(1);
    }
    advance();

    // Parse code block
    node->right = parse_expression();

    // Check for '}'
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        exit(1);
    }
    
    advance();
    return node;
}

// Parse while statements: while (condition) { ... }
static ASTNode *parse_while_statement(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance(); // consume 'while'

    // Check for '('
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        exit(1);
    }
    advance();
    
    // Check for conditional statement
    node->left = parse_expression();

    if (node->left == NULL) {
        parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
        exit(1);
    }

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        exit(1);
    }
    advance();

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        exit(1);
    }
    advance();

    // Parse code block
    node->right = parse_expression();

    // Check for '}'
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        exit(1);
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
            exit(1);
        }
        advance();
    
        // Parse code block
        node->right = parse_expression();

        // Check for '}'
        if (!match(TOKEN_RBRACE)) {
            parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
            exit(1);
        }
        advance();

        // Check for until keyword
        if (!match(TOKEN_UNTIL)) {
            parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
            exit(1);
        }
        advance();

        // Check for '('
        if (!match(TOKEN_LPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
            exit(1);
        }
        advance();

        // Check for conditional statement
        node->left = parse_expression();

        if (node->left == NULL) {
            parse_error(PARSE_ERROR_MISSING_CONDITION_STATEMENT, current_token);
            exit(1);
        }
        
        // Check for ')'
        if (!match(TOKEN_RPAREN)) {
            parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
            exit(1);
        }
        
        advance();
        return node;
}

// Parse print statements: print x
static ASTNode* parse_print_statement(void) {
    ASTNode *node = create_node(AST_PRINT);
    advance(); // consume 'print'

    // Check for variable to print
    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        exit(1);
    }

    node->token = current_token;
    advance();

    // check for ';'
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse block statements: { statement1; statement2; }
static ASTNode* parse_block_statement(void) {
    ASTNode *node = create_node(AST_BLOCK);

    // Check for '{'
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BLOCK_BRACES, current_token);
        exit(1);
    }
    advance();

    // Check for expressions until '}' is found
    while (match(TOKEN_RBRACE)) {
        node->left = parse_expression();
        if (node->left == NULL) {
            parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
            exit(1);
        }
        advance();

        // Check for ';'
        if (!match(TOKEN_SEMICOLON)) {
            parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
            exit(1);
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
        exit(1);
    }
    advance();
    
    // Check for number or variable
    if (!match(TOKEN_NUMBER) || !match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        exit(1);
    }
    advance();

    // Check for ')'
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_PARENTHESES, current_token);
        exit(1);
    }
    advance();
}

// Parse statement
static ASTNode *parse_statement(void) {
    if (match(TOKEN_INT)) {
        return parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        return parse_assignment();
    } else if (match(TOKEN_IF)) { 
        return parse_if_statement();
    } else if (match(TOKEN_WHILE)) {
        return parse_while_statement();
    } else if (match(TOKEN_REPEAT)) {
        return parse_repeat_statement();
    } else if (match(TOKEN_PRINT)) {
        return parse_print_statement();
    } else if (match(TOKEN_FACTORIAL)) {
        return parse_factorial();
    }
    // TODO 4: Add cases for new statement types
    // DONE else if (match(TOKEN_IF)) return parse_if_statement();
    // DONE else if (match(TOKEN_WHILE)) return parse_while_statement();
    // DONE else if (match(TOKEN_REPEAT)) return parse_repeat_statement();
    // DONE else if (match(TOKEN_PRINT)) return parse_print_statement();
    // ...

    printf("Syntax Error: Unexpected token\n");
    exit(1);
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
        exit(1);
    }

    return node;
}

// Parse program (multiple statements)
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    ASTNode *current = program;

    while (!match(TOKEN_EOF)) {
        current->left = parse_statement();
        if (!match(TOKEN_EOF)) {
            current->right = create_node(AST_PROGRAM);
            current = current->right;
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
    const char *invalid_input = "int x;\n"
                                "x = 42;\n"
                                "int ; \n"
                                "if (x == 42) x = 41 }\n"
                                "print x";

    printf("Parsing input:\n%s\n", input);
    parser_init(input);
    ASTNode *ast = parse();

    printf("\nAbstract Syntax Tree:\n");
    print_ast(ast, 0);

    free_ast(ast);
    return 0;
}
