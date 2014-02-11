all: clean build

build:
	cd osal && make  depshape 
	cd ds && make  depshape
	cd encoding && make  depshape
	cd carena && make  depshape
	cd math && make  depshape
	cd cminimacs && make clean depshape
	cd caes && make  depshape
	cd cinterp && make  depshape

clean:
	cd osal && make clean
	cd ds && make clean
	cd carena && make clean
	cd math && make clean	
	cd encoding && make clean
	cd cminimacs && make clean
	cd caes && make clean
	cd cinterp && make clean
REV_DIR=release_`svnversion`
release:
	mkdir $(REV_DIR)
	cp -r osal/build/osal/* ./$(REV_DIR)
	cp -r ds/build/ds/* ./$(REV_DIR)
	cp -r encoding/build/encoding/* ./$(REV_DIR)
	cp -r carena/build/carena/* ./$(REV_DIR)
	cp -r math/build/math/* ./$(REV_DIR)
	cp -r cminimacs/build/cminimacs/* ./$(REV_DIR)
	cp -r caes/build/caes/* ./$(REV_DIR)
	cp -r cinterp/build/cinterp/* ./$(REV_DIR)

.PHONY=all clean release
