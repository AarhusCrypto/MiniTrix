define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([Operating Environment for Linux],[0.1.svnversion])
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
# 0.1: Trud 0.2: Alviss 0.3: Modi 0.4 Mjolner ... 1.0: Thor
AC_DEFINE(CODENAME, "Asgard", [CODE Name])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/common.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
#AC_PROG_CXX
AC_PROG_CC
AC_CHECK_FUNCS([memset free malloc read write open close fcntl setsockopt socket listen accept])
AC_PROG_RANLIB
#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)

AC_OUTPUT([Makefile 
           headers/Makefile
           src/Makefile
           linux_src/Makefile
           test/Makefile
          ])


