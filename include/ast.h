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
    NODE_FUNCTION_CALL,

    // IF
    NODE_IF_STATEMENT,

    // LOOPS
    NODE_LOOP_STATEMENT,
};

enum BinaryOperatorTypes {
    BIN_OP_PLUS, BIN_OP_MINUS, BIN_OP_STAR, BIN_OP_SLASH, BIN_OP_EQUALITY,
    BIN_OP_LESS, BIN_OP_GREATER, BIN_OP_LESSER_EQUAL, BIN_OP_GREATER_EQUAL
};

struct Parameter {
    enum TokenType dataType;
    char* name;
};

struct ASTNode;

struct ASTBinaryOperation {
    enum BinaryOperatorTypes    operationChar;
    struct ASTNode*             leftSide;
    struct ASTNode*             rightSide;
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
    char*               name;
    struct ASTNode**    arguments;
    size_t              argumentCount;
};

struct ASTIfStatement {
    struct ASTNode* condition;
    struct ASTNodeList* conditionTrueBlock;
};

struct ASTLoopStatement {
    struct ASTNode* loopCount;
    struct ASTNodeList* loopCodeBlock;
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
        struct  ASTIfStatement ifStatement;
        struct  ASTLoopStatement loopStatement;
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

