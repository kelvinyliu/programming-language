#pragma once

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
    union {
        double  numberValue;
        char*   textValue;
        struct  ASTVariableDeclaration;
        struct  ASTVariableAssignment;
    } data;
};
