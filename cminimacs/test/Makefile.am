include $(top_srcdir)/cminimacs.am

bin_PROGRAMS = test testminimacs client testencoder testfft clientfft perf testsymminimacs symclient

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
