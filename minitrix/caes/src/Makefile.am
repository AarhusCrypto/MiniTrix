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

include $(top_srcdir)/caes.am
bin_PROGRAMS = caesfft caessym multirun manyrun monitor multifftrun bitw\
	multibitwrun multirunmul multirunand multibitwfftrun multibitwfftmulparrun\
	mulpar2fft mulpar2mxt mulpar2bit mulpar2bitfft start manymulpar2bitfft


start_SOURCES = start.c
start_LDADD = $(DEPS_LIB) #-lrt

manymulpar2bitfft_SOURCES = manymulpar2bitfft.c caesmp.c
manymulpar2bitfft_LDADD = $(DEPS_LIB) #-lrt

mulpar2bitfft_SOURCES = mulpar2bitfft.c caesmp.c
mulpar2bitfft_LDADD = $(DEPS_LIB) #-lrt

mulpar2bit_SOURCES = mulpar2bit.c caesmp.c
mulpar2bit_LDADD = $(DEPS_LIB) #-lrt

mulpar2mxt_SOURCES = mulpar2mxt.c caesmp.c
mulpar2mxt_LDADD = $(DEPS_LIB) #-lrt

mulpar2fft_SOURCES = mulpar2fft.c caesmp.c
mulpar2fft_LDADD = $(DEPS_LIB) #-lrt

multibitwfftmulparrun_SOURCES = multibitwfftmulparrun.c caesmp.c
multibitwfftmulparrun_LDADD = $(DEPS_LIB) #-lrt


multirunand_SOURCES = multirunand.c
multirunand_LDADD = $(DEPS_LIB) #-lrt

multirunmul_SOURCES = multirunmul.c
multirunmul_LDADD = $(DEPS_LIB) #-lrt

multibitwrun_SOURCES = multibitwrun.c caes.c
multibitwrun_LDADD = $(DEPS_LIB) #-lrt

multibitwfftrun_SOURCES = multibitwfftrun.c caes.c
multibitwfftrun_CFLAGS = $(AM_CFLAGS) 
multibitwfftrun_LDADD = $(DEPS_LIB) #-lrt

bitw_SOURCES = mainbw.c caes.c
bitw_LDADD = $(DEPS_LIB) #-lrt

multifftrun_SOURCES = multirun.c caes.c
multifftrun_LDADD = $(DEPS_LIB) #-lrt
multifftrun_CFLAGS = $(AM_CFLAGS) -DFFT

monitor_SOURCES = monitor.c
monitor_LDADD = $(DEPS_LIB) #-lrt
monitor_CFLAGS = $(AM_CFLAGS) 


manyrun_SOURCES = manyrun.c caes.c
manyrun_LDADD = $(DEPS_LIB) #-lrt

multirun_SOURCES = multirun.c caes.c
multirun_LDADD = $(DEPS_LIB) #-lrt	

caessym_SOURCES = caes.c caessym.c 
caessym_LDADD = $(DEPS_LIB) #-lrt

caesfft_SOURCES = caes.c mainfft.c
caesfft_LDADD = $(DEPS_LIB) #-lrt