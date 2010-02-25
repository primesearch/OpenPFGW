#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfgluepch.h"
extern "C"
{
#include "primegen.h"
};
#include "primegenerator.h"

#define _USE_PHIL_ERAT
//#define _DEBUG_PHIL_ERAT

#if defined (_USE_PHIL_ERAT)
#include "prmsieve.h"
#endif

PrimeGenerator::PrimeGenerator()
   : pg(NULL)
{
#if defined (_USE_PHIL_ERAT)
   #if defined (_DEBUG_PHIL_ERAT)
   pg=new primegen;
   #endif
#else
   pg=new primegen;
#endif
   restart();
}

PrimeGenerator::~PrimeGenerator()
{
#if defined (_USE_PHIL_ERAT)
   erat_free();
#endif
   delete pg;
}

void PrimeGenerator::restart()
{
#if defined (_USE_PHIL_ERAT)
   erat_init();
   #if defined (_DEBUG_PHIL_ERAT)
       primegen_init(pg);
   #endif
#else
   primegen_init(pg);
   #if defined (_DEBUG_PHIL_ERAT)
       erat_init();
   #endif
#endif
}

void PrimeGenerator::skip(uint64 to)
{
#if defined (_USE_PHIL_ERAT)
   if (to > 128849018000ULL) // 128849018880 is max, due to 2^32*30 "bands" used in erat.c  If we seek over that, erat.c will crash (on 32 bit systems)
   {
      static bool once = false;
      if (!once)
      {
         once = true;
         fprintf (stderr, "\nERROR, primegen can not seek beyond 128849018000.\nWe will only seek that far, which may be WAY less than you expect.\nThis causes an infinite loop with ABC2 files using the \"prime\" modifier\n\n");
      }
      to = 128849018000ULL;
   }
   erat_skipto(to);
   #if defined (_DEBUG_PHIL_ERAT)
       primegen_skipto(pg,to);
   #endif
#else
   primegen_skipto(pg,to);
   #if defined (_DEBUG_PHIL_ERAT)
       erat_skipto(to);
   #endif
#endif
}

void PrimeGenerator::skip(uint32 to)
{
#if defined (_USE_PHIL_ERAT)
   erat_skipto(uint64(to));
   #if defined (_DEBUG_PHIL_ERAT)
       primegen_skipto(pg,uint64(to));
   #endif
#else
   primegen_skipto(pg,uint64(to));
   #if defined (_DEBUG_PHIL_ERAT)
       erat_skipto(uint64(to));
   #endif
#endif
}

void PrimeGenerator::next(uint64 &p)
{
#if defined (_USE_PHIL_ERAT)
   p=erat_next();
   #if defined (_DEBUG_PHIL_ERAT)
      uint64 p1=primegen_next(pg);
       if (p != p1)
         fprintf(stderr, "\nERAT next error!  %d returned, but expecting %d\n", p, p1);
   #endif
#else
   p=primegen_next(pg);
   #if defined (_DEBUG_PHIL_ERAT)
       uint64 p1=erat_next();
       if (p != p1)
         fprintf(stderr, "\nERAT next error!  %d returned, but expecting %d\n", p1, p);
   #endif
#endif
}

void PrimeGenerator::next(uint32 &p_ret)
{
#if defined (_USE_PHIL_ERAT)
   #if defined (_DEBUG_PHIL_ERAT)
      uint64 p=erat_next();
      uint64 p1=primegen_next(pg);
       if (p != p1)
         fprintf(stderr, "\nERAT next error!  %d returned, but expecting %d\n", p, p1);
      p_ret = (uint32)p;
   #else
      p_ret = (uint32)erat_next();
   #endif
#else
   #if defined (_DEBUG_PHIL_ERAT)
      uint64 p=primegen_next(pg);
       uint64 p1=erat_next();
       if (p != p1)
         fprintf(stderr, "\nERAT next error!  %d returned, but expecting %d\n", p1, p);
      p_ret = (uint32)p;
   #else
      p_ret = (uint32)primegen_next(pg);
   #endif
#endif
}
