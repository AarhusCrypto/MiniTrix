define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([Operating System Abstraction Layer],[0.1.svnversion])
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
AC_DEFINE(CODENAME, "Asgard", [CODE Name])
AC_CONFIG_SRCDIR([src/common.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC_STDC
AC_PROG_CC
AC_CHECK_FUNCS([memset free malloc read\
                write open close fcntl\
                setsockopt socket listen\
                accept pthread_create\
                pthread_join pthread_mutex_lock])
AC_PROG_RANLIB
AC_C_INLINE
AC_FUNC_MALLOC

AC_OUTPUT([Makefile 
           headers/Makefile
           src/Makefile
           posix_src/Makefile
           test/Makefile
          ])


