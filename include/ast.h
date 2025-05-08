#pragma once
#include <stdlib.h>

enum ASTNodeType {
    // LITERALS
    NODE_NUMBER_LITERAL,
    NODE_TEXT_LITERAL,

    NODE_BINARY_OPERATION,

    // VARIABLE OPERATIONS
    NODE_VARIABLE_DECLARATION,
    NODE_VARIABLE_ASSIGN,
    NODE_VARIABLE_REFERENCE,
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

