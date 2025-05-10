#pragma once
#include <stdlib.h>
#include "tokeniser.h"
#include <stdbool.h>

enum ASTNodeType {
    // LITERALS
    NODE_NUMBER_LITERAL,
    NODE_TEXT_LITERAL,
    NODE_BOOL_LITERAL,

    NODE_BINARY_OPERATION,

    // VARIABLE OPERATIONS
    NODE_VARIABLE_DECLARATION,
    NODE_VARIABLE_ASSIGN,
    NODE_VARIABLE_REFERENCE,

    // FUNCTION OPERATIONS
    NODE_FUNCTION_DECLARATION,
    NODE_FUNCTION_CALL
};

struct Parameter {
    enum TokenType dataType;
    char* name;
};

struct ASTNode;

struct ASTBinaryOperation {
    char            operationChar;
    struct ASTNode* leftSide;
    struct ASTNode* rightSide;
};

struct ASTVariableDeclaration {
    char*           name;
    enum TokenType  dataType;
    struct ASTNode* node;
};

struct ASTVariableAssignment {
    char*           name;
    struct ASTNode* node;
};

struct ASTFunctionDeclaration {
    char*               name;
    struct Parameter* parameters;
    size_t parameterCount;
    struct ASTNodeList* codeBlock;
};

struct ASTFunctionCall {
    char*           name;
    // add function arguments here
};

struct ASTNode {
    enum ASTNodeType nodeType;
    size_t line;
    size_t column;
    union {
        double  numberValue;
        char*   textValue;
        bool    boolValue;
        struct  ASTBinaryOperation binary;
        struct  ASTVariableDeclaration varDeclaration;
        struct  ASTVariableAssignment varAssignment;
        struct  ASTFunctionDeclaration funcDeclaration;
        struct  ASTFunctionCall funcCall;
    } data;
};

struct ASTNodeList {
    struct ASTNode* *nodes;
    size_t          count;
    size_t          capacity;
};

void initAST(struct ASTNodeList* ast);
void appendAST(struct ASTNodeList* ast, struct ASTNode* node);
void destroyAST(struct ASTNodeList* ast);

