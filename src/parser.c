#include "../include/parser.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct ASTNode* createBinaryNode(enum BinaryOperatorTypes op, struct ASTNode* left, struct ASTNode* right, size_t line, size_t column) {
    struct ASTNode* n = malloc(sizeof(struct ASTNode));
    n->nodeType = NODE_BINARY_OPERATION;
    n->line = line;
    n->column = column;
    n->data.binary.operationChar = op;
    n->data.binary.leftSide = left;
    n->data.binary.rightSide = right;
    
    return n;
}

struct ASTNode* parsePrimary(struct TokenList* tokens, size_t* index) {
    struct Token* token = &tokens->data[*index];

    if (token->tokenType == NUMBER) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->line = token->line;
        node->column = token->column;
        node->nodeType = NODE_NUMBER_LITERAL;
        node->data.numberValue = token->literal.number_value;
        
        (*index)++;
        return node;
    }

    if (token->tokenType == IDENTIFIER) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->line = token->line;
        node->column = token->column;
        node->nodeType = NODE_VARIABLE_REFERENCE;
        node->data.textValue = strndup(token->lexeme, token->length);
        
        (*index)++;
        return node;
    }

    if (token->tokenType == TEXT) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->line = token->line;
        node->column = token->column;
        node->nodeType = NODE_TEXT_LITERAL;
        node->data.textValue = strndup(token->lexeme, token->length);

        (*index)++;
        return node;
    }

    if (token->tokenType == FALSE || token->tokenType == TRUE) {
        struct ASTNode* node = malloc(sizeof(struct ASTNode));
        node->line = token->line;
        node->column = token->column;
        node->nodeType = NODE_BOOL_LITERAL;
        node->data.boolValue = token->tokenType == FALSE ? false : true;

        (*index)++;
        return node;
    }

    // CANNOT PARSE PRIMARY
    // LOG ERROR
    printf("error parsing primary");
    return NULL;
}

struct ASTNode* parseFactor(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];

    // if has parens, parse entire; else its a literal or identifier.
    if (token.tokenType == LEFT_PAREN) {
        (*index)++;
        struct ASTNode* subExpr = parseTopLevel(tokens,index);
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
            enum BinaryOperatorTypes op;
            switch (token.tokenType) {
                case STAR:
                    op = BIN_OP_STAR;
                    break;
                case SLASH:
                    op = BIN_OP_SLASH;
                    break;
                default:
                    printf("Could not identify binary operator. Line %zu\n", token.line);
                    exit(1);
                    
            }
            (*index)++; 
            struct ASTNode* rightSide = parseFactor(tokens, index);
            
            size_t line = token.line;
            size_t column = token.column;
            node = createBinaryNode(op, node, rightSide, line, column);
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
            enum BinaryOperatorTypes op;
            switch (token.tokenType) {
                case PLUS:
                    op = BIN_OP_PLUS;
                    break;
                case MINUS:
                    op = BIN_OP_MINUS;
                    break;
                default:
                    printf("Could not identify binary operator. Line %zu\n", token.line);
                    exit(1);

            }
            (*index)++;
            struct ASTNode* rightSide = parseTerm(tokens, index);
            size_t line = token.line;
            size_t column = token.column;
            node = createBinaryNode(op, node, rightSide, line, column);
        } else {
            break;
        }
    }
    return node;
}

struct ASTNode* parseDeclaration(struct TokenList* tokens, size_t* index) {
    enum TokenType dataType = tokens->data[*index].tokenType;
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
    
    struct ASTNode* init = parseTopLevel(tokens, index);

    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';'. Line %zu\n", token.line);
        exit(1);
    }
    (*index)++;

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_VARIABLE_DECLARATION;
    node->data.varDeclaration.name = name;
    node->data.varDeclaration.dataType = dataType;
    node->data.varDeclaration.node = init;
    return node;
}

struct ASTNode* parseAssignment(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];

    // to assign variable correctly, the token datatype has to match the correct literal
    // this would involve many if statements, would creating a method for this be better as it could be 
    // used multiple times later?

    // encounter error with statements such as number x = 1 + 2; <-- expression but expects a number literal
    // solution puts the datatype check in evaluator.
    
    // get var name
    char* name = strndup(token.lexeme, token.length);
    (*index)++;
    
    // get = 
    if (tokens->data[*index].tokenType != EQUAL) {
        printf("Expected '=', in assignment.\n");
        exit(1);
    }
    (*index)++;

    struct ASTNode* assignValue = parseTopLevel(tokens, index);

    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';'\n");
        exit(1);
    }
    (*index)++;


    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_VARIABLE_ASSIGN;
    node->data.varAssignment.name = name;
    node->data.varAssignment.node = assignValue;
    
    return node;
}

struct ASTNodeList* parseCodeBlock(struct TokenList* tokens, size_t* index) {
    // check for '{'
    if (tokens->data[*index].tokenType != LEFT_CURLY) {
        printf("Expected '{' at line %zu, column %zu\n", tokens->data[*index].line, 
            tokens->data[*index].column);
        exit(1);
    }
    (*index)++;

    struct ASTNodeList* ast = malloc(sizeof(struct ASTNodeList));
    initAST(ast);

    while (tokens->data[*index].tokenType != RIGHT_CURLY)
    {
        if (tokens->data[*index].tokenType == END_OF_FILE) {
            printf("Expected '}' to close code block, reached end of file instead.\n");
            exit(1);
        }
        struct ASTNode* statement = parseStatement(tokens, index);
        appendAST(ast, statement);
    }
    (*index)++;
    
    return ast;
}

struct ASTNode* parseFunctionDeclaration(struct TokenList* tokens, size_t* index) {
    (*index)++; // skip "fn" keyword
    struct Token token = tokens->data[*index];

    // get function name
    char* name = strndup(token.lexeme, token.length);
    (*index)++;

    // handle params
    (*index)++;
    size_t parameterCounter = 0;
    struct Parameter* params = NULL;
    while (tokens->data[*index].tokenType != RIGHT_PAREN) {
        enum TokenType dataType = tokens->data[*index].tokenType;
        if (dataType != NUMBER_TYPE &&
                dataType != TEXT_TYPE &&
                dataType != BOOLEAN_TYPE) {
            printf("Declaring function parameters must be in the form of datatype variable_name, line %zu\n", token.line);
            exit(1);
        }
        (*index)++;
        struct Token parameterToken = tokens->data[*index];
        if (parameterToken.tokenType != IDENTIFIER) {
            printf("Expected parameter name on line %zu\n", token.line);
            exit(1);
        } 

        struct Parameter param;
        param.dataType = dataType;
        param.name = strndup(parameterToken.lexeme, parameterToken.length);

        params = realloc(params, sizeof(struct Parameter) * (parameterCounter+1));
        params[parameterCounter] = param;
        parameterCounter++;

        (*index)++;
        if (tokens->data[*index].tokenType == COMMA) {
            (*index)++;
        }
    }
    (*index)++;

    // get code block
    struct ASTNodeList* codeBlock = parseCodeBlock(tokens, index);

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_FUNCTION_DECLARATION;
    node->data.funcDeclaration.name = name;
    node->data.funcDeclaration.parameters = params;
    node->data.funcDeclaration.parameterCount = parameterCounter;
    node->data.funcDeclaration.codeBlock = codeBlock;

    return node;
}

struct ASTNode* parseFunctionCall(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];
    (*index)++;

    char* funcName = strndup(token.lexeme, token.length);

    // ( paren
    (*index)++;

    size_t argumentCounter = 0;
    struct ASTNode** arguments = NULL;
    while (tokens->data[*index].tokenType != RIGHT_PAREN) {
        struct ASTNode* arg = parseTopLevel(tokens, index);

        arguments = realloc(arguments, sizeof(struct ASTNode*) * (argumentCounter + 1));
        arguments[argumentCounter] = arg;
        argumentCounter++;

        if (tokens->data[*index].tokenType == COMMA) {
            (*index)++;
        }
    }
    (*index)++;

    // semi colon
    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expected ';' at line %zu, column %zu.\n", 
            tokens->data[*index].line, tokens->data[*index].column);
        exit(1);
    }
    (*index)++;

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_FUNCTION_CALL;
    node->data.funcCall.argumentCount = argumentCounter;
    node->data.funcCall.arguments = arguments;
    node->data.funcCall.name = funcName;

    return node;
}

// LEFT == RIGHT
struct ASTNode* parseEquality(struct TokenList* tokens, size_t* index) {
    struct ASTNode* left = parseComparsion(tokens, index);

    while (tokens->data[*index].tokenType == EQUALITY_OPERATOR) {
        struct Token token = tokens->data[*index];
        (*index)++;
        struct ASTNode* right = parseComparsion(tokens, index);
        left = createBinaryNode(BIN_OP_EQUALITY, left, right, token.line, token.column);
    }

    return left;
}

struct ASTNode* parseComparsion(struct TokenList* tokens, size_t* index) {
    struct ASTNode* left = parseExpression(tokens, index);

    while (true) {
        enum TokenType type = tokens->data[*index].tokenType;
        enum BinaryOperatorTypes op;

        switch (type) {
            case LESS_THAN:
                op = BIN_OP_LESS;
                break;
            case MORE_THAN:
                op = BIN_OP_GREATER;
                break;
            case LESSER_EQUAL:
                op = BIN_OP_LESSER_EQUAL;
                break;
            case GREATER_EQUAL:
                op = BIN_OP_GREATER_EQUAL;
                break;
            default:
                return left;
        }

        if (type == LESS_THAN || type == MORE_THAN || type == LESSER_EQUAL || type == GREATER_EQUAL) {
            struct Token token = tokens->data[*index];
            (*index)++;

            struct ASTNode* right = parseExpression(tokens, index);
            left = createBinaryNode(op, left, right, token.line, token.column);
        } else {
            break;
        }
    }
    return left;

}

struct ASTNode* parseIfStatement(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];
    (*index)++;

    // check for left paren;
    if (tokens->data[*index].tokenType != LEFT_PAREN) {
        printf("Expected '(' to create if statement on line %zu\n", token.line);
        exit(1);
    }
    (*index)++;
    
    // get condition
    struct ASTNode* condition = parseTopLevel(tokens, index);
    
    // check for right paren
    if (tokens->data[*index].tokenType != RIGHT_PAREN) {
        printf("Expected ')' to end if statement condition on line %zu\n", token.line);
        exit(1);
    }
    (*index)++;

    // get code block
    struct ASTNodeList* codeBlock = parseCodeBlock(tokens, index);

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_IF_STATEMENT;
    node->data.ifStatement.condition = condition;
    node->data.ifStatement.conditionTrueBlock = codeBlock;

    return node;
}

struct ASTNode* parseLoopStatement(struct TokenList* tokens, size_t* index) {
    struct Token token = tokens->data[*index];
    (*index)++;

    // get loop count
    struct ASTNode* loopCount = parseTopLevel(tokens, index);

    // get code block
    struct ASTNodeList* codeBlock = parseCodeBlock(tokens, index);

    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->line = token.line;
    node->column = token.column;
    node->nodeType = NODE_LOOP_STATEMENT;
    node->data.loopStatement.loopCount = loopCount;
    node->data.loopStatement.loopCodeBlock = codeBlock;

    return node;
}

// recursive descent top level call
struct ASTNode* parseTopLevel(struct TokenList* tokens, size_t* index) {
    return parseEquality(tokens, index);
}

struct ASTNode* parseStatement(struct TokenList* tokens, size_t* index) {
    enum TokenType tokenType = tokens->data[*index].tokenType;

    // FUNCTION DECLARATION
    if (tokenType == FUNCTION_DECLARATION) {
        return parseFunctionDeclaration(tokens, index);
    }

    // FUNCTION CALL
    if (tokenType == IDENTIFIER && 
            tokens->data[*index + 1].tokenType == LEFT_PAREN) {
        return parseFunctionCall(tokens, index);
    }

    // IF STATEMENT
    if (tokenType == IF_DECLARATION) {
        return parseIfStatement(tokens, index);
    }

    // LOOP STATEMENT
    if (tokenType == LOOP_DECLARATION) {
        return parseLoopStatement(tokens, index);
    }

    // DECLARATION
    if (tokenType == TEXT_TYPE || tokenType == NUMBER_TYPE || tokenType == BOOLEAN_TYPE) {
        return parseDeclaration(tokens, index);
    }

    // ASSIGNMENT
    if (tokenType == IDENTIFIER && tokens->data[*index + 1].tokenType == EQUAL) {
        return parseAssignment(tokens, index);
    }

    // EXPRESSION
    struct ASTNode* expression = parseTopLevel(tokens, index);
    if (tokens->data[*index].tokenType != SEMICOLON) {
        printf("Expectedb ';'. Line %zu\n", tokens->data[*index].line);
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
