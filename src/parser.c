#include "parser.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct ASTNode* createBinaryNode(char op, struct ASTNode* left, struct ASTNode* right) {
    struct ASTNode* n = malloc(sizeof(struct ASTNode));
    n->nodeType = NODE_BINARY_OPERATION;
    n->data.binary.operationChar = op;
    n->data.binary.leftSide = left;
    n->data.binary.rightSide = right;
    
    return n;
}

struct ASTNode* parsePrimary(struct TokenList* tokens, size_t* index) {
    struct Token* token = &tokens->data[*index];

    if (token->tokenType == NUMBER) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->nodeType = NODE_NUMBER_LITERAL;
        node->data.numberValue = token->literal.number_value;
        
        (*index)++;
        return node;
    }

    if (token->tokenType == IDENTIFIER) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->nodeType = NODE_VARIABLE_REFERENCE;
        node->data.textValue = strndup(token->lexeme, token->length);
        
        (*index)++;
        return node;
    }

    if (token->tokenType == TEXT) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->nodeType = NODE_TEXT_LITERAL;
        node->data.textValue = strndup(token->lexeme, token->length);

        (*index)++;
        return node;
    }

    // CANNOT PARSE PRIMARY
    // LOG ERROR
    return NULL;
}

struct ASTNode* parseFactor(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];

    // if has parens, parse entire; else its a literal or identifier.
    if (token.tokenType == LEFT_PAREN) {
        (*index)++;
        struct ASTNode* subExpr = parseExpression(tokens,index);
        if (tokens->data[*index].tokenType != RIGHT_PAREN) {
            // ERROR;
            printf("Expected ')'\n");
            exit(1);
        }
        (*index)++;
        return subExpr;
    }

    return parsePrimary(tokens, index);
}

struct ASTNode* parseTerm(struct TokenList* tokens, size_t* index) {
    struct ASTNode* node = parseFactor(tokens, index);

    while (true) {
        struct Token token = tokens->data[*index];
        if (token.tokenType == STAR || token.tokenType == SLASH) {
            char op = token.lexeme[0];
            (*index)++; 
            struct ASTNode* rightSide = parseFactor(tokens, index);

            node = createBinaryNode(op, node, rightSide);
        } else {
            break;
        }
    }
    return node;
}

struct ASTNode* parseExpression(struct TokenList* tokens, size_t* index) {
    struct ASTNode* node = parseTerm(tokens, index);

    while (true) {
        struct Token token = tokens->data[*index];
        if (token.tokenType == PLUS || token.tokenType == MINUS) {
            char op = token.lexeme[0];
            (*index)++;
            struct ASTNode* rightSide = parseTerm(tokens, index);

            node = createBinaryNode(op, node, rightSide);
        } else {
            break;
        }
    }
    return node;
}

struct ASTNode* parseDeclaration(struct TokenList* tokens, size_t* index) {
    // skip parsing datatype for now
    (*index)++;

    struct Token token = tokens->data[*index];
    if (token.tokenType != IDENTIFIER) {
        // ERROR;
        exit(1);
    }

    // get var name
    char* name = strndup(token.lexeme, token.length);
    (*index)++;

    // get =
    if (tokens->data[*index].tokenType != EQUAL) {
        // ERROR;
        exit(1);
    }
    (*index)++;
    
    struct ASTNode* init = parseExpression(tokens, index);

    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';'.\n");
        exit(1);
    }
    (*index)++;

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->nodeType = NODE_VARIABLE_DECLARATION;
    node->data.varDeclaration.name = name;
    node->data.varDeclaration.node = init;
    return node;
}

struct ASTNode* parseAssignment(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];
    
    // get var name
    char* name = strndup(token.lexeme, token.length);
    (*index)++;
    
    // get = 
    if (tokens->data[*index].tokenType != EQUAL) {
        printf("Expected '=', in assignment.\n");
        exit(1);
    }
    (*index)++;

    struct ASTNode* assignValue = parseExpression(tokens, index);

    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';'\n");
        exit(1);
    }
    (*index)++;


    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->nodeType = NODE_VARIABLE_ASSIGN;
    node->data.varAssignment.name = name;
    node->data.varAssignment.node = assignValue;
    
    return node;
}

struct ASTNode* parseStatement(struct TokenList* tokens, size_t* index) {
    enum TokenType tokenType = tokens->data[*index].tokenType;

    // DECLARATION
    if (tokenType == TEXT_TYPE || tokenType == NUMBER_TYPE) {
        return parseDeclaration(tokens, index);
    }

    // ASSIGNMENT
    if (tokenType == IDENTIFIER && tokens->data[*index + 1].tokenType == EQUAL) {
        return parseAssignment(tokens, index);
    }

    // EXPRESSION
    struct ASTNode* expression = parseExpression(tokens, index);
    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';'.\n");
        exit(1);
    }
    (*index)++;
    return expression;
}

struct ASTNodeList parseProgram(const char* sourceCode) {
    struct TokenList tokens = tokenise(sourceCode);
    struct ASTNodeList ast;
    initAST(&ast);

    size_t i = 0;
    while (tokens.data[i].tokenType != END_OF_FILE) {
        struct ASTNode* statement = parseStatement(&tokens, &i);
        appendAST(&ast, statement);
    }

    destroyTokenList(&tokens);
    return ast;
}
