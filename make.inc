UNAME_S := $(shell uname -s)

CXXFLAGS = -m64 -fno-builtin -DX86_64 -D_64BIT -std=c++11 

ifeq ($(DEBUG),yes)
   CXXFLAGS +=  -g
else
   CXXFLAGS += -g
endif

CXXFLAGS += -I../../packages/gmp/64bit -I../../pfconfig/headers  -I../primesieve

ifeq ($(UNAME_S),Darwin)
#   CXXFLAGS+=-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
endif

ifeq ($(UNAME_S),Darwin)
   LDFLAGS+=-lpthread
else
   LDFLAGS+=-no-pie -lpthread -ldl -lstdc++
endif
