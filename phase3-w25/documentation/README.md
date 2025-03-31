## DOCUMENTATION ##

## Modifications Made
From the previous phase, the array data type was introudced. Introducing this data type requires the checking of more semantic errors.
The semantic analyzer was created to check for the following:
    Variable Declaration Before Use: Ensure all variables are declared before they are used.
    Type Checking: Ensure operations and assignments involve compatible data types.
    Scope Management: Validate that variables are used within their declared scope.
    Array Bound Checking: Detect cases where an array index is out of bounds.
    Use of Uninitialized Variables: Ensure variables are assigned values before use.
    Arithmetic Errors: Detect division by zero and similar runtime errors.
    Loop Condition Validation: Ensure loop conditions are of the correct type (boolean).
    Duplicate Declarations: Ensure variables are not declared multiple times within the same scope.

In order to check for these the following functions were implemented:
    enter_scope
    remove_symbols_in_current_scope
    exit_scope
    free_symbol_table
    check_expression
    check_statement
    init_symbol_table
    add_symbol
    lookup_symbol
    lookup_symbol_current_scope
    analyze_semantics
    check_program
    check_declaration
    check_assignment
    check_block
    check_condition
    check_array_access
    check_array_declaration

## Error handling approach
In order to check for errors the semantic_error function is used.
Graceful error handling, parsing after encountering errors when possible.
Multiple errors collected during a single parse pass. 
Implemented robust recovery from parsing failures.
Parser implements a  panic mode recovery where it detects an error at a specific point in the parse, 
records the error with contextual information and attempts to skip ahead to a reliable parsing point to continue.
Based on what type of error is detected, several different error types can be thrown, they are:
    SEM_ERROR_UNDECLARED_VARIABLE
    SEM_ERROR_REDECLARED_VARIABLE
    SEM_ERROR_TYPE_MISMATCH
    SEM_ERROR_UNINITIALIZED_VARIABLE
    SEM_ERROR_INVALID_OPERATION
    SEM_ERROR_DIVIDE_BY_ZERO
    SEM_ERROR_INVALID_ARRAY_SIZE
    SEM_ERROR_NOT_AN_ARRAY
    SEM_ERROR_ARRAY_INDEX_OUT_OF_BOUNDS
    SEM_ERROR_ARRAY_ASSIGNMENT
    And a defalt case for unknown errors.

## Grammar rules
For the most part the grammar rules follow the grammar of the C programming language.
There are a few exceptions as follows:
For printing, it follows the format of "print x;" where x is a variable.
For the newly introduced factorial function it follows the format "factorial(x);" Where x is a number or variable.
The newly added Repeat-Until Loop, that executes statements repeatedly until a specified condition is met.
Nested block statements are not allowed within this grammar.

## Test cases

## The expected output for the input_valid.txt is:

Analyzing input from file src/test/input_valid.txt:
int x;
int y;
int result;

x = 42;
y = 10;

if (x == 42) {
    x = x - 1;
    print x;
}

print x;
print y;
print result;

AST created. Performing semantic analysis...

Semantic analysis successful. No errors found.

## The expected ouput for the input_invalid.txt is:

