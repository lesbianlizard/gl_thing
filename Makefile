CC = gcc
CFLAGS = -Wall -Wextra -ggdb
CFLAGS += -lglut -lGLEW -lGL

thing: thing.c
	$(CC) $(CFLAGS) $< -o $@
