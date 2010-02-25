#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#if !defined (__NO_PF_HEADERS__)
#include "pfiopch.h"
#endif

#if !defined (_MSC_VER)

// If someone else want to insert these for *nix builds (along with the part of PFGW using them which
// is the NETWORK2 file format), then they can do so (but BE SURE to send us the modified files).
// I am not doing so at this time.
bool bLoad_zLibDLL() { return false; }
bool PFGW_deflate(uint8 *compr, uint8 *uncompr, uint32 *comprLen, uint32 uncomprLen) { return false; }
bool PFGW_inflate(uint8 *uncompr, uint8 *compr, uint32 comprLen, uint32 *uncomprLen) { return false; }
void Free_zLibDLL() {}

#else

#include <windows.h>
#include "pfgw_zlib.h"

static HMODULE h_zLibInst;
static bool b_zLibInited;
static bool b_zLibValid;

// These are needed to be "imported"  We do ALL work dynamically.  So that zLib.dll can be there or not, and the
// app will work. If zLib is there, then network traffic will only be about 10% or less.  If it is not there,
// then traffic will be higher.  Also, if zLib is there, a client can request MORE work at a singel time (since
// the 48k sized max packet can now hold MUCH more.

//_deflateEnd
//_deflateParams
//_deflate
//_deflateInit_
//_inflateEnd
//_inflate
//_inflateInit_

extern "C" {
   // type define the zLib functions here
   typedef int (CDECL *_deflateEnd_t)     (z_streamp strm);
   typedef int (CDECL *_deflateParams_t)  (z_streamp strm,int level, int strategy);
   typedef int (CDECL *_deflate_t)     (z_streamp strm, int flush);
   typedef int (CDECL *_deflateInit__t)   (z_streamp strm, int level, const char *version, int stream_size);
   typedef int (CDECL *_inflateEnd_t)     (z_streamp strm);
   typedef int (CDECL *_inflate_t)     (z_streamp strm, int flush);
   typedef int (CDECL *_inflateInit__t)   (z_streamp strm, const char *version, int stream_size);
};

// Now declare the function pointers.
_deflateEnd_t     _deflateEnd_fp;
_deflateParams_t  _deflateParams_fp;
_deflate_t        _deflate_fp;
_deflateInit__t      _deflateInit__fp;
_inflateEnd_t     _inflateEnd_fp;
_inflate_t        _inflate_fp;
_inflateInit__t      _inflateInit__fp;

#define _deflateInit_fp(strm, level) \
        _deflateInit__fp((strm), (level),       ZLIB_VERSION, sizeof(z_stream))
#define _inflateInit_fp(strm) \
        _inflateInit__fp((strm),                ZLIB_VERSION, sizeof(z_stream))


#define bLOADBAIL do{ FreeLibrary(h_zLibInst); return(b_zLibValid=false); }while(0)

bool bLoad_zLibDLL()
{
   if (b_zLibInited)
      return b_zLibValid;

   // we only want to get to this part of the code one time.  For a user to "install" zlib.dll, they must exit
   // pfgw or winpfgw.
   b_zLibInited = true;

   // now load the .DLL if it is here, if not, then make sure b_zLibValid is false, and return false.
   h_zLibInst = LoadLibrary("zlib1.dll");
   if (h_zLibInst == 0)
      return (b_zLibValid=false);

   // Load the functions.
   _deflateEnd_fp = (_deflateEnd_t)GetProcAddress(h_zLibInst, "deflateEnd");
   if (!_deflateEnd_fp)
      bLOADBAIL;
   _deflateParams_fp = (_deflateParams_t)GetProcAddress(h_zLibInst, "deflateParams");
   if (!_deflateParams_fp)
      bLOADBAIL;
   _deflate_fp = (_deflate_t)GetProcAddress(h_zLibInst, "deflate");
   if (!_deflate_fp)
      bLOADBAIL;
   _deflateInit__fp = (_deflateInit__t)GetProcAddress(h_zLibInst, "deflateInit_");
   if (!_deflateInit__fp)
      bLOADBAIL;
   _inflateEnd_fp = (_inflateEnd_t)GetProcAddress(h_zLibInst, "inflateEnd");
   if (!_inflateEnd_fp)
      bLOADBAIL;
   _inflate_fp = (_inflate_t)GetProcAddress(h_zLibInst, "inflate");
   if (!_inflate_fp)
      bLOADBAIL;
   _inflateInit__fp = (_inflateInit__t)GetProcAddress(h_zLibInst, "inflateInit_");
   if (!_inflateInit__fp)
      bLOADBAIL;

   // Got all functions, so all is well.
   return (b_zLibValid=true);
}

void Free_zLibDLL()
{
   if (b_zLibValid)
      FreeLibrary(h_zLibInst);
   h_zLibInst = 0;
   b_zLibInited = b_zLibValid = false;
}

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        PFPrintfStderr("%s error: %d\n", msg, err); \
        return false; \
    } \
}
/* ===========================================================================
 * deflate() with large buffers and dynamic change of compression level
 */
bool PFGW_deflate(uint8 *compr, uint8 *uncompr, uint32 *comprLen, uint32 uncomprLen)
{
    z_stream c_stream; /* compression stream */
    int err;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = _deflateInit_fp(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)*comprLen;

    /* Adjustments compressing mode: Slightly better compression, but also more CPU used. */
//    _deflateParams_fp(&c_stream, Z_BEST_COMPRESSION, Z_FILTERED);
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = _deflate_fp(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = _deflate_fp(&c_stream, Z_FINISH);
    if (err != Z_STREAM_END)
   {
        PFPrintfStderr("deflate should report Z_STREAM_END\n");
      return false;
    }
    err = _deflateEnd_fp(&c_stream);
    CHECK_ERR(err, "deflateEnd");

   if (uncomprLen > c_stream.total_out)
   {
      *comprLen = c_stream.total_out;
      return true;
   }
   *comprLen = c_stream.total_out;
   // compression made data larger (happens for really small data like "GN 100\n"
   return false;
}

/* ===========================================================================
 * inflate() with large buffers
 */
bool PFGW_inflate(uint8 *uncompr, uint8 *compr, uint32 comprLen, uint32 *uncomprLen)
{
    int err;
    z_stream d_stream; /* decompression stream */

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = _inflateInit_fp(&d_stream);
    CHECK_ERR(err, "inflateInit");

    for (;;)
   {
        d_stream.next_out = uncompr;            /* discard the output */
      d_stream.avail_out = (uInt)*uncomprLen;
        err = _inflate_fp(&d_stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END)
         break;
        CHECK_ERR(err, "large inflate");
    }

    err = _inflateEnd_fp(&d_stream);
    CHECK_ERR(err, "inflateEnd");

   /*
    if (d_stream.total_out != (uInt)(2 * (*uncomprLen) + comprLen/2))
   {
        PFPrintfStderr("bad large inflate: %ld\n", d_stream.total_out);
      return false;
    }
   */
   *uncomprLen = d_stream.total_out;
   return true;
}

#endif
