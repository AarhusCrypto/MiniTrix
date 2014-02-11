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

.PHONY=all clean
