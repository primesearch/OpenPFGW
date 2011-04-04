srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

OSPECIAL =
OLIB = ipfsymbol.o pfboolean.o \
	pfforwarditerator.o pflibpch.o \
	pflist.o pflistnode.o pfstring.o pfsymboltable.o \
	crc32.o erat_mod.o timer.o
OUTDIR = .libs
ALIB = $(OUTDIR)/pflib.a

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
	
include deps.d
