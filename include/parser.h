#pragma once
#include "ast.h"
#include "tokeniser.h"

struct ASTNode* createBinaryNode(char op, struct ASTNode* left, struct ASTNode* right, size_t line, size_t column);
struct ASTNode* parsePrimary(struct TokenList* tokens, size_t* index);
struct ASTNode* parseFactor(struct TokenList* tokens, size_t* index);
struct ASTNode* parseTerm(struct TokenList* tokens, size_t* index);
struct ASTNode* parseExpression(struct TokenList* tokens, size_t* index);
struct ASTNode* parseEquality(struct TokenList* tokens, size_t* index);
struct ASTNode* parseTopLevel(struct TokenList* tokens, size_t* index);
struct ASTNode* parseStatement(struct TokenList* tokens, size_t* index);
struct ASTNode* parseDeclaration(struct TokenList* tokens, size_t* index);
struct ASTNode* parseAssignment(struct TokenList* tokens, size_t* index);
struct ASTNodeList* parseCodeBlock(struct TokenList* tokens, size_t* index);
struct ASTNode* parseFunctionDeclaration(struct TokenList* tokens, size_t* index);
struct ASTNode* parseFunctionCall(struct TokenList* tokens, size_t* index);
struct ASTNodeList parseProgram(const char* sourceCode);

