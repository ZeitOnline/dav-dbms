
PREFIX=/usr/local

CC=gcc
CFLAGS=-I/usr/include/postgresql/server/ -fpic -shared
WARNINGS=-Wall -Wmissing-prototypes

INSTDIR=$(PREFIX)/lib/postgresql/


SRC=facts.c

install: dso
	install -D -o postgres facts.so $(INSTDIR)/lib/facts.so

dso: facts.so

facts.so: facts.c Makefile
	$(CC) $(WARNINGS) $(CFLAGS) -o facts.so $(SRC)
