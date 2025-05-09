#include "../include/evaluator.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define DEFAULT_BUCKET_COUNT 1087

unsigned long hash(const char* val) {
    unsigned long h = 5381;
    while (*val) {
        h = ((h << 5) + h) + (unsigned char)*val;
        val++;
    }
    return h;
}

void createEnvironment(struct Environment* env) {
    env->bucket_count = DEFAULT_BUCKET_COUNT;
    env->bucket = calloc(env->bucket_count, sizeof(struct Entry*));

    if (!env->bucket) {
        printf("Error calloc while creating environment.\n");
        exit(1);
    }
}

void freeEnvironment(struct Environment* env) {
    for(size_t i = 0; i < env->bucket_count; i++) {
        struct Entry* e = env->bucket[i];

        while (e) {
            struct Entry* next = e->next;
            free(e->key);
            // free char* if text
            if (e->value.type == VALUE_TEXT) {
                free(e->value.data.text);
            }

            free(e);
            e = next;
        }
    }
    free(env->bucket);
    env->bucket = NULL;
}

struct Value getValue(struct Environment* env, const char* key) {
    unsigned long h = hash(key) % env->bucket_count;
    struct Entry* e = env->bucket[h];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            return e->value;
        }
        // if collision
        e = e->next;
    }
    printf("Trying to retrieve value from key which does not exist in environment.\n");
    exit(1);
}

void setValue(struct Environment* env, const char* key, struct Value val) {
    unsigned long h = hash(key) % env->bucket_count;
    struct Entry* e = env->bucket[h];

    // if entry exists
    while (e) {
        if (strcmp(e->key, key) == 0) {
            // override data, free previous char*
            if (e->value.type == VALUE_TEXT) {
                free(e->value.data.text);
            }
            e->value = val;
            return;
        }
        e = e->next;
    }

    struct Entry* newEntry = malloc(sizeof (struct Entry));
    newEntry->key = strdup(key);
    newEntry->value = val;
    newEntry->next = env->bucket[h];

    env->bucket[h] = newEntry;
}

struct Value createNumberValue(double num) {
    struct Value val;
    val.type = VALUE_NUMBER;
    val.data.number = num;
    return val;
}

struct Value createTextValue(char* str) {
    struct Value val;
    val.type = VALUE_TEXT;
    val.data.text = str;
    return val;
}

struct Value createBoolValue(bool state) {
    struct Value val;
    val.type = state ? VALUE_TRUE : VALUE_FALSE;
    val.data.text = state ? "true" : "false";
    return val;
}

struct Value createFunctionValue(struct ASTNodeList* nodeList) {
    struct Value val;
    val.type = VALUE_FUNCTION;
    val.data.nodeList = nodeList;
    return val;
}

struct Value evaluateASTNode(const struct ASTNode* node, struct Environment* env) {
    switch (node->nodeType) {
        case NODE_NUMBER_LITERAL:
            return createNumberValue(node->data.numberValue);
        case NODE_TEXT_LITERAL:
            return createTextValue(strdup(node->data.textValue));
        case NODE_BOOL_FALSE:
            return createBoolValue(false);
        case NODE_BOOL_TRUE:
            return createBoolValue(true);
        case NODE_VARIABLE_REFERENCE:
            return getValue(env, node->data.textValue);
        case NODE_BINARY_OPERATION: 
            {
                struct Value left = evaluateASTNode(node->data.binary.leftSide, env);
                struct Value right = evaluateASTNode(node->data.binary.rightSide, env);
                char op = node->data.binary.operationChar;

                if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER) {
                    double leftNum = left.data.number;
                    double rightNum = right.data.number;

                    double res;
                    switch (op) {
                        case '+': 
                            res = leftNum + rightNum;
                            break;
                        case '-':
                            res = leftNum - rightNum;
                            break;
                        case '*':
                            res = leftNum * rightNum;
                            break;
                        case '/':
                            {
                                if (rightNum == 0) {
                                    printf("Cannot divide by zero. line %zu column %zu\n", node->line, node->column);
                                    exit(1);
                                }
                                res = leftNum / rightNum;
                                break;
                            }
                        default:
                            printf("Unknown operator.\n");
                            exit(1);
                    }
                    return createNumberValue(res);
                }
                printf("Unable to '+'?\n");
                exit(1);
            }
        case NODE_VARIABLE_DECLARATION: 
            {
                struct Value val = evaluateASTNode(node->data.varDeclaration.node, env);
                setValue(env, node->data.varDeclaration.name, val);
                return val;
            }
        case NODE_VARIABLE_ASSIGN:
            {
                struct Value val = evaluateASTNode(node->data.varAssignment.node, env);
                setValue(env, node->data.varAssignment.name, val);
                return val;
            }
        case NODE_FUNCTION_DECLARATION:
            {
                struct Value val = createFunctionValue(node->data.funcDeclaration.codeBlock);
                setValue(env, node->data.funcDeclaration.name, val);
                return val;
            }
        case NODE_FUNCTION_CALL:
            {
                struct Value val = getValue(env, node->data.funcCall.name);
                evaluateAST(val.data.nodeList, env);
                // could change later to get a return
                return createNumberValue(0);

            }
        default:
            printf("Unhandled node.\n");
            exit(1);
    }
}

void evaluateAST(const struct ASTNodeList* astList, struct Environment* env) {
    for (size_t i = 0; i < astList->count; i++) {
        struct ASTNode* node = astList->nodes[i];
        struct Value val = evaluateASTNode(node, env);

        // TEMP: print method without language builtins.
        if (node->nodeType != NODE_VARIABLE_DECLARATION && node->nodeType != NODE_VARIABLE_ASSIGN && node->nodeType != NODE_FUNCTION_CALL) {
            if (val.type == VALUE_NUMBER) {
                printf("%g\n", val.data.number);
            } else if (val.type == VALUE_TEXT || val.type == VALUE_FALSE || val.type == VALUE_TRUE) {
                printf("%s\n", val.data.text);
            }
        }
    }
}
