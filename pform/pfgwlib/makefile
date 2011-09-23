srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .


OSPECIAL =
OLIB = gwcontext.o gwinteger.o 
OUTDIR = .libs
ALIB = $(OUTDIR)/pfgwlib.a

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
	$(AR) ${ARFLAGS} $(ALIB) $^
	$(RANLIB) $(ALIB)

%.o:	%.c
	$(CC) $(CFLAGS) -I. -I../.. -I../pfgw -I../pfio -c -o $@ $<

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -I. -I../.. -I../pfgw -I../pfio -c -o $@ $<


%.o:	%.asm
	/usr/bin/nasm -felf -o $@ $<

maintainer-clean:	distclean
	rm -f configure
	autoconf
	
distclean: clean
	rm -f config.cache
	rm -f config.log
	rm -f config.status
	rm -f config.status.old
	rm -f Makefile
	rm -f config.h
	rm -f deps.d
	
clean:
	rm -f $(ALIB) $(OLIB)

cleanlibs:
	rm -f $(ALIB)

dependencies:
	$(CC) -MM -I. -I../.. $(srcdir)/*.c $(srcdir)/*.cpp >deps.d
	
