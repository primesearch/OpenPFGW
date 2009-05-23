#ifndef UINT64_H
#define UINT64_H

#if defined (_MSC_VER)
typedef unsigned __int64 uint64;
#else
typedef unsigned long long uint64;
#endif

#endif
