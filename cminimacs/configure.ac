define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
AC_INIT([cminimacs],[0.0.svnversion])
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(CODENAME, "MiniTrix", [Mini Trix])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/minimacs/generic_minimacs.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
#AC_PROG_CXX
AC_PROG_CC
#AC_CHECK_FUNCS([memset])
AC_PROG_RANLIB
#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
	  headers/Makefile
          src/Makefile
	  src/minimacs/Makefile
	  test/Makefile
          ])


