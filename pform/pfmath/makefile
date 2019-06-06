srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

include ../../make.inc

OLIB = integer.o

OUTDIR	= .libs
ALIB = $(OUTDIR)/pfmath.a

CC			= gcc
CXX			= g++
AR			= ar
ARFLAGS		= rc
RANLIB		= ranlib
LD			= ld


.PHONY:	all clean cleanlibs

$(ALIB):	$(OLIB) $(OSPECIAL)
	mkdir -p $(OUTDIR)
	$(AR) ${ARFLAGS} $(ALIB) $^
	$(RANLIB) $(ALIB)

%.o:	%.c
	$(CC) $(CFLAGS) -I. -I../.. -c -o $@ $<

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -I. -I../.. -c -o $@ $<


ifeq ($(UNAME_S),Darwin)
%.o:	%.asm
	/usr/bin/nasm -fmacho -o $@ $<
else
%.o:	%.asm
	/usr/bin/nasm -felf -o $@ $<
endif

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
	

