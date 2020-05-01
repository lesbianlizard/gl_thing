CC = gcc
CFLAGS = -Wall -Wextra -ggdb
CFLAGS += -lglfw -lGLEW -lGL

thing: thing.c
	$(CC) $(CFLAGS) $< -o $@
