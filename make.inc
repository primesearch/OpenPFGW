IS64	= 0

ifeq ($(IS64),1)
CFLAGS	= -O2 -m64 -DX86_64 -D_64BIT -I../../packages/gmp/64bit
CXXFLAGS	= -O2 -m64 -DX86_64 -D_64BIT  -I../../packages/gmp/64bit
endif

ifeq ($(IS64),0)
CFLAGS	= -O2 -malign-double -m32 -I../../packages/gmp/32bit
CXXFLAGS	= -O2 -malign-double -m32 -I../../packages/gmp/32bit -ffriend-injection
endif
