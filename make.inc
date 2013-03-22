IS64	= 1
ARCH	= OSX

ifeq ($(IS64),1)
CFLAGS	= -O3 -m64 -DX86_64 -D_64BIT -I../../packages/gmp/64bit
CXXFLAGS	= -O3 -m64 -DX86_64 -D_64BIT  -I../../packages/gmp/64bit -I../../pfconfig/headers
endif

ifeq ($(IS64),0)
CFLAGS	= -O3 -malign-double -m32 -I../../packages/gmp/32bit
CXXFLAGS	= -O3 -malign-double -m32 -I../../packages/gmp/32bit -I../../pfconfig/headers
endif
