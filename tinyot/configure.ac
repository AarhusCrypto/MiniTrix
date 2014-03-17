define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([TinyOT],[0.1.svnversion])
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
# 0.1: Trud 0.2: Alviss 0.3: Modi 0.4 Mjolner ... 1.0: Thor
AC_DEFINE(CODENAME, "Hermod", [CODE Name])
AC_CONFIG_SRCDIR([src/tinyot.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB
AC_OUTPUT([Makefile 
          headers/Makefile
          src/Makefile
  	      test/Makefile
          ])


