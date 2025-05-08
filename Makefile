CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
CFILES = main.c src/tokeniser.c src/parser.c src/ast.c src/evaluator.c

main:
	$(CC) $(CFLAGS) -o bin/main $(CFILES)

clean:
	rm -f bin/main
