How to test:
1. Run: make
2. Run: ./build/compiler correct_input.txt
3. Run: ./build/compiler incorrect_input.txt

Modifications made:
Instead of hardcoding the test cases as strings, the design of the analyzer was made so that a .txt file can be passed in.
This is to make it easier to design robust test cases that cover all of the needs of our program.

List of changes made:
Added handling for deletion of single-line comments and multi-line comments.
Added handling of keywords and identifiers.
Added handling of string literals.
Added handling of additional operators inluding comparison operators (ex. >,<,>=) and aditional aritmetic operators (++, *, /).
Added handling of delimeters.
Added handling of grammatical elements.

New tokens, grammar rules, or semantic operations added:
Added the followoiing token types: 
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_DELIMITER,
    TOKEN_GRAMMAR

New error signals or handling mechanisms:
The new errors added are ERROR_TOO_LONG, which occurs when a string is made that is over 247 characters long.


Correct output

> ./build/compiler correct_input.txt
Analyzing: 
123 + 456 - 789;
int x = 42;
y = x + 10;
if (a == b){
    printf("HELLO WORLD")
}

Token: NUMBER     | Lexeme: 123             | Line: 1
Token: OPERATOR   | Lexeme: +               | Line: 1
Token: NUMBER     | Lexeme: 456             | Line: 1
Token: OPERATOR   | Lexeme: -               | Line: 1
Token: NUMBER     | Lexeme: 789             | Line: 1
Token: DELIMITER  | Lexeme: ;               | Line: 1
Token: KEYWORD    | Lexeme: int             | Line: 1
Token: IDENTIFIER | Lexeme: x               | Line: 2
Token: OPERATOR   | Lexeme: =               | Line: 2
Token: NUMBER     | Lexeme: 42              | Line: 2
Token: DELIMITER  | Lexeme: ;               | Line: 2
Token: IDENTIFIER | Lexeme: y               | Line: 2
Token: OPERATOR   | Lexeme: =               | Line: 3
Token: IDENTIFIER | Lexeme: x               | Line: 3
Token: OPERATOR   | Lexeme: +               | Line: 3
Token: NUMBER     | Lexeme: 10              | Line: 3
Token: DELIMITER  | Lexeme: ;               | Line: 3
Token: KEYWORD    | Lexeme: if              | Line: 3
Token: PUNC       | Lexeme: (               | Line: 4
Token: IDENTIFIER | Lexeme: a               | Line: 4
Token: OPERATOR   | Lexeme: ==              | Line: 4
Token: IDENTIFIER | Lexeme: b               | Line: 4
Token: PUNC       | Lexeme: )               | Line: 4
Token: PUNC       | Lexeme: {               | Line: 4
Token: IDENTIFIER | Lexeme: printf          | Line: 4
Token: PUNC       | Lexeme: (               | Line: 5
Token: STRING     | Lexeme: HELLO WORLD     | Line: 5
Token: PUNC       | Lexeme: )               | Line: 5
Token: PUNC       | Lexeme: }               | Line: 5
Token: EOF        | Lexeme: EOF             | Line: 6

Incorrect output

> ./build/compiler incorrect_input.txt 
Analyzing: 
123 ++ 456
x@ = 10
123 +- 456
"Unterminated string    // Error case

Token: NUMBER     | Lexeme: 123             | Line: 1
Token: OPERATOR   | Lexeme: ++              | Line: 1
Token: NUMBER     | Lexeme: 456             | Line: 1
Token: IDENTIFIER | Lexeme: x               | Line: 1
Lexical Error at line 2: Invalid character '@'
Token: OPERATOR   | Lexeme: =               | Line: 2
Token: NUMBER     | Lexeme: 10              | Line: 2
Token: NUMBER     | Lexeme: 123             | Line: 2
Token: OPERATOR   | Lexeme: +               | Line: 3
Token: OPERATOR   | Lexeme: -               | Line: 3
Token: NUMBER     | Lexeme: 456             | Line: 3
Token: STRING     | Lexeme: Unterminated string    // Error case | Line: 3
Token: EOF        | Lexeme: EOF             | Line: 4