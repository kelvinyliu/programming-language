#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokeniser.h"

// Helper to print TokenType
const char* tokenTypeName(enum TokenType t) {
    switch (t) {
		case TEXT_TYPE:    return "TEXT_TYPE";
		case NUMBER_TYPE:  return "NUMBER_TYPE";
		case TEXT:         return "TEXT";
        case NUMBER:       return "NUMBER";
        case IDENTIFIER:   return "IDENTIFIER";
        case EQUAL:        return "EQUAL";
        case SEMICOLON:    return "SEMICOLON";
        case END_OF_FILE:  return "END_OF_FILE";
        default:           return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file-path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    // Determine file size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(file);
        return EXIT_FAILURE;
    }
    long fileSize = ftell(file);
    if (fileSize < 0) {
        perror("ftell");
        fclose(file);
        return EXIT_FAILURE;
    }
    rewind(file);

    // Read entire file into buffer
    char *source = malloc(fileSize + 1);
    if (!source) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t bytesRead = fread(source, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        fprintf(stderr, "Failed to read file\n");
        free(source);
        fclose(file);
        return EXIT_FAILURE;
    }
    source[fileSize] = '\0';
    fclose(file);

    // Tokenize
    struct TokenList tokens = tokenise(source);
    free(source);

    // Print out each token
    printf("Got %zu tokens:\n", tokens.count);
    for (size_t i = 0; i < tokens.count; i++) {
        struct Token *tok = &tokens.data[i];
        printf("  [%2zu] %-12s  \"", i, tokenTypeName(tok->tokenType));
        fwrite(tok->lexeme, 1, tok->length, stdout);
        printf("\"");

        if (tok->tokenType == NUMBER) {
            printf("  (value = %g)", tok->literal.number_value);
        } else if ((tok->tokenType == IDENTIFIER || tok->tokenType == TEXT) && tok->literal.text_value) {
            printf("  (text_value = \"%s\")", tok->literal.text_value);
        }
        printf("\n");
    }

    // Clean up
    destroyTokenList(&tokens);
    return EXIT_SUCCESS;
}

