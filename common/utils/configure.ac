AC_INIT([Utils],[1.0])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/utils/options.c])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([could not find pthread])],)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[-lpthread])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lpthread])
AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find ds])],[-losalposix -losal])
AC_CONFIG_FILES([Makefile 
          include/Makefile
          src/Makefile
          src/utils/Makefile
	        test/Makefile
          ])


AC_OUTPUT
