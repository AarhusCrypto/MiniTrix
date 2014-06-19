include $(top_srcdir)/cminimacs.am

bin_PROGRAMS = test testminimacs client testencoder testfft clientfft perf testsymminimacs symclient perfsrv perfcli loadreps bdttest bdtmptest mulpar2test bitenctest bitfftenctest manybdtmptest combienctest

combienctest_SOURCES = combienctest.c
combienctest_LDADD = $(CMINIMACS_LIB) -lrt

manybdtmptest_SOURCES = manybdtmptest.c
manybdtmptest_LDADD = $(CMINIMACS_LIB) -lrt

bitfftenctest_SOURCES = bitfftenctest.c
bitfftenctest_LDADD = $(CMINIMACS_LIB) -lrt


bitenctest_SOURCES = bitenctest.c
bitenctest_LDADD = $(CMINIMACS_LIB) -lrt

mulpar2test_SOURCES = mulpar2test.c
mulpar2test_LDADD = $(CMINIMACS_LIB) -lrt

bdtmptest_SOURCES = bdtmptest.c
bdtmptest_LDADD = $(CMINIMACS_LIB) -lrt


bdttest_SOURCES = bdttest.c
bdttest_LDADD = $(CMINIMACS_LIB) -lrt

loadreps_SOURCES = loadreps.c
loadreps_LDADD = $(CMINIMACS_LIB) -lrt

perfcli_SOURCES = perfcli.c
perfsrv_SOURCES = perfsrv.c

perfcli_LDADD = $(CMINIMACS_LIB) -lrt
perfsrv_LDADD = $(CMINIMACS_LIB) -lrt

symclient_SOURCES = symclient.c
symclient_LDADD = $(CMINIMACS_LIB) -lrt

testsymminimacs_SOURCES = testsymminimacs.c
testsymminimacs_LDADD = $(CMINIMACS_LIB) -lrt

perf_SOURCES = perf.c
perf_LDADD = $(CMINIMACS_LIB) -lrt 

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
