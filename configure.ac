# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Copyright 2003-2007 R. Mattes <rm@seid-online.de>
#
# Licence: GPL
#

AC_PREREQ(2.57) 
AC_INIT([facts],[0.2], [rm (at) seid-online (dot) de])
AC_CONFIG_SRCDIR([facts.c])
# AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
# AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for helper programs

AC_ARG_WITH(pg_config,
    AS_HELP_STRING(--with-pg_config=PATH,/path/to/pg_config),
    [ AC_PATH_PROGS(PGCONFIG, [pg_config], "notfound", "$withval:$PATH") ], 
    [ AC_PATH_PROGS(PGCONFIG, [pg_config], "notfound", "/usr/local/apache/bin:/usr/local/bin:/usr/sbin:$PATH")])

if test "x$PGCONFIG" = "xnotfound" ; then
    AC_MSG_ERROR([*** Sorry, could not find pg_config ***])
fi

postgresql_includes=`$PGCONFIG --includedir`
AC_MSG_NOTICE([using PostgreSQL headers at $postgresql_includes])

postgresql_server_includes=`$PGCONFIG --includedir-server`
AC_MSG_NOTICE([using PostgreSQL server headers at $postgresql_server_includes])

PG_LIBDIR=`$PGCONFIG --pkglibdir`
AC_MSG_NOTICE([Extension will be installed into $PG_LIBDIR])

AC_SUBST(PG_LIBDIR)
AC_CONFIG_FILES([Makefile SQL/install_funcs.sql SQL/create_dbm.sql])
AC_OUTPUT
