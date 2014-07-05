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

