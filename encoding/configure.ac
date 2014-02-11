AC_INIT([part_template],[1.0])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/encoding/der.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
#AC_PROG_CXX
AC_PROG_CC
#AC_CHECK_FUNCS([memset])
AC_PROG_RANLIB
#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
          src/Makefile
	  src/encoding/Makefile
	  headers/Makefile
	  test/Makefile
          ])


