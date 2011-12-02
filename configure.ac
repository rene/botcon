##
# BOTCON configure.ac
#
AC_INIT([botcon], 0.1, [rene@renesp.com.br])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([glib-2.0],[g_slist_free],,AC_MSG_ERROR([ERROR! glib-2.0 not found...]))
GLIB_CFLAGS=`pkg-config --cflags glib-2.0`
GLIB_LIBS=`pkg-config --libs glib-2.0`
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

GTHREAD_CFLAGS=`pkg-config --cflags gthread-2.0`
GTHREAD_LIBS=`pkg-config --libs gthread-2.0`
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

AC_CHECK_LIB([loudmouth-1],[lm_connection_open],,AC_MSG_ERROR([ERROR! loudmouth-1 not found...]))
LOUDMOUTH_CFLAGS=`pkg-config --cflags loudmouth-1.0`
LOUDMOUTH_LIBS=`pkg-config --libs loudmouth-1.0`
AC_SUBST(LOUDMOUTH_CFLAGS)
AC_SUBST(LOUDMOUTH_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([getopt.h])

# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

# Output files.
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_CHECK_FUNCS([regcomp regexec])
AC_OUTPUT

