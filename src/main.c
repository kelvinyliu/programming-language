// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "evaluator.h"

// forward
static void printNode(const struct ASTNode* n);
static void dumpAST(const struct ASTNodeList* prog);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file-path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 1) Read whole file into `source`
    const char *path = argv[1];
    FILE *file = fopen(path, "rb");
    if (!file) { perror("fopen"); return EXIT_FAILURE; }
    if (fseek(file, 0, SEEK_END) != 0) { perror("fseek"); fclose(file); return EXIT_FAILURE; }
    long fileSize = ftell(file);
    if (fileSize < 0) { perror("ftell"); fclose(file); return EXIT_FAILURE; }
    rewind(file);

    char *source = malloc(fileSize + 1);
    if (!source) { fprintf(stderr, "malloc failed\n"); fclose(file); return EXIT_FAILURE; }
    if (fread(source, 1, fileSize, file) != (size_t)fileSize) {
        fprintf(stderr, "fread failed\n");
        free(source);
        fclose(file);
        return EXIT_FAILURE;
    }
    source[fileSize] = '\0';
    fclose(file);

    // 2) Parse entire program into an ASTNodeList
    struct ASTNodeList program = parseProgram(source);
    free(source);

    // 3) Dump the AST
    //dumpAST(&program);

    struct Environment env;
    createEnvironment(&env);
    evaluateAST(&program, &env);
    freeEnvironment(&env);

    // 4) Clean up
    destroyAST(&program);
    return EXIT_SUCCESS;
}

// recursively print one AST node
static void printNode(const struct ASTNode* n) {
    switch (n->nodeType) {
      case NODE_NUMBER_LITERAL:
        printf("%g", n->data.numberValue);
        break;
      case NODE_TEXT_LITERAL:
        printf("%s", n->data.textValue);
        break;
      case NODE_VARIABLE_REFERENCE:
        printf("%s", n->data.textValue);
        break;
      case NODE_BINARY_OPERATION:
        printf("(");
        printNode(n->data.binary.leftSide);
        printf(" %c ", n->data.binary.operationChar);
        printNode(n->data.binary.rightSide);
        printf(")");
        break;
      case NODE_VARIABLE_DECLARATION:
        printf("[declare %s = ", n->data.varDeclaration.name);
        printNode(n->data.varDeclaration.node);
        printf("]");
        break;
      case NODE_VARIABLE_ASSIGN:
        printf("[assign %s = ", n->data.varAssignment.name);
        printNode(n->data.varAssignment.node);
        printf("]");
        break;
      default:
        printf("<unknown-node>");
    }
}

// walk the ASTNodeList and print each statement on its own line
static void dumpAST(const struct ASTNodeList* prog) {
    for (size_t i = 0; i < prog->count; i++) {
        printNode(prog->nodes[i]);
        printf("\n");
    }
}

