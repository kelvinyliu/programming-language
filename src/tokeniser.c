#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "../include/tokeniser.h"

void initTokenList(struct TokenList *tokenList) {
    tokenList->data = malloc(sizeof(struct Token) * 10);
    
    if (!tokenList->data) {
        printf("Error malloc in init of token list.\n");
        abort();
    }
    
    tokenList->count = 0;
    tokenList->capacity = 10;
}

void appendTokenList(struct TokenList *tokenList, struct Token token) {
    if (tokenList->count >= tokenList->capacity) {
        size_t newCapacity = tokenList->capacity * 2;
        struct Token *newData = realloc(tokenList->data, sizeof(struct Token) * newCapacity);
        
        // if realloc fails
        if (!newData) {
            printf("Realloc token list failed...\n");
            abort();
        }

        tokenList->data = newData;
        tokenList->capacity = newCapacity;
    }
    tokenList->data[tokenList->count] = token;
    tokenList->count++;
}

void destroyTokenList(struct TokenList *tokenList) {
    if (!tokenList || !tokenList->data) return;
    
    //for(size_t i = 0; i < tokenList->count; i++) {
    //  struct Token * currToken = &tokenList->data[i];

    //  if (currToken->literal.text_value) {
    //      free(currToken->literal.text_value);
    //  }
    //}

    free(tokenList->data);

    tokenList->data = NULL;
    tokenList->count = 0;
    tokenList->capacity = 0;
}

struct Token createToken(enum TokenType tokenType, const char* lexeme, size_t length, size_t line, size_t column, union uLiteral literal) {
    struct Token newToken;
    newToken.line = line;
    newToken.column = column;
    newToken.tokenType = tokenType;
    newToken.lexeme = (char *)lexeme;
    newToken.length = length;
    newToken.literal = literal;

    return newToken;
}

void nextCharacter(const char *sourceCode, size_t *i, size_t *line, size_t *column) {
    if (sourceCode[*i] == '\n') {
        (*line)++;
        *column = 1;
    } else {
        (*column)++;
    }
    (*i)++;
}

struct TokenList tokenise(const char* sourceCode) {
    struct TokenList tokens;
    initTokenList(&tokens);

    size_t i = 0;
    size_t line = 1;
    size_t column = 1;
    size_t startLine;
    size_t startColumn;

    while (true) {
        
        char c = sourceCode[i];
        startLine = line;
        startColumn = column;

        if (c == '\0') {
            union uLiteral literal;
            literal.number_value = 0;
            struct Token eofToken = createToken(END_OF_FILE, 
                    &sourceCode[i], 0, startLine, startColumn, literal);
            appendTokenList(&tokens, eofToken);
            break;
        }
        
        // whitespace
        if (isspace((unsigned char) c)) {
            nextCharacter(sourceCode, &i, &line, &column);
            continue;
        }
        
        // comments -> ignore like white space
        if (c == '/' && sourceCode[i+1] == '*') {
            // 2 nextchars because comments start with /*
            nextCharacter(sourceCode, &i, &line, &column); // get /
            nextCharacter(sourceCode, &i, &line, &column); // get *
            while (sourceCode[i] != '\0') {
                if (sourceCode[i] == '*' && sourceCode[i+1] == '/') {
                    nextCharacter(sourceCode, &i, &line, &column); // get *
                    nextCharacter(sourceCode, &i, &line, &column); // get /
                    break;
                }
                nextCharacter(sourceCode, &i, &line, &column);
            }
            continue;
        }

        // literals
        // number
        if (isdigit((unsigned char) c)) {
            size_t startIndex = i;

            // set i to end of number
            bool hasDecimalPoint = false;
            while (isdigit((unsigned char) sourceCode[i]) || (!hasDecimalPoint && sourceCode[i] == '.')) { 
                if (sourceCode[i] == '.') hasDecimalPoint = true;
                nextCharacter(sourceCode, &i, &line, &column);
            }
            size_t numSize = i - startIndex;
            
            char numberBuffer[numSize+1];
            memcpy(numberBuffer, &sourceCode[startIndex], numSize);
            numberBuffer[numSize] = '\0';

            double num = strtod(numberBuffer, NULL);

            union uLiteral literal;

            literal.number_value = num;
            struct Token numberToken = createToken(NUMBER, &sourceCode[startIndex], numSize, startLine, startColumn, literal);
            appendTokenList(&tokens, numberToken);
            continue;
        }

        // identifiers / keywords
        if (isalnum((unsigned char) c) || sourceCode[i] == '_') {
            size_t startIndex = i;
            while (isalnum((unsigned char) sourceCode[i]) || sourceCode[i] == '_') {
                nextCharacter(sourceCode, &i, &line, &column);
            }
            size_t textSize = i - startIndex;

            char buff[textSize + 1];
            memcpy(buff, &sourceCode[startIndex], textSize);
            buff[textSize] = '\0';

            // Check what type of token it is
            // default to identifier if not detect any keywords
            enum TokenType tokenType = IDENTIFIER;
            if (textSize == 6 && strncmp(&sourceCode[startIndex], "number", 6) == 0)
                tokenType = NUMBER_TYPE;
            else if (textSize == 4 && strncmp(&sourceCode[startIndex], "text", 4) == 0)
                tokenType = TEXT_TYPE;


            union uLiteral literal;
            literal.text_value = 0;

            struct Token textToken = createToken(tokenType, &sourceCode[startIndex], textSize, startLine, startColumn, literal);
            appendTokenList(&tokens, textToken);
            continue;
        }
        
        // " literals
        if (c == '"') {
            nextCharacter(sourceCode, &i, &line, &column);
            size_t startIndex = i;
            while (sourceCode[i] != '"') {
                nextCharacter(sourceCode, &i, &line, &column);
            }
            
            size_t stringLength = i - startIndex;
            //char *stringValue = malloc(stringLength + 1);
            //memcpy(stringValue, &sourceCode[startIndex], stringLength);
            //stringValue[stringLength] = '\0';

            union uLiteral literal;
            literal.text_value = 0;

            struct Token stringToken = createToken(TEXT, &sourceCode[startIndex - 1], stringLength + 2, startLine, startColumn, literal);
            appendTokenList(&tokens, stringToken);
            nextCharacter(sourceCode, &i, &line, &column);
            continue;
        }
        
        // single character token
        switch (c) {
            case '=': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token equalToken = createToken(EQUAL, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, equalToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case ';': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token semiColonToken = createToken(SEMICOLON, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, semiColonToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case '*': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token starToken = createToken(STAR, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, starToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case '/': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token slashToken = createToken(SLASH, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, slashToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case '(': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token leftParenToken = createToken(LEFT_PAREN, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, leftParenToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case ')': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token rightParenToken = createToken(RIGHT_PAREN, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, rightParenToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case ',': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token commaToken = createToken(COMMA, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, commaToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case '+': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token plusToken = createToken(PLUS, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, plusToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            case '-': {
                union uLiteral literal;
                literal.number_value = 0;
                struct Token minusToken = createToken(MINUS, &sourceCode[i], 1, startLine, startColumn, literal);
                appendTokenList(&tokens, minusToken);
                nextCharacter(sourceCode, &i, &line, &column);
                continue;
            }
            default:
                nextCharacter(sourceCode, &i, &line, &column);
                break;
        }
        
        printf("cannot tokenise?"); 

    }
    return tokens;
}
