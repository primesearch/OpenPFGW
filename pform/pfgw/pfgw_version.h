// This single file is the file to edit when a version is changed.  PFGW simply
// printf's the VERSION_STRING macro when it starts

#if defined (_MSC_VER)
 #define	SHORT_OS_NAME	"Win"
 #define LONG_OS_NAME	"Windows"
#else
#if defined __BEOS__
 #define SHORT_OS_NAME	"BeOS"
 #define	LONG_OS_NAME	"BeOS R5 x86"
#else
#if defined __APPLE__
 #define SHORT_OS_NAME	"Mac"
 #define LONG_OS_NAME	"MacIntel"
#else
 #define SHORT_OS_NAME	"x86"
 #define LONG_OS_NAME	"Pentium and compatibles"
#endif
#endif
#endif

// 0-dev 1-stable 2-release

//#define	RELEASE_LEVEL	0
//#define	RELEASE_LEVEL	1
#define	RELEASE_LEVEL	2

// for dev level
#define DEVELOPMENT_VERSION      "20091201"
// for sta level
#define	STABLE_VERSION          "20091201"
#define	RELEASE_CANDIDATE       "1f"
// for sta and release level
#define	RELEASE_VERSION         "3.2.4"
// for "special" builds, debugging builds, ...  This will NORMALLy simply be a "" empty string.

//
// Put all of the #if(RELEASE_LEVEL==?) logic here, and not inside of the .cpp file.
//
// The VERSION_STRING is the "correct" thing to printf to show proper build version.
//

#if(RELEASE_LEVEL==2)
 #define VERSION_STRING RELEASE_VERSION" for "LONG_OS_NAME
 #define WINPFGW_ABOUT_VERSION_STRING RELEASE_VERSION" for "LONG_OS_NAME
#elif(RELEASE_LEVEL==1)	
 #define VERSION_STRING STABLE_VERSION"."SHORT_OS_NAME"_Stable (v"RELEASE_VERSION" RC"RELEASE_CANDIDATE")"
 #define WINPFGW_ABOUT_VERSION_STRING STABLE_VERSION"."SHORT_OS_NAME"_Stable (v"RELEASE_VERSION" RC"RELEASE_CANDIDATE")"
#elif(RELEASE_LEVEL==0)	
 #define VERSION_STRING DEVELOPMENT_VERSION"."SHORT_OS_NAME"_Dev (Beta 'caveat utilitor')"
 #define WINPFGW_ABOUT_VERSION_STRING DEVELOPMENT_VERSION"."SHORT_OS_NAME"_Dev (Beta)"
#endif	
