include $(top_srcdir)/cminimacs.am

bin_PROGRAMS = test testminimacs client testencoder testfft clientfft

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
