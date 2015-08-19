############################################################
#
# Author: Rasmus Winther Zakarias
# Project: Operating System Abstraction Layer for MiniTrix
# Date: Mon 24. Jun 2013
#
# Description: Meta-makefile to call autotools to build 
# configure scripts, execute configure scripts and do 
# make install.
#
# Usage: 
#  [*] Build release: make release [BUILDDIR=path]
#  [*] Build debug: make debug [BUILDDIR=path]
# 
# [BUILDDIR=path] is an optional argument stipulating  
# the --prefix for configure.
#
############################################################


############################################################
#
# BUILDDIR SETUP
#
############################################################
ifdef $(BUILDDIR)
echo "Building at ${BUILDDIR}"
else
BUILDDIR=${PWD}/../../system
endif 


############################################################
#
# Targets 
#
############################################################

#----------------------------------------
all: release  
#----------------------------------------


#----------------------------------------
debug: autotools
	../configure --prefix=${BUILDDIR} CFLAGS="-O0 -g3" LDFLAGS="-L${BUILDDIR}/lib"
	make install
#----------------------------------------


#----------------------------------------
release: autotools
	../configure --prefix=${BUILDDIR} CFLAGS="-O3 -mtune=corei7 -march=corei7" LDFLAGS="-L${BUILDDIR}/lib"
	make install
#----------------------------------------


#----------------------------------------
autotools:
	cd .. && aclocal && autoconf && autoheader && automake --add-missing
#----------------------------------------


#----------------------------------------
clean:
	find . -maxdepth 1 -not -iname "linux.mak" -not -iname "." -not -name ".svn" | xargs rm -rf 
	cd .. && rm -rf compile aclocal.m4 config.in configure depcomp install-sh missing autom4te.cache/ 
	cd .. && find . -iname Makefile.in | xargs rm -f
#----------------------------------------


.PHONY: all debug release autotools clean