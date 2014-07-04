include $(top_srcdir)/osal.am

LIB_OSAL=../posix_src/libosalposix.a ../src/libosal.a

bin_PROGRAMS =  utf8test opentest threadtest mutextest testoe singlelinkedlisttest stack 

stack_SOURCES = stack.c
stack_LDADD = $(LIB_OSAL) -lrt

singlelinkedlisttest_SOURCES = singlelinkedlisttest.c
singlelinkedlisttest_LDADD = $(LIB_OSAL) -lrt

testoe_SOURCES = testoe.c
testoe_LDADD = $(LIB_OSAL) -lrt

mutextest_SOURCES = mutextest.c
mutextest_LDADD = $(LIB_OSAL) -lrt

threadtest_SOURCES = threadtest.c
threadtest_LDADD = $(LIB_OSAL) -lrt 

opentest_SOURCES = opentest.c
opentest_LDADD = $(LIB_OSAL) -lrt

utf8test_SOURCES = utf8test.c
utf8test_LDADD = $(LIB_OSAL) -lrt
