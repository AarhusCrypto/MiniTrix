include $(top_srcdir)/carena.am

bin_PROGRAMS = test tests gracefulshutdown

gracefulshutdown_SOURCES = gracefulshutdown.c
gracefulshutdown_LDADD = $(CARENA_LIB)

tests_SOURCES = tests.c
tests_LDADD = $(CARENA_LIB)

test_SOURCES = test.c
test_LDADD = $(CARENA_LIB)
