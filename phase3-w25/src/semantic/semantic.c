/* semantic.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"
#include "../../include/semantic.h"


// Initialize a new symbol table
// Creates an empty symbol table structure with scope level set to 0
SymbolTable* init_symbol_table();

// Add a symbol to the table
// Inserts a new variable with given name, type, and line number into the current scope
void add_symbol(SymbolTable* table, const char* name, int type, int line);

// Look up a symbol in the table
// Searches for a variable by name across all accessible scopes
// Returns the symbol if found, NULL otherwise
Symbol* lookup_symbol(SymbolTable* table, const char* name);

// Main semantic analysis function
int analyze_semantics(ASTNode* ast);

// Check a variable declaration
int check_declaration(ASTNode* node, SymbolTable* table);

// Check a variable assignment
int check_assignment(ASTNode* node, SymbolTable* table);

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table);

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table);


int check_program(ASTNode* node, SymbolTable* table);

// Enter a new scope level
// Increments the current scope level when entering a block (e.g., if, while)
void enter_scope(SymbolTable* table){
    table->current_scope++;
}

void remove_symbols_in_current_scope(SymbolTable* table){
    Symbol* prev = NULL;
    Symbol* curr = table->head;
    while (curr) {
        if (curr->scope_level == table->current_scope) {
            if (prev) {
                prev->next = curr->next;
            } else {
                table->head = curr->next;
            }
            Symbol* temp = curr;
            curr = curr->next;
            free(temp);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

// Exit the current scope
// Decrements the current scope level when leaving a block
// Optionally removes symbols that are no longer in scope
void exit_scope(SymbolTable* table){
    remove_symbols_in_current_scope(table);
    table->current_scope--;
}

// Free the symbol table memory
// Releases all allocated memory when the symbol table is no longer needed
void free_symbol_table(SymbolTable* table){
    Symbol* curr = table->head;
    while (curr) {
        Symbol* next = curr->next;
        free(curr);
        curr = next;
    }
    free(table);
}

// Check an expression for type correctness
int check_expression(ASTNode* node, SymbolTable* table){
    if (!node) return 0;
    int valid = 1;
    switch (node->type) {
        case AST_NUMBER:
            // Number literals are int by default.
            break;
        case AST_IDENTIFIER: {
            // Check that the variable is declared.
            Symbol* symbol = lookup_symbol(table, node->token.lexeme);
            if (!symbol) {
                semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
                valid = 0;
            } else if (!symbol->is_initialized) {
                // Warn if used without initialization.
                semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->token.lexeme, node->token.line);
            }
            break;
        }
        case AST_BINOP: {
            // Recursively check both operands.
            int leftValid = check_expression(node->left, table);
            int rightValid = check_expression(node->right, table);
            valid = leftValid && rightValid;
            // In a more advanced type system, you would compare the types of both operands here.
            break;
        }
        case AST_FACTORIAL:
            // Factorial should have a valid operand.
            if (!node->left) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "factorial", node->token.line);
                valid = 0;
            } else {
                valid = check_expression(node->left, table);
            }
            break;
        default:
            // For any other expression types, recursively check child nodes.
            valid = check_expression(node->left, table) && check_expression(node->right, table);
            break;
    }
    return valid;
}

int check_statement(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;

    switch (node->type) {
        case AST_VARDECL:
            return check_declaration(node, table);
        case AST_ASSIGN:
            return check_assignment(node, table);
        case AST_IF:
            return check_condition(node->left, table) && check_block(node->right, table);
        case AST_WHILE:
            return check_condition(node->left, table) && check_block(node->right, table);
        case AST_BLOCK:
            return check_block(node, table);
        case AST_PRINT:
            return check_expression(node->left, table);
        default:
            semantic_error(SEM_ERROR_INVALID_OPERATION, node->token.lexeme, node->token.line);
            return 0;
    }
}

// Initialize symbol table
SymbolTable* init_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;
    }
    return table;
}

// Add symbol to table
void add_symbol(SymbolTable* table, const char* name, int type, int line) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol) {
        strcpy(symbol->name, name);
        symbol->type = type;
        symbol->scope_level = table->current_scope;
        symbol->line_declared = line;
        symbol->is_initialized = 0;
        
        // Add to beginning of list
        symbol->next = table->head;
        table->head = symbol;
    }
}

// Look up symbol by name
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Look up symbol in current scope only
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0 && 
            current->scope_level == table->current_scope) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Analyze AST semantically
int analyze_semantics(ASTNode* ast) {
    SymbolTable* table = init_symbol_table();
    int result = check_program(ast, table);
    free_symbol_table(table);
    return result;
}

// Check program node
int check_program(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    
    int result = 1;
    
    if (node->type == AST_PROGRAM) {
        // Check left child (statement)
        if (node->left) {
            result = check_statement(node->left, table) && result;
        }
        
        // Check right child (rest of program)
        if (node->right) {
            result = check_program(node->right, table) && result;
        }
    }
    
    return result;
}



// Check declaration node
int check_declaration(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_VARDECL) {
        return 0;
    }
    
    const char* name = node->token.lexeme;
    
    // Check if variable already declared in current scope
    Symbol* existing = lookup_symbol_current_scope(table, name);
    if (existing) {
        semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    
    // Add to symbol table
    add_symbol(table, name, TOKEN_INT, node->token.line);
    return 1;
}

// Check assignment node
int check_assignment(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_ASSIGN || !node->left || !node->right) {
        return 0;
    }
    
    const char* name = node->left->token.lexeme;
    
    // Check if variable exists
    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    
    // Check expression
    int expr_valid = check_expression(node->right, table);
    
    // Mark as initialized
    if (expr_valid) {
        symbol->is_initialized = 1;
    }
    
    return expr_valid;
}

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table){
    if (node->type != AST_BLOCK || !node) return 0;

    enter_scope(table);

    int valid = 1;
    ASTNode* stmt = node->left; // Assuming the left child is the list of statements

    while (stmt) {
        valid &= check_statement(stmt, table);
        stmt = stmt->right; // Move to next statement in block
    }

    exit_scope(table);
    return valid;
}

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table){
    if (node-> type != AST_IF || node-> type != AST_WHILE || !node) return 0;

    int valid = check_expression(node, table);

    // Ensure the expression is a boolean/integer type
    if (valid && node->type != AST_BINOP && node->type != AST_IDENTIFIER && node->type != AST_NUMBER) {
        semantic_error(SEM_ERROR_TYPE_MISMATCH, node->token.lexeme, node->token.line);
        return 0;
    }

    return valid;
}

void semantic_error(SemanticErrorType error, const char* name, int line) {
    printf("Semantic Error at line %d: ", line);
    
    switch (error) {
        case SEM_ERROR_UNDECLARED_VARIABLE:
            printf("Undeclared variable '%s'\n", name);
            break;
        case SEM_ERROR_REDECLARED_VARIABLE:
            printf("Variable '%s' already declared in this scope\n", name);
            break;
        case SEM_ERROR_TYPE_MISMATCH:
            printf("Type mismatch involving '%s'\n", name);
            break;
        case SEM_ERROR_UNINITIALIZED_VARIABLE:
            printf("Variable '%s' may be used uninitialized\n", name);
            break;
        case SEM_ERROR_INVALID_OPERATION:
            printf("Invalid operation involving '%s'\n", name);
            break;
        default:
            printf("Unknown semantic error with '%s'\n", name);
    }
}


int main() {
    const char* input = "int x;\n"
                        "x = 42;\n"
                        "if (x > 0) {\n"
                        "    int y;\n"
                        "    y = x + 10;\n"
                        "    print y;\n"
                        "}\n";
    
    printf("Analyzing input:\n%s\n\n", input);
    
    // Lexical analysis and parsing
    parser_init(input);
    ASTNode* ast = parse();
    
    printf("AST created. Performing semantic analysis...\n\n");
    
    // Semantic analysis
    int result = analyze_semantics(ast);
    
    if (result) {
        printf("Semantic analysis successful. No errors found.\n");
    } else {
        printf("Semantic analysis failed. Errors detected.\n");
    }
    
    // Clean up
    free_ast(ast);
    
    return 0;
}