// This single file is the file to edit when a version is changed.  PFGW simply
// printf's the VERSION_STRING macro and the SPECIAL_BUILD macro when it starts

#if defined (_MSC_VER)
 #define	SHORT_OS_NAME	"Win"
 #define LONG_OS_NAME	"Windows"
#else
#if defined __BEOS__
 #define SHORT_OS_NAME	"BeOS"
 #define	LONG_OS_NAME	"BeOS R5 x86"
#else
 #define SHORT_OS_NAME	"x86"
 #define LONG_OS_NAME	"Pentium and compatibles"
#endif
#endif

// 0-dev 1-stable 2-release

//#define	RELEASE_LEVEL	0
//#define	RELEASE_LEVEL	1
#define	RELEASE_LEVEL	2

// for dev level
#define DEVELOPMENT_VERSION     "20041020"
// for sta level
#define	STABLE_VERSION		"20050130"
#define	RELEASE_CANDIDATE	"1e"
// for sta and release level
#define	RELEASE_VERSION		"3.0.0"
// for "special" builds, debugging builds, ...  This will NORMALLy simply be a "" empty string.

#define SPECIAL_BUILD
// to enable a special build, decomment the following line. Release builds SHOULD NOT define this.
// This is required for gcc, which doesn't allow string tests on #defines
//#define	SPECIAL_ENABLED

#ifndef SPECIAL_ENABLED
#undef SPECIAL_BUILD
#define SPECIAL_BUILD		"[FFT v25.10]"
#endif

#if defined (JBC_BUILD)
#undef  SPECIAL_BUILD
#define SPECIAL_BUILD		"[JBC code]"
#undef  SPECIAL_ENABLED
#define	SPECIAL_ENABLED
#endif

//
// Put all of the #if(RELEASE_LEVEL==?) logic here, and not inside of the .cpp file.
//
// The VERSION_STRING is the "correct" thing to printf to show proper build version.
//

#if(RELEASE_LEVEL==2)
 #ifdef SPECIAL_ENABLED
  #error "SPECIAL_ENABLED must not be defined for RELEASE builds.  If you REALLY want to have a release/special build, then comment out the error line"
 #endif
 #define VERSION_STRING RELEASE_VERSION" for "LONG_OS_NAME
 #define WINPFGW_ABOUT_VERSION_STRING RELEASE_VERSION" for "LONG_OS_NAME
#elif(RELEASE_LEVEL==1)	
 #define VERSION_STRING STABLE_VERSION"."SHORT_OS_NAME"_Stable (v"RELEASE_VERSION" RC"RELEASE_CANDIDATE")"
 #define WINPFGW_ABOUT_VERSION_STRING STABLE_VERSION"."SHORT_OS_NAME"_Stable (v"RELEASE_VERSION" RC"RELEASE_CANDIDATE")"
#elif(RELEASE_LEVEL==0)	
 #define VERSION_STRING DEVELOPMENT_VERSION"."SHORT_OS_NAME"_Dev (Beta 'caveat utilitor')"
 #define WINPFGW_ABOUT_VERSION_STRING DEVELOPMENT_VERSION"."SHORT_OS_NAME"_Dev (Beta)"
#endif	
