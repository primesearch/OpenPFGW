srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

OSPECIAL = 
OLIB = algebra.o exponentiation.o exponentiator.o expr.o f_decimal.o f_factor.o \
	f_prime.o f_trivial.o f_vector.o factorarray.o factornode.o nminus1exponentiator.o \
	nplus1exponentiator.o pffactorizationsymbol.o pffunctionsymbol.o pfintegersymbol.o \
	pfiterativesymbol.o pfsamplersymbol.o pfstringsymbol.o powerbuffer.o primeserver.o \
	recursionbuffer.o tests.o f_fibonacci.o f_repunit.o f_cyclotomic.o f_prevprime.o \
	f_issquare.o f_endminus1.o f_endplus1.o h_mobius.o h_primitive.o f_nextprime.o \
	f_gcd.o f_binomial.o f_if.o f_smarandache.o f_sequence.o f_length.o pfintsymbol.o
OUTDIR = .libs
ALIB = $(OUTDIR)/pfoo.a

include ../../make.inc
CC			= gcc
CXX			= g++
AR			= ar
ARFLAGS		= rc
RANLIB		= ranlib
LD			= ld


.PHONY:	all clean cleanlibs

all:	$(ALIB)

$(ALIB):	$(OLIB) $(OSPECIAL)
	mkdir -p $(OUTDIR)
	$(AR) ${ARFLAGS} $(ALIB) $(OLIB) $(OSPECIAL)
	$(RANLIB) $(ALIB)

%.o:	%.c
	$(CC) $(CFLAGS) -I. -I../.. -c -o $@ $<

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -I. -I../.. -I../pfgw -c -o $@ $<

maintainer-clean:	distclean
	rm -f configure
	autoconf
	
distclean: clean
	rm -f config.cache
	rm -f config.log
	rm -f config.status
	rm -f config.status.old
	rm -f Makefile
	rm -f deps.d

clean:
	rm -f $(ALIB) $(OLIB)

cleanlibs:
	rm -f $(ALIB)

dependencies:
	$(CC) -MM -I. -I../.. $(srcdir)/*.cpp >deps.d
	

