#pragma once
#include "ast.h"

enum ValueType {
    VALUE_NUMBER,
    VALUE_TEXT,
    VALUE_FUNCTION,
    VALUE_FUNCTION_RETURN,
    VALUE_BOOL,
};

struct Value {
    enum ValueType type;
    union {
        double  number;
        char*   text;
        bool    boolVal;
        struct ASTNodeList* nodeList; 
    } data;
};

// Hashmap for environment, to store values

struct Entry {
    char            *key;
    struct Value    value;
    
    // collision resolution, linked list strat
    struct Entry*   next;
};

struct Environment {
    struct Entry*   *bucket;
    size_t          bucket_count;
};

// Environment
void createEnvironment(struct Environment* env);
void freeEnvironment(struct Environment* env);

struct Value getValue(struct Environment* env, const char* key);
void setValue(struct Environment* env, const char* key, struct Value val);

// Evaluation
struct Value createNumberValue(double num);
struct Value createTextValue(char* str);
struct Value createBoolValue(bool state);
struct Value evaluateASTNode(const struct ASTNode* node, struct Environment* env);
void evaluateAST(const struct ASTNodeList* astList, struct Environment* env);
