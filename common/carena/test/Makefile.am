include $(top_srcdir)/carena.am

CARENA_LIB=../src/libcarena.a

bin_PROGRAMS = test 

test_SOURCES = test.c testsendraw.c testsend.c
test_LDADD = $(CARENA_LIB) -ldatetime
