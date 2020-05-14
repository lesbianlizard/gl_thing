CC = gcc
CFLAGS = -Wall -Wextra -ggdb
CFLAGS += -lglut -lGLEW -lGL -lpthread -ljack

thing: thing.c
	$(CC) $(CFLAGS) $< -o $@
