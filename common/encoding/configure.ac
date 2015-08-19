AC_INIT([Encoding],[1.0])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/encoding/der.c])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
#AC_PROG_CXX
AC_PROG_CC
#AC_CHECK_FUNCS([memset])
AC_PROG_RANLIB
AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthrad])],)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lpthread])


#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)

AC_CONFIG_FILES([Makefile 
          src/Makefile
	  src/encoding/Makefile
	  include/Makefile
	  test/Makefile
          ])


AC_OUTPUT
