CFLAGS+= -m64 -mtune=corei7 -march=corei7 -DSTATS_ON=1 -O2
build:
	cd osal && make depshape CFLAGS="$(CFLAGS)"
	cd encoding && make depshape CFLAGS="$(CFLAGS)"
	cd ds && make  depshape CFLAGS="$(CFLAGS)"
	cd carena && make  depshape CFLAGS="$(CFLAGS)"
	cd math && make  depshape CFLAGS="$(CFLAGS)"
	cd cminimacs && make depshape CFLAGS="$(CFLAGS)"
	cd caes && make depshape CFLAGS="$(CFLAGS)" CFLAGS+="-O0"
	cd cinterp && make depshape CFLAGS="$(CFLAGS)"
	cd tinyot && make depshape CFLAGS="$(CFLAGS)"

depshape: all
all: clean build

clean:
	cd osal && make clean
	cd ds && make clean
	cd carena && make clean
	cd math && make clean	
	cd encoding && make clean
	cd cminimacs && make clean
	cd caes && make clean
	cd cinterp && make clean
	cd tinyot && make clean

up:
	svn commit -m "Preparing release"
	svn up 

REV_DIR=parvusfortis_r`svnversion`
release: up build
	mkdir $(REV_DIR)
	cp -r osal/build/osal/* ./$(REV_DIR)
	cp -r ds/build/ds/* ./$(REV_DIR)
	cp -r encoding/build/encoding/* ./$(REV_DIR)
	cp -r carena/build/carena/* ./$(REV_DIR)
	cp -r math/build/math/* ./$(REV_DIR)
	cp -r cminimacs/build/cminimacs/* ./$(REV_DIR)
	cp -r caes/build/caes/* ./$(REV_DIR)
	cp -r cinterp/build/cinterp/* ./$(REV_DIR)
	cp -r tinyot/build/tinyot/* ./$(REV_DIR)

.PHONY=build all clean release depshape
