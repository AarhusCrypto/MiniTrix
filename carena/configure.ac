define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([Arena],[0.1.svnversion])
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(CODENAME, "Heimdal", [CODE Name])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
AC_CONFIG_SRCDIR([src/carena.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB

AC_CHECK_LIB([rt],[clock_gettime],,[AC_MSG_ERROR([clock gettime missing])],)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_LinuxNew],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lrt])
AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find ds])],[-losal -lrt])
AC_CHECK_LIB([encoding],[der_decode_seq],,[AC_MSG_ERROR([Could not find encoding])], [-losalposix -losal])
AC_OUTPUT([Makefile 
           src/Makefile
           test/Makefile
	         headers/Makefile
          ])


