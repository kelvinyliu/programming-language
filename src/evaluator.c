#include "../include/evaluator.h"
#include "../include/typeHelper.h"
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

struct Value* getValue(struct Environment* env, const char* key) {
    unsigned long h = hash(key) % env->bucket_count;
    struct Entry* e = env->bucket[h];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            return &e->value;
        }
        // if collision
        e = e->next;
    }
    return NULL;
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
    val.type = VALUE_BOOL;
    val.data.boolVal = state ? true : false;
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
        case NODE_BOOL_LITERAL:
            return createBoolValue(node->data.boolValue);
        case NODE_VARIABLE_REFERENCE:
            {
                struct Value* val = getValue(env, node->data.textValue);
                if (!val) {
                    printf("Variable reference %s does not exist, line %zu\n", node->data.textValue, node->line);
                    exit(1);
                }
                return *val;
            }
        case NODE_BINARY_OPERATION: 
            {
                struct Value left = evaluateASTNode(node->data.binary.leftSide, env);
                struct Value right = evaluateASTNode(node->data.binary.rightSide, env);
                enum BinaryOperatorTypes op = node->data.binary.operationChar;

                if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER) {
                    double leftNum = left.data.number;
                    double rightNum = right.data.number;

                    double res;
                    switch (op) {
                        case BIN_OP_PLUS: 
                            res = leftNum + rightNum;
                            break;
                        case BIN_OP_MINUS:
                            res = leftNum - rightNum;
                            break;
                        case BIN_OP_STAR:
                            res = leftNum * rightNum;
                            break;
                        case BIN_OP_SLASH:
                            {
                                if (rightNum == 0) {
                                    printf("Cannot divide by zero. line %zu column %zu\n", node->line, node->column);
                                    exit(1);
                                }
                                res = leftNum / rightNum;
                                break;
                            }
                        case BIN_OP_EQUALITY:
                            return createBoolValue(leftNum == rightNum);
                        case BIN_OP_LESS:
                            return createBoolValue(leftNum < rightNum);
                        case BIN_OP_GREATER:
                            return createBoolValue(leftNum > rightNum);
                        case BIN_OP_LESSER_EQUAL:
                            return createBoolValue(leftNum <= rightNum);
                        case BIN_OP_GREATER_EQUAL:
                            return createBoolValue(leftNum >= rightNum);
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
                struct Value* previousData = getValue(env, node->data.varDeclaration.name);
                // if get data does exists
                if (previousData) {
                    printf("Variable with name '%s' already exists, therefore cannot declare with same name. Line %zu\n", node->data.varDeclaration.name, node->line);
                    exit(1);
                }
                struct Value val = evaluateASTNode(node->data.varDeclaration.node, env);
                // val.type matches node->nodeType then set, else type error.
                bool typeMatch = doesDataTypeMatchesData(val.type, node->data.varDeclaration.dataType);
                if (!typeMatch) {
                    printf("Cannot assign variable at line %zu, data and type does not match.\n", node->line);
                    exit(1);
                }

                setValue(env, node->data.varDeclaration.name, val);
                return val;
            }
        case NODE_VARIABLE_ASSIGN:
            {
                struct Value* previousData = getValue(env, node->data.varAssignment.name);
                // if get data does not exist
                if (!previousData) {
                    printf("Variable reference on line %zu does not exist, therefore cannot assign value.\n", node->line);
                    exit(1);
                }
                struct Value val = evaluateASTNode(node->data.varAssignment.node, env);
                // if datatype does not match
                if (previousData->type != val.type) {
                    printf("Assigning variable datatype does not match on line %zu.\n", node->line);
                    exit(1);
                }
                setValue(env, node->data.varAssignment.name, val);
                return val;
            }
        case NODE_FUNCTION_DECLARATION:
            {
                struct Value val = createFunctionValue(node->data.funcDeclaration.codeBlock);
                val.originNode = node;
                setValue(env, node->data.funcDeclaration.name, val);
                return val;
            }
        case NODE_FUNCTION_CALL:
            {
                struct Value val = *getValue(env, node->data.funcCall.name);
                struct ASTFunctionDeclaration funcDeclaration = val.originNode->data.funcDeclaration;

                if (funcDeclaration.parameterCount != node->data.funcCall.argumentCount) {
                    printf("Argument count does not match. Expected %zu, got %zu. Line %zu\n", 
                        funcDeclaration.parameterCount, node->data.funcCall.argumentCount, node->line);
                    exit(1);
                }
                
                struct Environment scopeEnv;
                createEnvironment(&scopeEnv);

                for (size_t i = 0; i < node->data.funcCall.argumentCount; i++) {
                    struct Value argVal = evaluateASTNode(node->data.funcCall.arguments[i], env);
                    if (!doesDataTypeMatchesData(argVal.type, funcDeclaration.parameters[i].dataType)) {
                        printf("Datatype of argument does not match relative parameter datatype, line %zu\n", node->line);
                        exit(1);
                    }
                    setValue(&scopeEnv, funcDeclaration.parameters[i].name, argVal);
                }

                evaluateAST(val.data.nodeList, &scopeEnv);
                freeEnvironment(&scopeEnv);
                // could change later to get a return
                return createNumberValue(0);

            }
        case NODE_IF_STATEMENT:
            {
                // get value of condition, should be boolean. If true then execute code block.
                struct Value val = evaluateASTNode(node->data.ifStatement.condition, env);
                if (val.type != VALUE_BOOL) {
                    printf("Condition in if statement should have a boolean value, line %zu\n", node->line);
                    exit(1);
                }
                if (val.data.boolVal) {
                    evaluateAST(node->data.ifStatement.conditionTrueBlock, env);
                }
                return createNumberValue(0);
            }
        case NODE_LOOP_STATEMENT:
            {
                struct Value loopCount = evaluateASTNode(node->data.loopStatement.loopCount, env);
                // not number
                if (loopCount.type != VALUE_NUMBER) {
                    printf("Loop count must be a number value, line %zu\n", node->line);
                    exit(1);
                }
                // negative
                if (loopCount.data.number < 0.0) {
                    printf("Negative loop count is not possible, line %zu\n", node->line);
                    exit(1);
                }
                // convert double value to size_t,
                size_t loopAmount = (size_t) loopCount.data.number;
                for (size_t i = 0; i < loopAmount; i++) {
                    evaluateAST(node->data.loopStatement.loopCodeBlock, env);
                }
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
        if (node->nodeType == NODE_VARIABLE_REFERENCE) {
            if (val.type == VALUE_NUMBER) {
                printf("%g\n", val.data.number);
            } else if (val.type == VALUE_TEXT) {
                printf("%s\n", val.data.text);
            } else if (val.type == VALUE_BOOL) {
                printf("%s\n", val.data.boolVal ? "true" : "false");
            }
        }
    }
}
