CC = gcc
CFLAGS = -Wall -Wextra -ggdb
CFLAGS += -lglut -lGLEW -lGL -lpthread -ljack

thing: thing.c jack_simple_client.h utils.h
	$(CC) $(CFLAGS) $< -o $@
