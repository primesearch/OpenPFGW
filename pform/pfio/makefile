srcdir=.
vpath %.c .
vpath %.cpp .
vpath %.h .
vpath %.asm .

OSPECIAL =
OLIB = pfcpapfile.o pffile.o pfccfile.o pfnewpgenfile.o pfoutput.o pfconsoleoutput.o pfini.o \
	pfabcfile.o pfabc2file.o pfprzfile.o pfscriptfile.o pfscriptfns.o pfabcdfile.o \
	pfdecimalfile.o pfcheckfile.o prz.o pfabctaskcompleted.o pffilesymbol.o
OUTDIR = .libs
ALIB = $(OUTDIR)/pfio.a

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
	
