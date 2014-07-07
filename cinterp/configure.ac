define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_INIT([CInterp],[0.1])
# Safety check that this configure is in the right position
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(CODENAME, "Yggdrasil", [CODE Name])
AC_DEFINE(TIMEDATE, "buildtime", [TIME of Build])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC


AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Needs pthread])],)
AC_CHECK_LIB([rt],[clock_gettime],,[AC_MSG_ERROR([clock gettime missing])],)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_LinuxNew],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lrt])
AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find ds])],[-losal -lrt])
AC_CHECK_LIB([math],[init_polynomial],,[AC_MSG_ERROR([Could not find math])], [-losalposix -losal])
AC_CHECK_LIB([encoding],[der_decode_seq],,[AC_MSG_ERROR([Could not find encoding])], [-losalposix -losal])
AC_CHECK_LIB([carena],[CArena_new],,[AC_MSG_ERROR([Could not find carena])],[-lds -lencoding -losal -lrt -lpthread])
AC_CHECK_LIB([cminimacs],[GenericMiniMacs_DefaultNew],,[AC_MSG_ERROR([Could not find cminimacs],[-lcarena -lds -lmath -lencoding -losalposix -losal])])
AC_CHECK_LIB([utils],[Options_New],,[AC_MSG_ERROR([Could not find utils])], [-losalposix -losal])


#AC_CHECK_FUNCS([memset])
#AC_PROG_RANLIB
#AC_C_INLINE
#AC_FUNC_MALLOC
# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
           src/Makefile
           test/Makefile
          ])


