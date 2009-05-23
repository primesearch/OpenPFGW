include ../Makedefs

all: tests

CSRC=eratspeed.c primespeed.c
OBJS=eratspeed.o primespeed.o
TESTS=eratspeed primespeed

depend:
	-${MAKEDEPEND} ${CSRC}

clean:
	-rm ${OBJS}
	-rm ${TESTS}

tests: ${TESTS}

eratspeed: eratspeed.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

eratspeed.o: eratspeed.c 
	${CC} ${CC_OUT} $@ ${CC_INP} $<

compri: compri.o \
	../.libs/prmsieve.a
	${LINK} ${LINK_OUT} $@ ${LINK_INP} $+

compri.o: compri.c
	${CC} ${CC_OUT} $@ ${CC_INP} $<

include ${MAKEDEPFILE}