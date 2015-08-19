include $(top_srcdir)/osal.am

LIB_OSAL=../posix_src/libosalposix.a ../src/libosal.a
testdir=test


test_PROGRAMS = testoe test_listperf

testoe_SOURCES = testtest.c testoe.c testcase.c opentest.c testprint.c threadtest.c
testoe_LDADD = $(LIB_OSAL)	

test_listperf_SOURCES = test_singlelinkedlist_perf.c
test_listperf_LDADD = $(LIB_OSAL)

#coov3test_SOURCES = coov3test.c
#coov3test_LDADD = $(LIB_OSAL)

#printtest_SOURCES = printtest.c
#printtest_LDADD = $(LIB_OSAL) 
#-lrt

#stack_SOURCES = stack.c
#stack_LDADD = $(LIB_OSAL) 
#-lrt

#singlelinkedlisttest_SOURCES = singlelinkedlisttest.c
#singlelinkedlisttest_LDADD = $(LIB_OSAL) 
#-lrt

#mutextest_SOURCES = mutextest.c
#mutextest_LDADD = $(LIB_OSAL) 
#-lrt

#threadtest_SOURCES = threadtest.c
#threadtest_LDADD = $(LIB_OSAL) 
#-lrt 

#opentest_SOURCES = opentest.c
#opentest_LDADD = $(LIB_OSAL) 
#-lrt

#utf8test_SOURCES = utf8test.c
#utf8test_LDADD = $(LIB_OSAL) 
#-lrt
