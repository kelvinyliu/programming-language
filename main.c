#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "evaluator.h"

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

    struct Environment env;
    createEnvironment(&env);
    evaluateAST(&program, &env);
    freeEnvironment(&env);

    // 3) Clean up
    destroyAST(&program);
    return EXIT_SUCCESS;
}

