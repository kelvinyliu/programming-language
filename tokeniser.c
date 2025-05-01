#include <stdio.h>
#include "tokeniser.h"

void initTokenList(struct TokenList *tokenList) {
	tokenList->data = malloc(sizeof(struct Token) * 10);
	tokenList->count = 0;
	tokenList->capacity = 10;
}

void appendTokenList(struct TokenList *tokenList, struct Token token) {
	if (tokenList->count >= tokenList->capacity) {
		size_t newCapacity = tokenList->capacity * 2;
		struct Token *newData = realloc(tokenList->data, sizeof(struct Token) * newCapacity);
		
		// if realloc fails
		if (!newData) {
			printf("Realloc token list failed...\n");
			abort();
		}

		tokenList->data = newData;
		tokenList->capacity = newCapacity;
	}
	tokenList->data[tokenList->count] = token;
	tokenList->count++;
}


struct Token createToken(enum TokenType tokenType, const char* lexeme, size_t length, union uLiteral literal) {
	struct Token newToken;

	newToken.tokenType = tokenType;
	newToken.lexeme = (char *)lexeme;
	newToken.length = length;
	newToken.literal = literal;

	return newToken;
}
