# Operation modes:
#   --bindir              show location of user executables
#   --includedir          show location of C header files of the client
#                         interfaces
#   --includedir-server   show location of C header files for the server
#   --libdir              show location of object code libraries
#   --pkglibdir           show location of dynamically loadable modules
#   --configure           show options given to 'configure' script when
#                         PostgreSQL was built
#   --version             show the PostgreSQL version and exit
# 
#
PREFIX=/usr/local
PGCONFIG=pg_config
CC=gcc
CFLAGS=-I `$(PGCONFIG) --includedir` -I `$(PGCONFIG) --includedir-server`  -fpic -shared
LIBS=-L `$(PGCONFIG) --libdir`
WARNINGS=-Wall -Wmissing-prototypes

INSTDIR=`$(PGCONFIG) --pkglibdir`


SRC=facts.c

install: dso
	install -D -o postgres facts.so $(INSTDIR)/facts.so

dso: facts.so

facts.so: facts.c facts.h Makefile
	$(CC) $(WARNINGS) $(CFLAGS) $(LIBS) -o facts.so $(SRC)
