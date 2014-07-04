DBGDIR=${PWD}/../../debug
RELDIR=${PWD}/../../release
FILES=configure.ac  ds.am  headers  linux  Makefile.am  src  test
debug: autotools
	../configure --prefix=${DBGDIR} CFLAGS="-O0 -g3"

release: autotools
	../configure --prefix=${RELDIR} CFLAGS="-O3 -mtune=corei7 -march=corei7"

autotools:
	cd .. && aclocal && autoconf && autoheader && automake --add-missing

clean:
	find . -not -iname "linux.mak" -not -iname "." -not -name ".svn" -maxdepth 1| xargs rm -rf 
	cd .. && rm -rf aclocal.m4 config.in configure depcomp install-sh missing autom4te.cache/ 
	cd .. && find . -iname Makefile.in | xargs rm -f

.PHONY: debug release autotools clean