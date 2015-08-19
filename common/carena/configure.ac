define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(CODENAME, "Heimdal", [CODE Name])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
AC_INIT([Arena],[3.2])
AC_CONFIG_SRCDIR([src/carena.c])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB
AM_CONDITIONAL(OSX,test x`uname` = "xDarwin")
if test x`uname` = "xDarwin";
then
AC_DEFINE([MAC_OS_X],[1],[Assuming we are building on MAC OSX])
else
AC_CHECK_LIB([rt],[clock_gettime],,[AC_MSG_ERROR([Could not find clock_gettime])],)
fi
AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([could not find pthread])],)

AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,

[AC_MSG_ERROR([Could not find osal])],[-lpthread])

AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lpthread])

AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find ds])],[-losal])

AC_CHECK_LIB([encoding],[der_decode_seq],,[AC_MSG_ERROR([Could not find encoding])], [-losalposix -losal])

AC_CONFIG_FILES([Makefile 
                 src/Makefile
                 test/Makefile
	         include/Makefile
                ])

AC_OUTPUT

