## DOCUMENTATION ##

## Modification Made
All modifications were made in order to enable the parser file to function correctly for all of it's use cases.
The list of modifications is as follows:
    The expanding of parse_statement() to allow for operator presidence and support for 
    Also allowed for Binary operations (+, -, *, /), Comparison operators (<, >, ==, !=) and Parenthesized expressions
    Modifications were also made to the lexer.c file to allow the use of new token types.
    The token types introduced were:
        TOKEN_WHILE,       // while keyword
        TOKEN_REPEAT,      // repeat keyword
        TOKEN_UNTIL,       // until keyword
        TOKEN_FACTORIAL,   // factorial keyword

    Adding of the following parse functions which parse the related types of statements:
        parse_factorial()   // Parses factorial statments in the form factorial(x);
        parse_block_statement() // Parses block statements int the form { block1; block2; }
        parse_print_statement() // Parses print statements in the form print x;
        parse_repeat_statement() // Parses repeat statements
        parse_while_statement() // Parses while statements
        parse_if_statement()  // Parses if statements
        parse_equality() // Parses multiple operators
        parse_comparison() // Parses comparison statments such as greater than and less than
        parse_additive() // Parses addition and subtraction
        parse_multiplicative() // Parses multiplication
        parse_primary() // Parses primary tokens

    Additional error types were added and handled:
        PARSE_ERROR_MISSING_PARENTHESES
        PARSE_ERROR_MISSING_CONDITION_STATEMENT
        PARSE_ERROR_MISSING_BLOCK_BRACES
        PARSE_ERROR_INVALID_OPERATOR
        PARSE_ERROR_FUNCTION_CALL
    These were all added to give more specific error indicators to the user when programming.

    Error recovery and line and column tracking was added.
    Support for factorial and block scoping was added.
    

##Grammar rules
For the most part the grammar rules follow the grammar of the C programming language.
There are a few exceptions as follows:
For printing, it follows the format of "print x;" where x is a variable.
For the newly introduced factorial function it follows the format "factorial(x);" Where x is a number or variable.
Nested block statements are not allowed within this grammar.

## Error handling strategy
Graceful error handling, parsing after encountering errors when possible.
Multiple errors collected during a single parse pass. 
Implemented robust recovery from parsing failures.
Parser implements a  panic mode recovery where it detects an error at a specific point in the parse, 
records the error with contextual information and attempts to skip ahead to a reliable parsing point to continue.
The parser handles the following types of errors and raises the corresponding error type when appropriate:
    PARSE_ERROR_MISSING_SEMICOLON
    PARSE_ERROR_MISSING_IDENTIFIER
    PARSE_ERROR_MISSING_EQUALS
    PARSE_ERROR_INVALID_EXPRESSION
    PARSE_ERROR_MISSING_PARENTHESES
    PARSE_ERROR_MISSING_CONDITION_STATEMENT
    PARSE_ERROR_MISSING_BLOCK_BRACES
    PARSE_ERROR_INVALID_OPERATOR
    PARSE_ERROR_FUNCTION_CALL
    PARSE_ERROR_UNEXPECTED_TOKEN


## AST structure
The AST has the following possible types for it's nodes:
    AST_PROGRAM,        // Program node
    AST_VARDECL,        // Variable declaration (int x)
    AST_ASSIGN,         // Assignment (x = 5)
    AST_PRINT,          // Print statement
    AST_NUMBER,         // Number literal
    AST_IDENTIFIER,     // Variable name
    AST_IF,             // If statement 
    AST_WHILE,          // While statement
    AST_REPEAT,         // Repeat statement
    AST_BLOCK,          // Code block?
    AST_BINOP,          // Binary operator
    AST_FACTORIAL,      // Factorial function

The AST structure has two nodes branching off of it, left, right and next.
Left and right create a child and next links it to the next statement.

## Test cases
For the test cases there are two different files.
One file is valid and it is called "input_valid.txt" and the other is invalid called "input_invalid.txt".
Both these files are located within the "test" folder.
They can be run as followed from within the phase2-w25 folder:
    make clean && make
    ./build/compiler ./test/input_valid.txt
    ./build/compiler ./test/input_invalid.txt

The expected output for the input_valid.txt is:

  Parsing file: ./test/input_valid.txt

  File parsed successfully!
  Abstract Syntax Tree:
  Program
  VarDecl: int
    Identifier: x
  VarDecl: int
    Identifier: y
  VarDecl: int
    Identifier: result
  Assign
    Identifier: x
    Number: 42
  Assign
    Identifier: y
    Number: 10
  If
    BinaryOp: ==
      Identifier: x
      Number: 42
    Block
    Assign
      Identifier: x
      BinaryOp: -
        Identifier: x
        Number: 1
    Print
      Identifier: x
  Print
    Identifier: x
  Print
    Identifier: y
  Print
    Identifier: result

---
The expected ouput for the input_invalid.txt is:

  Parsing file: ./test/input_invalid.txt

  5 errors found:
  Error 1:5: Missing semicolon after 'x'
  Error 2:5: Unexpected '49'
  Error 4:1: Missing identifier after 'int'
  Error 7:1: Missing identifier after 'print'
  Error 10:10: Missing parentheses for '42'