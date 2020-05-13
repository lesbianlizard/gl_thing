CC = gcc
CFLAGS = -Wall -Wextra -ggdb
CFLAGS += -lglut -lGLEW -lGL -lpthread

thing: thing.c
	$(CC) $(CFLAGS) $< -o $@
