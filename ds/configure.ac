AC_INIT([ds],[0.1])
AC_CONFIG_SRCDIR([src/hashmap.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_CHECK_LIB(osal,OperatingEnvironment_LinuxNew,,,)
AC_PROG_CC
AC_PROG_RANLIB

AC_OUTPUT([Makefile 
          headers/Makefile
          src/Makefile
  	      test/Makefile
          ])


