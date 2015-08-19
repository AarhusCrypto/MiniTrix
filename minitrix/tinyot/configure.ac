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
AM_CONDITIONAL(OSX,test x`uname` = "xDarwin")
if test x`uname ` = "xDarwin";
then
AC_DEFINE([MAC_OS_X],[1],[Assuming we are building on MAC OSX])
else
AC_CHECK_LIB([rt],[clock_gettime],,[AC_MSG_ERROR([Could not find pthread])],)
fi

AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthread])],)
AC_CHECK_LIB([osal],[l2b],,[AC_MSG_ERROR([Could not find OSal])],-lpthread)
AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSalPosix])],-lpthread)
AC_CHECK_LIB([carena],[CArena_new],,[AC_MSG_ERROR([Could not find carena])],[-lds -lencoding -losal -lpthread])
AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find ds])],[-losal ])
AC_CHECK_LIB([encoding],[der_decode_seq],,[AC_MSG_ERROR([Could not find encoding])], [-losalposix -losal])

AC_PROG_RANLIB
AC_OUTPUT([Makefile 
          headers/Makefile
          src/Makefile
  	      test/Makefile
          ])


