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

release:
	mkdir release
	cp -r osal/build/osal/* ./release
	cp -r ds/build/ds/* ./release
	cp -r encoding/build/encoding/* ./release
	cp -r carena/build/carena/* ./release
	cp -r math/build/math/* ./release
	cp -r cminimacs/build/cminimacs/* ./release
	cp -r caes/build/caes/caes/* ./release
	cp -r cinterp/build/cinterp/* ./release

.PHONY=all clean
