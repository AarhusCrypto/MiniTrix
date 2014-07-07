AC_INIT([math],[1.0])
# Safety check that this configure is in the right position
AC_CONFIG_SRCDIR([src/math/matrix.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_RANLIB
# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
          headers/Makefile
          src/Makefile
	  src/math/Makefile
	  test/Makefile
          ])


