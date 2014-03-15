define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([CInterp],[0.1])
# Safety check that this configure is in the right position
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(CODENAME, "Yggdrasil", [CODE Name])
AC_DEFINE(TIMEDATE, "buildtime", [TIME of Build])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
#AC_CHECK_FUNCS([memset])
#AC_PROG_RANLIB
#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
           src/Makefile
           test/Makefile
          ])


