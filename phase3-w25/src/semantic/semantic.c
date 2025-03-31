/* semantic.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"
#include "../../include/semantic.h"

/* Declare external error count and print_errors() from parser.c */
extern int error_count;
extern void print_errors(void);

/* Function prototypes from semantic analysis */
SymbolTable* init_symbol_table();
void add_symbol(SymbolTable* table, const char* name, int type, int line);

Symbol* lookup_symbol(SymbolTable* table, const char* name);

int analyze_semantics(ASTNode* ast);

int check_declaration(ASTNode* node, SymbolTable* table);

int check_assignment(ASTNode* node, SymbolTable* table);

int check_block(ASTNode* node, SymbolTable* table);

int check_condition(ASTNode* node, SymbolTable* table);

int check_program(ASTNode* node, SymbolTable* table);

/* Scope management functions */
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

void exit_scope(SymbolTable* table){
    remove_symbols_in_current_scope(table);
    table->current_scope--;
}

void free_symbol_table(SymbolTable* table){
    Symbol* curr = table->head;
    while (curr) {
        Symbol* next = curr->next;
        free(curr);
        curr = next;
    }
    free(table);
}

/* Expression and type checking */
int check_expression(ASTNode* node, SymbolTable* table){
    if (!node) return 0;
    int valid = 1;
    switch (node->type) {
        case AST_NUMBER:
            /* Number literals are int by default. */
            break;
        case AST_IDENTIFIER: {
            Symbol* symbol = lookup_symbol(table, node->token.lexeme);
            if (!symbol) {
                semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
                valid = 0;
            } else if (!symbol->is_initialized) {
                semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->token.lexeme, node->token.line);
            }
            break;
        }
        case AST_BINOP: {
            int leftValid = check_expression(node->left, table);
            int rightValid = check_expression(node->right, table);
            valid = leftValid && rightValid;
            break;
        }
        case AST_FACTORIAL:
            if (!node->left) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "factorial", node->token.line);
                valid = 0;
            } else {
                valid = check_expression(node->left, table);
            }
            break;
        default:
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

/* Symbol table functions */
SymbolTable* init_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;
    }
    return table;
}

void add_symbol(SymbolTable* table, const char* name, int type, int line) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol) {
        strcpy(symbol->name, name);
        symbol->type = type;
        symbol->scope_level = table->current_scope;
        symbol->line_declared = line;
        symbol->is_initialized = 0;
        symbol->next = table->head;
        table->head = symbol;
    }
}

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

Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0 && current->scope_level == table->current_scope) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* High-level semantic analysis */
int analyze_semantics(ASTNode* ast) {
    SymbolTable* table = init_symbol_table();
    int result = check_program(ast, table);
    free_symbol_table(table);
    return result;
}

int check_program(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;
    int result = 1;
    if (node->type == AST_PROGRAM) {
        if (node->left) {
            result = check_statement(node->left, table) && result;
        }
        if (node->right) {
            result = check_program(node->right, table) && result;
        }
    }
    return result;
}

int check_declaration(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_VARDECL) {
        return 0;
    }
    const char* name = node->token.lexeme;
    Symbol* existing = lookup_symbol_current_scope(table, name);
    if (existing) {
        semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    add_symbol(table, name, TOKEN_INT, node->token.line);
    return 1;
}

int check_assignment(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_ASSIGN || !node->left || !node->right) {
        return 0;
    }
    const char* name = node->left->token.lexeme;
    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, name, node->token.line);
        return 0;
    }
    int expr_valid = check_expression(node->right, table);
    if (expr_valid) {
        symbol->is_initialized = 1;
    }
    return expr_valid;
}

int check_block(ASTNode* node, SymbolTable* table){
    if (!node || node->type != AST_BLOCK) return 0;
    enter_scope(table);
    int valid = 1;
    /* Iterate over block statements linked via the 'next' pointer */
    ASTNode* stmt = node->next;
    while (stmt) {
        valid &= check_statement(stmt, table);
        stmt = stmt->next;
    }
    exit_scope(table);
    return valid;
}

int check_condition(ASTNode* node, SymbolTable* table){
    if (!node) return 0;
    /* Simply validate the expression for the condition */
    return check_expression(node, table);
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

/* Main function */
int main(int argc, char* argv[]) {
    FILE* file;
    char* buffer = NULL;
    long file_size;
    const char* filename;
    
    if (argc < 2) {
        printf("Error: No input file specified.\n");
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    filename = argv[1];
    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    
    buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    if (fread(buffer, 1, file_size, file) != file_size) {
        printf("Error: Failed to read file %s\n", filename);
        free(buffer);
        fclose(file);
        return 1;
    }
    
    buffer[file_size] = '\0';
    fclose(file);
    
    printf("Analyzing input from file %s:\n%s\n\n", filename, buffer);
    parser_init(buffer);
    ASTNode* ast = parse();
    
    // Check for parse errors before semantic analysis, ensuring that parser.c is working
    if (error_count > 0) {
        printf("\nParsing failed with %d errors. Semantic analysis aborted.\n", error_count);
        print_errors();
        free_ast(ast);
        free(buffer);
        return 1;
    }
    
    printf("AST created. Performing semantic analysis...\n\n");
    
    int result = analyze_semantics(ast);
    if (result) {
        printf("Semantic analysis successful. No errors found.\n");
    } else {
        printf("Semantic analysis failed. Errors detected.\n");
    }
    
    free_ast(ast);
    free(buffer);
    
    return result;
}
