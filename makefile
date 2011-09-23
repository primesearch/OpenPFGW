
srcdir=.

CXX		=	g++
include make.inc

pfgw32:	baselib integer fft pfoo io entrypoint
	${CXX} ${CXXFLAGS}	\
		pform/pfgw/.libs/pfgw_main.a  pform/pfio/.libs/pfio.a pform/pfoo/.libs/pfoo.a pform/pfglue/.libs/pfglue.a pform/pfgwlib/.libs/pfgwlib.a \
		pform/pfmath/.libs/pfmath.a pform/pflib/.libs/pflib.a pform/prmsieve/.libs/prmsieve.a \
		packages/gwnum/32bit/gwnum.a  packages/gwnum/32bit/gwnum.ld -lpthread -lstdc++ -lgmp -lgmpxx -static -o pfgw32s

maintainer-clean:	distclean
	rm -f configure
	autoconf
	rm -f config.h.in
	autoheader
	
distclean:	clean
	rm -f config.cache
	rm -f config.log
	rm -f config.status
	rm -f config.status.old
	rm -f config.h
	rm -f stdtypes.h
	rm -f pflib.h
	rm -f pfoo.h
	rm -f pfio.h
	rm -f pfmath.h
	rm -f pfgwlib.h
	rm -f Makefile
	${MAKE} -C pform/pflib distclean
	${MAKE} -C pform/pfmath distclean
	${MAKE} -C pform/pfgwlib distclean
	${MAKE} -C pform/pfoo distclean
	${MAKE} -C pform/pfio distclean
	${MAKE} -C pform/pfgw distclean
	
clean:
	rm -f pfgw32 pfgw64
	${MAKE} -C pform/pflib clean
	${MAKE} -C pform/pfmath clean
	${MAKE} -C pform/pfgwlib clean
	${MAKE} -C pform/pfoo clean
	${MAKE} -C pform/pfio clean
	${MAKE} -C pform/pfgw clean
	
dependencies:
	${MAKE} -C pform/pflib dependencies
	${MAKE} -C pform/pfmath dependencies
	${MAKE} -C pform/pfgwlib dependencies
	${MAKE} -C pform/pfoo dependencies
	${MAKE} -C pform/pfio dependencies
	${MAKE} -C pform/pfgw dependencies

baselib:
	${MAKE} -C pform/pflib
	
integer:
	${MAKE} -C pform/pfmath
	
fft:
	${MAKE} -C pform/pfgwlib

pfoo:
	${MAKE} -C pform/pfoo
	
io:
	${MAKE} -C pform/pfio

entrypoint:
	${MAKE} -C pform/pfgw
	
gmp:
	${MAKE} -C packages/gmp

