release:
	cd osal/linux && make -f linux.mak release && make install
	cd ds/linux && make -f linux.mak  release && make install
	cd math/linux && make -f linux.mak release  && make install
	cd encoding/linux && make -f linux.mak release  && make install
	cd utils/linux && make -f linux.mak release  && make install
	cd carena/linux && make -f linux.mak release  && make install
	cd cminimacs/linux && make -f linux.mak release  && make install
	cd caes/linux && make -f linux.mak release  && make install
	cd cinterp/linux && make -f linux.mak release  && make install

<<<<<<< HEAD
debug:
	cd osal/linux && make -f linux.mak debug && make install
	cd ds/linux && make -f linux.mak debug && make install
	cd math/linux && make -f linux.mak debug && make install
	cd encoding/linux && make -f linux.mak debug && make install
	cd utils/linux && make -f linux.mak debug && make install
	cd carena/linux && make -f linux.mak debug && make install
	cd cminimacs/linux && make -f linux.mak debug && make install
	cd caes/linux && make -f linux.mak debug && make install
	cd cinterp/linux && make -f linux.mak debug && make install
=======
debug	:
	cd osal/linux && make -f linux.mak && make install
	cd ds/linux && make -f linux.mak && make install
	cd math/linux && make -f linux.mak && make install
	cd encoding/linux && make -f linux.mak && make install
	cd utils/linux && make -f linux.mak && make install
	cd carena/linux && make -f linux.mak && make install
	cd cminimacs/linux && make -f linux.mak && make install
	cd caes/linux && make -f linux.mak && make install
	cd cinterp/linux && make -f linux.mak && make install
>>>>>>> d0e077b051ddc418440addc41a302712458e3921

clean:
	rm -rf debug release
	cd osal/linux && make -f linux.mak clean
	cd ds/linux && make -f linux.mak clean
	cd math/linux && make -f linux.mak clean
	cd encoding/linux && make -f linux.mak clean
	cd utils/linux && make -f linux.mak clean
	cd carena/linux && make -f linux.mak clean
	cd cminimacs/linux && make -f linux.mak clean
	cd caes/linux && make -f linux.mak clean
	cd cinterp/linux && make -f linux.mak clean

<<<<<<< HEAD
.PHONY: clean debug release
=======
.PHONY: clean debug release
>>>>>>> d0e077b051ddc418440addc41a302712458e3921
