IS64	= 1
ISDEBUG = 1
ARCH	= OSX

ifeq ($(IS64),1)
ifeq ($(ISDEBUG),1)
CFLAGS	= -g -m64 -mmacosx-version-min=10.14 -DX86_64 -D_64BIT -std=c++11 -stdlib=libc++ -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -I../../packages/gmp/64bit -I../primesieve
CXXFLAGS	= -g -m64 -mmacosx-version-min=10.14 -DX86_64 -D_64BIT -std=c++11 -stdlib=libc++ -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -I../../packages/gmp/64bit -I../primesieve -I../../pfconfig/headers
endif
ifeq ($(ISDEBUG),0)
CFLAGS	= -O3 -m64 -mmacosx-version-min=10.14 -DX86_64 -D_64BIT -std=c++11 -stdlib=libc++ -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -I../../packages/gmp/64bit -I../primesieve
CXXFLAGS	= -O3 -m64 -mmacosx-version-min=10.14 -DX86_64 -D_64BIT -std=c++11 -stdlib=libc++ -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -I../../packages/gmp/64bit -I../primesieve -I../../pfconfig/headers
endif
endif

# 32-bit is no longer supported
ifeq ($(IS64),0)
ifeq ($(ISDEBUG),1)
CFLAGS	= -g -m32 -mmacosx-version-min=10.5 -I../../packages/gmp/32bit
CXXFLAGS	= -g -m32 -mmacosx-version-min=10.5 -I../../packages/gmp/32bit -I../../pfconfig/headers
endif
ifeq ($(ISDEBUG),0)
CFLAGS	= -O3 -m32 -mmacosx-version-min=10.5 -I../../packages/gmp/32bit
CXXFLAGS	= -O3 -m32 -mmacosx-version-min=10.5 -I../../packages/gmp/32bit -I../../pfconfig/headers
endif
endif
