srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

include ../../make.inc

OSPECIAL = 
ifeq ($(IS64),1)
OLIB = integer.o
endif

ifeq ($(IS64),0)
OLIB = integer.o mod.o
endif

OUTDIR	= .libs
ALIB = $(OUTDIR)/pfmath.a

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
	$(AR) ${ARFLAGS} $(ALIB) $^
	$(RANLIB) $(ALIB)

%.o:	%.c
	$(CC) $(CFLAGS) -I. -I../.. -c -o $@ $<

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -I. -I../.. -c -o $@ $<


%.o:	%.asm
	/usr/bin/nasm -fmacho -o $@ $<


maintainer-clean:	distclean
	rm -f configure
	autoconf
	rm -f lconfig.h.in
	autoheader
	
distclean: clean
	rm -f config.cache
	rm -f config.log
	rm -f config.status
	rm -f config.status.old
	rm -f Makefile
	rm -f nasmrule
	rm -f lconfig.h
	rm -f deps.d
	
clean:
	rm -f $(ALIB) $(OLIB)

cleanlibs:
	rm -f $(ALIB)

dependencies:
	$(CC) -MM -I. -I../.. $(srcdir)/*.cpp >deps.d
	
include deps.d
