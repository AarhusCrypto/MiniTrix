include $(top_srcdir)/cminimacs.am

bin_PROGRAMS = test testminimacs client testencoder testfft clientfft	\
perf testsymminimacs symclient perfsrv perfcli loadreps bdttest				\
bdtmptest mulpar2test bitenctest bitfftenctest manybdtmptest					\
combienctest testbwmp2 testfft119enc

testfft119enc_SOURCES = testfft119.c
testfft119enc_LDADD = $(CMINIMACS_LIB)

testbwmp2_SOURCES = testbwmp2.c
testbwmp2_LDADD = $(CMINIMACS_LIB) 

combienctest_SOURCES = combienctest.c
combienctest_LDADD = $(CMINIMACS_LIB) 

manybdtmptest_SOURCES = manybdtmptest.c
manybdtmptest_LDADD = $(CMINIMACS_LIB) 

bitfftenctest_SOURCES = bitfftenctest.c
bitfftenctest_LDADD = $(CMINIMACS_LIB) 


bitenctest_SOURCES = bitenctest.c
bitenctest_LDADD = $(CMINIMACS_LIB) 

mulpar2test_SOURCES = mulpar2test.c
mulpar2test_LDADD = $(CMINIMACS_LIB) 

bdtmptest_SOURCES = bdtmptest.c
bdtmptest_LDADD = $(CMINIMACS_LIB) 


bdttest_SOURCES = bdttest.c
bdttest_LDADD = $(CMINIMACS_LIB) 

loadreps_SOURCES = loadreps.c
loadreps_LDADD = $(CMINIMACS_LIB) 

perfcli_SOURCES = perfcli.c
perfsrv_SOURCES = perfsrv.c

perfcli_LDADD = $(CMINIMACS_LIB) 
perfsrv_LDADD = $(CMINIMACS_LIB) 

symclient_SOURCES = symclient.c
symclient_LDADD = $(CMINIMACS_LIB) 

testsymminimacs_SOURCES = testsymminimacs.c
testsymminimacs_LDADD = $(CMINIMACS_LIB) 

perf_SOURCES = perf.c
perf_LDADD = $(CMINIMACS_LIB)  

clientfft_SOURCES = clientfft.c
clientfft_LDADD = $(CMINIMACS_LIB)

testfft_SOURCES = testfft.c
testfft_LDADD = $(CMINIMACS_LIB)

testencoder_SOURCES = testencoder.c
testencoder_LDADD = $(CMINIMACS_LIB)

client_SOURCES = client.c
client_LDADD = $(CMINIMACS_LIB)

testminimacs_SOURCES = testminimacs.c
testminimacs_LDADD = $(CMINIMACS_LIB) 

test_SOURCES = test.c
test_LDADD = $(CMINIMACS_LIB)
