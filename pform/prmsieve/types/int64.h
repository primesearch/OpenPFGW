#ifndef INT64_H
#define INT64_H

#if defined (_MSC_VER)
typedef __int64 int64;
#else
typedef long long int64;
#endif

#endif
