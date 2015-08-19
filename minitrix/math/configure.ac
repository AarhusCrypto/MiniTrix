AC_INIT([math],[1.0])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/math/matrix.c])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB



AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthread])],)

AC_CHECK_LIB([datetime],[DateTime_New],,[AC_MSG_ERROR([Could not find datetime])],-ldatetime -losalposix -losal -lpthread )

AC_CHECK_LIB([osal],[l2b],,[AC_MSG_ERROR([Could not find OSal])],-lpthread)

AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSalPosix])],-lpthread)

AC_CONFIG_FILES([Makefile
           include/Makefile
           src/Makefile
           src/math/Makefile
           test/Makefile
          ])

AC_OUTPUT
