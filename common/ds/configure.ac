AC_INIT([ds],[0.1])
AC_CONFIG_SRCDIR([src/hashmap.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthrad])],)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lpthread])
AC_PROG_CC
AC_PROG_RANLIB

AC_OUTPUT([Makefile 
          include/Makefile
          src/Makefile
	  test/Makefile
          ])


