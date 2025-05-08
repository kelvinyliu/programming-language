CC = gcc
CFLAGS = -Wall
CFILES = main.c tokeniser.c parser.c ast.c evaluator.c

main:
	$(CC) $(CFLAGS) -o main $(CFILES)

clean:
	rm -f main
