include $(top_srcdir)/osal.am

bin_PROGRAMS =  utf8test opentest threadtest mutextest testoe singlelinkedlisttest

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

test1_SOURCES = test1.c
test1_LDADD = $(LIB_OSAL) -lrt

utf8test_SOURCES = utf8test.c
utf8test_LDADD = $(LIB_OSAL) -lrt
