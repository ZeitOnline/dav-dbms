

CC=gcc
CFLAGS=-I/usr/include/postgresql/server/ -fpic -shared
WARNINGS=-Wall -Wmissing-prototypes

SRC=facts.c

dso: facts.so

facts.so: facts.c Makefile
	$(CC) $(WARNINGS) $(CFLAGS) -o facts.so $(SRC)