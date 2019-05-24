srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

OSPECIAL =
OLIB = CpuInfo.o Erat.o EratBig.o EratMedium.o EratSmall.o IteratorHelper.o \
	MemoryPool.o ParallelSieve.o PreSieve.o PrimeGenerator.o PrimeSieve.o PrintPrimes.o \
	SievingPrimes.o Wheel.o api-c.o api.o iterator-c.o nthPrime.o popcount.o
OUTDIR = .libs
ALIB = $(OUTDIR)/primesieve.a

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
	$(CXX) $(CXXFLAGS) -I. -I../.. -c -o $@ $<

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
	
