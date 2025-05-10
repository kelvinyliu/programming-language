#include "../include/ast.h"

#define AST_INITIAL_CAPACITY 10

void initAST(struct ASTNodeList* ast) {
    ast->nodes = malloc(sizeof(struct ASTNode*) * AST_INITIAL_CAPACITY);
    ast->count = 0;
    ast->capacity = AST_INITIAL_CAPACITY;
}

void appendAST(struct ASTNodeList* ast, struct ASTNode* node) {
    if (ast->count >= ast->capacity) {
        size_t newCapacity = ast->capacity * 2;
        ast->nodes = realloc(ast->nodes, sizeof(struct ASTNode*) * newCapacity);
        ast->capacity = newCapacity;
    }
    ast->nodes[ast->count] = node;
    ast->count++;
}

void destroyNode(struct ASTNode *n) {
    if (!n) return;
    switch (n->nodeType) {
      case NODE_NUMBER_LITERAL:
        // nothing extra
        break;

      case NODE_TEXT_LITERAL:
        // textValue was strdup'ed in parsePrimary
        free(n->data.textValue);
        break;
      
      case NODE_BOOL_LITERAL:
        // NOTHING
        break;

      case NODE_VARIABLE_REFERENCE:
        // same: strdup'ed name
        free(n->data.textValue);
        break;

      case NODE_BINARY_OPERATION:
        // destroy both subtrees
        destroyNode(n->data.binary.leftSide);
        destroyNode(n->data.binary.rightSide);
        break;

      case NODE_VARIABLE_DECLARATION:
        // free the variable name, then the initialiser subtree
        free(n->data.varDeclaration.name);
        destroyNode(n->data.varDeclaration.node);
        break;

      case NODE_VARIABLE_ASSIGN:
        // free the variable name, then the RHS subtree
        free(n->data.varAssignment.name);
        destroyNode(n->data.varAssignment.node);
        break;

      case NODE_FUNCTION_DECLARATION:
        free(n->data.funcDeclaration.name);

        if (n->data.funcDeclaration.parameters && n->data.funcDeclaration.parameterCount > 0) {
          for (size_t i = 0; i < n->data.funcDeclaration.parameterCount; i++) {
            free(n->data.funcDeclaration.parameters[i].name);
          }
          free(n->data.funcDeclaration.parameters);
        }

        destroyAST(n->data.funcDeclaration.codeBlock);
        free(n->data.funcDeclaration.codeBlock);
        break;

      case NODE_FUNCTION_CALL:
        free(n->data.funcCall.name);
        break;

      default:
        // others
        break;
    }
    free(n);
}

void destroyAST(struct ASTNodeList* ast) {
    // 1) free each ASTNode and its contents
    for (size_t i = 0; i < ast->count; i++) {
        destroyNode(ast->nodes[i]);
    }

    // 2) free the array of pointers
    free(ast->nodes);

    // 3) reset the list
    ast->nodes    = NULL;
    ast->count    = 0;
    ast->capacity = 0;
}
