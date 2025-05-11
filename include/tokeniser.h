#pragma once    
#include <stddef.h>
#include <stdlib.h>

enum TokenType {

    // datatypes
    TEXT_TYPE, NUMBER_TYPE, BOOLEAN_TYPE,

    // identifier + literals
    IDENTIFIER, TEXT, NUMBER, TRUE, FALSE,

    // operators
    EQUAL, STAR, SLASH, PLUS, MINUS, LESS_THAN, MORE_THAN,
    EQUALITY_OPERATOR, LESSER_EQUAL, GREATER_EQUAL, NOT_EQUAL,

    // punctuation
    SEMICOLON, LEFT_PAREN, RIGHT_PAREN, COMMA, LEFT_CURLY, RIGHT_CURLY,

    // keywords
    COMMENT, FUNCTION_DECLARATION, IF_DECLARATION, LOOP_DECLARATION, END_OF_FILE, 
};

union uLiteral {
    double number_value;
    char   *text_value;
};

struct Token {
    enum  TokenType tokenType;
    char            *lexeme;
    size_t          length;

    size_t          line;
    size_t          column;

    union uLiteral  literal; 
};

struct TokenList {
    struct Token  *data;
    size_t        count;
    size_t        capacity;
};


struct Token createToken(enum TokenType tokenType, const char* lexeme, size_t length, size_t line, size_t column, union uLiteral literal);
struct TokenList tokenise(const char* sourceCode);

// Token list manager
void initTokenList(struct TokenList *tokenList);
void appendTokenList(struct TokenList *tokenList, struct Token token);
void destroyTokenList(struct TokenList *tokenList);

