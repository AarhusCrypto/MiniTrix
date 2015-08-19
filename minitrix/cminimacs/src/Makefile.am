# Copyright (c) 2013, Rasmus Lauritsen, Aarhus University
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software

#    must display the following acknowledgement:
#    This product includes software developed by the Aarhus University.
# 4. Neither the name of the Aarhus University nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY Rasmus Lauritsen at Aarhus University ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Rasmus Lauritsen at Aarhus University BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Created: 2013-07-25

# Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

# Changes: 
# 2013-07-25 12:28: Initial version created

# Example creating one binary executable from main.c

SUBDIRS=minimacs

include $(top_srcdir)/cminimacs.am
lib_LIBRARIES= libcminimacs.a
libcminimacs_a_SOURCES = minimacs/generic_minimacs.c\
	minimacs/symmetric_minimacs.c\
	minimacs/bitwiseand_minimacs.c\
	minimacs/genericmulpar_minimacs.c\
	minimacs/minimacs_pre.c\
	minimacs/minimacs.c\
	minimacs/minimacs_rep.c\
	reedsolomon/reedsolomon.c\
	reedsolomon/minimacs_enc_fft.c\
	reedsolomon/minimacs_bit_encoder.c\
	reedsolomon/minimacs_bitfft_encoder.c\
	reedsolomon/minimacs_combi_encoder.c\
	bedoza/bedoza_mac.c\
	ass/ass.c \
	minimacs/bitwiseandmulpar_minimacs.c \
	minimacs/bitwisemulpar2_minimacs.c

bin_PROGRAMS = lsrep genpre

lsrep_SOURCES = minimacs/lsrep.c
lsrep_LDADD=$(CMINIMACS_LIB) libcminimacs.a -lds -lencoding -lmath -losal -lpthread


genpre_SOURCES = minimacs/genpre.c 
genpre_LDADD = libcminimacs.a
#genpre_LDFLAGS = -static

#libcminimacs_a_LIBADD = $(DEPS_OSAL_A)


