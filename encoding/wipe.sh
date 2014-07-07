#!/bin/sh
rm -rf  aclocal.m4 autom4te.cache/ config.h.in configure src/Makefile.in config.in COPYING missing install-sh INSTALL depcomp config.in
find . -iname Makefile.in | xargs rm -f
