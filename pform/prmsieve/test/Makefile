include ../../../make.inc
include ../Makedefs

all: tests

CSRS=eratskip.c eratlist.c eratrestart.c erattest.c test7.c
OBJS=eratskip.o eratlist.o eratrestart.o erattest.o test7.o
TESTS=eratskip eratlist eratrestart erattest test7

depend:
	-${MAKEDEPEND} ${CSRC}

clean:
	-rm ${OBJS}
	-rm ${TESTS}

tests: ${TESTS}

eratlist: eratlist.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

eratskip: eratskip.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

eratrestart: eratrestart.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

erattest: erattest.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

eratlist.o: eratlist.c
	${CC} ${CC_OUT} $@ ${CC_INP} $<

eratskip.o: eratskip.c
	${CC} ${CC_OUT} $@ ${CC_INP} $<

eratrestart.o: eratrestart.c
	${CC} ${CC_OUT} $@ ${CC_INP} $<

erattest.o: erattest.c
	${CC} ${CC_OUT} $@ ${CC_INP} $<

test7:
	@echo no CP7 test yet

include ${MAKEDEPFILE}
