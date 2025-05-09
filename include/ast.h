#pragma once
#include <stdlib.h>

enum ASTNodeType {
    // LITERALS
    NODE_NUMBER_LITERAL,
    NODE_TEXT_LITERAL,
    NODE_BOOL_TRUE,
    NODE_BOOL_FALSE,

    NODE_BINARY_OPERATION,

    // VARIABLE OPERATIONS
    NODE_VARIABLE_DECLARATION,
    NODE_VARIABLE_ASSIGN,
    NODE_VARIABLE_REFERENCE,

    // FUNCTION OPERATIONS
    NODE_FUNCTION_DECLARATION,
    NODE_FUNCTION_CALL
};

struct ASTNode;

struct ASTBinaryOperation {
    char            operationChar;
    struct ASTNode* leftSide;
    struct ASTNode* rightSide;
};

struct ASTVariableDeclaration {
    char*           name;
    struct ASTNode* node;
};

struct ASTVariableAssignment {
    char*           name;
    struct ASTNode* node;
};

struct ASTFunctionDeclaration {
    char*               name;
    // add function parameters
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

