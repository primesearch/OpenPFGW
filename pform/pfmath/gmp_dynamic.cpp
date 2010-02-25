//
//  WARNING, VC takes a LOOOOONG time to compile this file in release mode.
//
// Also note that this file (causes about 12kb of extra code/data to be included in the project)
//

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#if defined (_MSC_VER)     // Only useful code for VC

// If using these wrapper functions in software OTHER than the OpenPFGW project (which has the PFIO library),
// then simply uncomment the next line to change the OpenPFGW specific code to generic C code.

#if defined(__THIS_IS_NOT_OPENPFGW__)
#define PFPrintfLog printf
#else
// OpenPFGW compiles with this
#include "pfmathpch.h"
#include "pfio.h"
#endif

#include <windows.h>
#include <stdio.h>

#define _GMP_STATIC_DYNAMIC_FUNCTIONS_
#include "gmp_dynamic.h"
#undef _GMP_STATIC_DYNAMIC_FUNCTIONS_

// Used by our static constructor class.
unsigned GMP__Constructor_Class::m_nCnt;

static bool g_bGMPUsingDLL;
static bool g_bNoAlloca;
static bool g_bCygwinDLL;
void ShowGMPLinkage()
{
   if (g_bGMPUsingDLL)
   {
      if (g_bNoAlloca)
         PFPrintfLog ("***** Using functions from the GMP DLL (w/o alloca)!!!\n\n");
      else
      {
         if (g_bCygwinDLL)
            PFPrintfLog ("***** Using functions from the Cygwin GMP DLL!!!\n\n");
         else
            PFPrintfLog ("***** Using functions from the MinGW GMP DLL!!!\n\n");
      }
   }
   else
      PFPrintfLog ("***** Using functions from the static linked GMP library\n\n");
}

//*****************************************************************************************************
// WARNING!!! Only uncomment the functions in Connect_to_Static_GMP_Functions() which your app actually
// uses.  If you uncomment ALL of them, then you will link in the ENTIRE GMP library (probably not what
// you wanted.  A quick way to see what is needed, is to uncomment the __GMP__INCLUDE__NOTHING line, and
// let the linker tell you.  We HAVE to be the linker here.  We could use the shotgun method, and simply
// link in the WHOLE lib, but why.  By simply getting the "symbol not found" error messages from the
// linker, we can link in only what is needed (like the linker does.
//*****************************************************************************************************
//#define __GMP__INCLUDE__NOTHING

#if defined (__GMP__INCLUDE__NOTHING)
  // define there here, so they do not get in our way of seeing which GMP functions are needed to link in
  void Initialize_Dynamic_GMP_System(){}
  void Free_Dynamic_GMP_System(){}
  void Connect_to_Static_GMP_Functions(){}
#else

#if 0
// Link in the libraries
extern "C"
{
   int _alloca[1];
   size_t __gmpz_sizeinbase(mpz_srcptr, int) {return 0;}
   int __gmpz_jacobi (mpz_srcptr, mpz_srcptr) {return 0;}
   long int __gmpz_get_si (mpz_srcptr) {return 0;}
   int __gmpz_cmp_si (mpz_srcptr, signed long int) {return 0;}
   int __gmpz_cmp (mpz_srcptr, mpz_srcptr) {return 0;}
   unsigned long int __gmpz_tdiv_qr_ui (mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int) {return 0;}
   void __gmpz_tdiv_qr (mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_tdiv_q_2exp (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_tdiv_q (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_sub_ui (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_sub (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_sqrt (mpz_ptr, mpz_srcptr) {}
   void __gmpz_set_ui (mpz_ptr, unsigned long int) {}
   int __gmpz_set_str (mpz_ptr, __gmp_const char *, int) {return 0;}
   void __gmpz_set_si (mpz_ptr, signed long int) {}
   void __gmpz_set (mpz_ptr, mpz_srcptr) {}
   void __gmpz_powm_ui (mpz_ptr, mpz_srcptr, unsigned long int, mpz_srcptr) {}
   void __gmpz_powm (mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_pow_ui (mpz_ptr, mpz_srcptr, unsigned long int) {}
   size_t __gmpz_out_str (FILE *, int, mpz_srcptr) {return 0;}
   size_t __gmpz_out_raw (FILE *, mpz_srcptr) {return 0;}
   void __gmpz_mul_ui (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_mul_2exp (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_mul (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_ior (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   size_t __gmpz_inp_raw (mpz_ptr, FILE *) {return 0;}
   void __gmpz_init_set_ui (mpz_ptr, unsigned long int) {}
   void __gmpz_init_set_si (mpz_ptr, signed long int) {}
   void __gmpz_init_set (mpz_ptr, mpz_srcptr) {}
   void __gmpz_init (mpz_ptr) {}
   char *__gmpz_get_str (char *, int, mpz_srcptr) {return 0;}
   void __gmpz_gcd (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   unsigned long int __gmpz_fdiv_r_ui (mpz_ptr, mpz_srcptr, unsigned long int) {return 0;}
   void __gmpz_clear (mpz_ptr) {}
   void __gmpz_and (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   void __gmpz_addmul_ui (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_add_ui (mpz_ptr, mpz_srcptr, unsigned long int) {}
   void __gmpz_add (mpz_ptr, mpz_srcptr, mpz_srcptr) {}
   int __gmpn_perfect_square_p (mp_srcptr, mp_size_t) {return 0;}
   const int __gmp_bits_per_limb=32;
}

#else

#if defined (_MSC_VER)
#pragma comment (lib, "libgmp.a")
// libgcc.a is NOT needed any more, since we link in a non alloca static GMP now.
//#pragma comment (lib, "libgcc.a")
#endif

#endif

// Ok, NOW create the function to actually map DLL functions into our function pointers, or the included
// static functions to our function pointers

static HMODULE h_Gmp4DllInst;
static bool bGmp4DllInited=false;

/*static */void Connect_to_Static_GMP_Functions();

#define FAIL_AND_BAIL goto FailAndBailJump

void Initialize_Dynamic_GMP_System()
{
   if (bGmp4DllInited)
      return;

#if defined (__MINGW32__)
// printf ("WE DID IT!!!!\n");
#endif

   // we only want to get to this part of the code one time.
   bGmp4DllInited = true;

   // now load the .DLL.  2 names are possible, so search for either of them

   // Not loaded any more, since the linked in GMP is non-alloca
// h_Gmp4DllInst = LoadLibrary("libgmp-3-naa.dll");
// if (h_Gmp4DllInst)
//    g_bNoAlloca = true;
// else
   {
      h_Gmp4DllInst = LoadLibrary("libgmp-3.dll");
      if (!h_Gmp4DllInst)
      {
         // do NOT load the cygwin built DLL.  Screen output was NOT working correctly.
//       h_Gmp4DllInst = LoadLibrary("cyggmp-3.dll"); // Can't find the mingw32 version, so try for the Cygwin version
//       if (h_Gmp4DllInst)
//          g_bCygwinDLL = true;
      }

      // Can't find a DLL, so load the defaults and let the user use them.
      if (h_Gmp4DllInst == 0)
      {
         Connect_to_Static_GMP_Functions();
         return;
      }
   }

   // Load the functions from the DLL

// "extra" exported mpz's
//__gmpz_aorsmul_1         // not in manual, not in gmp.h, but exported
//__gmpz_divexact_gcd      // not in manual, not in gmp.h, but exported (mpz_divexact and divexact_ui are in gmp.h and exported)
//__gmpz_inp_str_nowhite   // not in manual, not in gmp.h, but exported
//__gmpz_legendre       // In manual, but not in gmp.h file.
//__gmpz_n_pow_ui       // not in manual, not in gmp.h, but exported

// Not sure what the hell these are They appear in gmp.h, but are NOT exported.
// _gmp_obstack_printf_fp = (_gmp_obstack_printf_t)GetProcAddress(h_Gmp4DllInst, "");
// if (!_gmp_obstack_printf_fp)
//    FAIL_AND_BAIL;

// _gmp_obstack_vprintf_fp = (_gmp_obstack_vprintf_t)GetProcAddress(h_Gmp4DllInst, "");
// if (!_gmp_obstack_vprintf_fp)
//    FAIL_AND_BAIL;

   _mp_set_memory_functions_fp = (_mp_set_memory_functions_t)GetProcAddress(h_Gmp4DllInst, "__gmp_set_memory_functions");
   if (!_mp_set_memory_functions_fp)
      FAIL_AND_BAIL;

   _gmp_randinit_fp = (_gmp_randinit_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randinit");
   if (!_gmp_randinit_fp)
      FAIL_AND_BAIL;

   _gmp_randinit_default_fp = (_gmp_randinit_default_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randinit_default");
   if (!_gmp_randinit_default_fp)
      FAIL_AND_BAIL;

   _gmp_randinit_lc_fp = (_gmp_randinit_lc_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randinit_lc");
   if (!_gmp_randinit_lc_fp)
      FAIL_AND_BAIL;

   _gmp_randinit_lc_2exp_fp = (_gmp_randinit_lc_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randinit_lc_2exp");
   if (!_gmp_randinit_lc_2exp_fp)
      FAIL_AND_BAIL;

   _gmp_randinit_lc_2exp_size_fp = (_gmp_randinit_lc_2exp_size_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randinit_lc_2exp_size");
   if (!_gmp_randinit_lc_2exp_size_fp)
      FAIL_AND_BAIL;

   _gmp_randseed_fp = (_gmp_randseed_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randseed");
   if (!_gmp_randseed_fp)
      FAIL_AND_BAIL;

   _gmp_randseed_ui_fp = (_gmp_randseed_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randseed_ui");
   if (!_gmp_randseed_ui_fp)
      FAIL_AND_BAIL;

   _gmp_randclear_fp = (_gmp_randclear_t)GetProcAddress(h_Gmp4DllInst, "__gmp_randclear");
   if (!_gmp_randclear_fp)
      FAIL_AND_BAIL;

   _gmp_asprintf_fp = (_gmp_asprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_asprintf");
   if (!_gmp_asprintf_fp)
      FAIL_AND_BAIL;

   _gmp_fprintf_fp = (_gmp_fprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_fprintf");
   if (!_gmp_fprintf_fp)
      FAIL_AND_BAIL;

   _gmp_printf_fp = (_gmp_printf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_printf");
   if (!_gmp_printf_fp)
      FAIL_AND_BAIL;

   _gmp_snprintf_fp = (_gmp_snprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_snprintf");
   if (!_gmp_snprintf_fp)
      FAIL_AND_BAIL;

   _gmp_sprintf_fp = (_gmp_sprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_sprintf");
   if (!_gmp_sprintf_fp)
      FAIL_AND_BAIL;

   _gmp_vasprintf_fp = (_gmp_vasprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vasprintf");
   if (!_gmp_vasprintf_fp)
      FAIL_AND_BAIL;

   _gmp_vfprintf_fp = (_gmp_vfprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vfprintf");
   if (!_gmp_vfprintf_fp)
      FAIL_AND_BAIL;

   _gmp_vprintf_fp = (_gmp_vprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vprintf");
   if (!_gmp_vprintf_fp)
      FAIL_AND_BAIL;

   _gmp_vsnprintf_fp = (_gmp_vsnprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vsnprintf");
   if (!_gmp_vsnprintf_fp)
      FAIL_AND_BAIL;

   _gmp_vsprintf_fp = (_gmp_vsprintf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vsprintf");
   if (!_gmp_vsprintf_fp)
      FAIL_AND_BAIL;

   _gmp_fscanf_fp = (_gmp_fscanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_fscanf");
   if (!_gmp_fscanf_fp)
      FAIL_AND_BAIL;

   _gmp_scanf_fp = (_gmp_scanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_scanf");
   if (!_gmp_scanf_fp)
      FAIL_AND_BAIL;

   _gmp_sscanf_fp = (_gmp_sscanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_sscanf");
   if (!_gmp_sscanf_fp)
      FAIL_AND_BAIL;

   _gmp_vfscanf_fp = (_gmp_vfscanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vfscanf");
   if (!_gmp_vfscanf_fp)
      FAIL_AND_BAIL;

   _gmp_vscanf_fp = (_gmp_vscanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vscanf");
   if (!_gmp_vscanf_fp)
      FAIL_AND_BAIL;

   _gmp_vsscanf_fp = (_gmp_vsscanf_t)GetProcAddress(h_Gmp4DllInst, "__gmp_vsscanf");
   if (!_gmp_vsscanf_fp)
      FAIL_AND_BAIL;

   __mpz_realloc_fp = (__mpz_realloc_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_realloc");
   if (!__mpz_realloc_fp)
      FAIL_AND_BAIL;

   _mpz_abs_fp = (_mpz_abs_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_abs");
   if (!_mpz_abs_fp)
      FAIL_AND_BAIL;

   _mpz_add_fp = (_mpz_add_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_add");
   if (!_mpz_add_fp)
      FAIL_AND_BAIL;

   _mpz_add_ui_fp = (_mpz_add_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_add_ui");
   if (!_mpz_add_ui_fp)
      FAIL_AND_BAIL;

   _mpz_addmul_fp = (_mpz_addmul_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_addmul");
   if (!_mpz_addmul_fp)
      FAIL_AND_BAIL;

   _mpz_addmul_ui_fp = (_mpz_addmul_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_addmul_ui");
   if (!_mpz_addmul_ui_fp)
      FAIL_AND_BAIL;

   _mpz_and_fp = (_mpz_and_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_and");
   if (!_mpz_and_fp)
      FAIL_AND_BAIL;

   _mpz_array_init_fp = (_mpz_array_init_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_array_init");
   if (!_mpz_array_init_fp)
      FAIL_AND_BAIL;

   _mpz_bin_ui_fp = (_mpz_bin_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_bin_ui");
   if (!_mpz_bin_ui_fp)
      FAIL_AND_BAIL;

   _mpz_bin_uiui_fp = (_mpz_bin_uiui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_bin_uiui");
   if (!_mpz_bin_uiui_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_q_fp = (_mpz_cdiv_q_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_q");
   if (!_mpz_cdiv_q_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_q_2exp_fp = (_mpz_cdiv_q_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_q_2exp");
   if (!_mpz_cdiv_q_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_q_ui_fp = (_mpz_cdiv_q_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_q_ui");
   if (!_mpz_cdiv_q_ui_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_qr_fp = (_mpz_cdiv_qr_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_qr");
   if (!_mpz_cdiv_qr_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_qr_ui_fp = (_mpz_cdiv_qr_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_qr_ui");
   if (!_mpz_cdiv_qr_ui_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_r_fp = (_mpz_cdiv_r_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_r");
   if (!_mpz_cdiv_r_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_r_2exp_fp = (_mpz_cdiv_r_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_r_2exp");
   if (!_mpz_cdiv_r_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_r_ui_fp = (_mpz_cdiv_r_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_r_ui");
   if (!_mpz_cdiv_r_ui_fp)
      FAIL_AND_BAIL;

   _mpz_clear_fp = (_mpz_clear_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_clear");
   if (!_mpz_clear_fp)
      FAIL_AND_BAIL;

   _mpz_clrbit_fp = (_mpz_clrbit_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_clrbit");
   if (!_mpz_clrbit_fp)
      FAIL_AND_BAIL;

   _mpz_com_fp = (_mpz_com_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_com");
   if (!_mpz_com_fp)
      FAIL_AND_BAIL;

   _mpz_divexact_fp = (_mpz_divexact_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_divexact");
   if (!_mpz_divexact_fp)
      FAIL_AND_BAIL;

   _mpz_divexact_ui_fp = (_mpz_divexact_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_divexact_ui");
   if (!_mpz_divexact_ui_fp)
      FAIL_AND_BAIL;

   _mpz_dump_fp = (_mpz_dump_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_dump");
   if (!_mpz_dump_fp)
      FAIL_AND_BAIL;

   _mpz_fac_ui_fp = (_mpz_fac_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fac_ui");
   if (!_mpz_fac_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_q_fp = (_mpz_fdiv_q_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_q");
   if (!_mpz_fdiv_q_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_q_2exp_fp = (_mpz_fdiv_q_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_q_2exp");
   if (!_mpz_fdiv_q_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_q_ui_fp = (_mpz_fdiv_q_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_q_ui");
   if (!_mpz_fdiv_q_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_qr_fp = (_mpz_fdiv_qr_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_qr");
   if (!_mpz_fdiv_qr_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_qr_ui_fp = (_mpz_fdiv_qr_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_qr_ui");
   if (!_mpz_fdiv_qr_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_r_fp = (_mpz_fdiv_r_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_r");
   if (!_mpz_fdiv_r_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_r_2exp_fp = (_mpz_fdiv_r_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_r_2exp");
   if (!_mpz_fdiv_r_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_r_ui_fp = (_mpz_fdiv_r_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_r_ui");
   if (!_mpz_fdiv_r_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fib_ui_fp = (_mpz_fib_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fib_ui");
   if (!_mpz_fib_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fib2_ui_fp = (_mpz_fib2_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fib2_ui");
   if (!_mpz_fib2_ui_fp)
      FAIL_AND_BAIL;

   _mpz_gcd_fp = (_mpz_gcd_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_gcd");
   if (!_mpz_gcd_fp)
      FAIL_AND_BAIL;

   _mpz_gcd_ui_fp = (_mpz_gcd_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_gcd_ui");
   if (!_mpz_gcd_ui_fp)
      FAIL_AND_BAIL;

   _mpz_gcdext_fp = (_mpz_gcdext_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_gcdext");
   if (!_mpz_gcdext_fp)
      FAIL_AND_BAIL;

   _mpz_get_d_2exp_fp = (_mpz_get_d_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_get_d_2exp");
   if (!_mpz_get_d_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_get_str_fp = (_mpz_get_str_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_get_str");
   if (!_mpz_get_str_fp)
      FAIL_AND_BAIL;

   _mpz_init_fp = (_mpz_init_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init");
   if (!_mpz_init_fp)
      FAIL_AND_BAIL;

   _mpz_init2_fp = (_mpz_init2_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init2");
   if (!_mpz_init2_fp)
      FAIL_AND_BAIL;

   _mpz_init_set_fp = (_mpz_init_set_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init_set");
   if (!_mpz_init_set_fp)
      FAIL_AND_BAIL;

   _mpz_init_set_d_fp = (_mpz_init_set_d_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init_set_d");
   if (!_mpz_init_set_d_fp)
      FAIL_AND_BAIL;

   _mpz_init_set_si_fp = (_mpz_init_set_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init_set_si");
   if (!_mpz_init_set_si_fp)
      FAIL_AND_BAIL;

   _mpz_init_set_str_fp = (_mpz_init_set_str_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init_set_str");
   if (!_mpz_init_set_str_fp)
      FAIL_AND_BAIL;

   _mpz_init_set_ui_fp = (_mpz_init_set_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_init_set_ui");
   if (!_mpz_init_set_ui_fp)
      FAIL_AND_BAIL;

   _mpz_inp_raw_fp = (_mpz_inp_raw_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_inp_raw");
   if (!_mpz_inp_raw_fp)
      FAIL_AND_BAIL;

   _mpz_inp_str_fp = (_mpz_inp_str_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_inp_str");
   if (!_mpz_inp_str_fp)
      FAIL_AND_BAIL;

   _mpz_invert_fp = (_mpz_invert_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_invert");
   if (!_mpz_invert_fp)
      FAIL_AND_BAIL;

   _mpz_ior_fp = (_mpz_ior_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_ior");
   if (!_mpz_ior_fp)
      FAIL_AND_BAIL;

   _mpz_lcm_fp = (_mpz_lcm_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_lcm");
   if (!_mpz_lcm_fp)
      FAIL_AND_BAIL;

   _mpz_lcm_ui_fp = (_mpz_lcm_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_lcm_ui");
   if (!_mpz_lcm_ui_fp)
      FAIL_AND_BAIL;

   _mpz_lucnum_ui_fp = (_mpz_lucnum_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_lucnum_ui");
   if (!_mpz_lucnum_ui_fp)
      FAIL_AND_BAIL;

   _mpz_lucnum2_ui_fp = (_mpz_lucnum2_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_lucnum2_ui");
   if (!_mpz_lucnum2_ui_fp)
      FAIL_AND_BAIL;

   _mpz_mod_fp = (_mpz_mod_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_mod");
   if (!_mpz_mod_fp)
      FAIL_AND_BAIL;

   _mpz_mul_fp = (_mpz_mul_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_mul");
   if (!_mpz_mul_fp)
      FAIL_AND_BAIL;

   _mpz_mul_2exp_fp = (_mpz_mul_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_mul_2exp");
   if (!_mpz_mul_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_mul_si_fp = (_mpz_mul_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_mul_si");
   if (!_mpz_mul_si_fp)
      FAIL_AND_BAIL;

   _mpz_mul_ui_fp = (_mpz_mul_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_mul_ui");
   if (!_mpz_mul_ui_fp)
      FAIL_AND_BAIL;

   _mpz_neg_fp = (_mpz_neg_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_neg");
   if (!_mpz_neg_fp)
      FAIL_AND_BAIL;

   _mpz_nextprime_fp = (_mpz_nextprime_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_nextprime");
   if (!_mpz_nextprime_fp)
      FAIL_AND_BAIL;

   _mpz_out_raw_fp = (_mpz_out_raw_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_out_raw");
   if (!_mpz_out_raw_fp)
      FAIL_AND_BAIL;

   _mpz_out_str_fp = (_mpz_out_str_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_out_str");
   if (!_mpz_out_str_fp)
      FAIL_AND_BAIL;

   _mpz_pow_ui_fp = (_mpz_pow_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_pow_ui");
   if (!_mpz_pow_ui_fp)
      FAIL_AND_BAIL;

   _mpz_powm_fp = (_mpz_powm_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_powm");
   if (!_mpz_powm_fp)
      FAIL_AND_BAIL;

   _mpz_powm_ui_fp = (_mpz_powm_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_powm_ui");
   if (!_mpz_powm_ui_fp)
      FAIL_AND_BAIL;

   _mpz_random_fp = (_mpz_random_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_random");
   if (!_mpz_random_fp)
      FAIL_AND_BAIL;

   _mpz_random2_fp = (_mpz_random2_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_random2");
   if (!_mpz_random2_fp)
      FAIL_AND_BAIL;

   _mpz_realloc2_fp = (_mpz_realloc2_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_realloc2");
   if (!_mpz_realloc2_fp)
      FAIL_AND_BAIL;

   _mpz_remove_fp = (_mpz_remove_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_remove");
   if (!_mpz_remove_fp)
      FAIL_AND_BAIL;

   _mpz_root_fp = (_mpz_root_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_root");
   if (!_mpz_root_fp)
      FAIL_AND_BAIL;

   _mpz_rrandomb_fp = (_mpz_rrandomb_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_rrandomb");
   if (!_mpz_rrandomb_fp)
      FAIL_AND_BAIL;

   _mpz_set_fp = (_mpz_set_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set");
   if (!_mpz_set_fp)
      FAIL_AND_BAIL;

   _mpz_set_d_fp = (_mpz_set_d_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_d");
   if (!_mpz_set_d_fp)
      FAIL_AND_BAIL;

   _mpz_set_f_fp = (_mpz_set_f_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_f");
   if (!_mpz_set_f_fp)
      FAIL_AND_BAIL;

   _mpz_set_q_fp = (_mpz_set_q_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_q");
   if (!_mpz_set_q_fp)
      FAIL_AND_BAIL;

   _mpz_set_si_fp = (_mpz_set_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_si");
   if (!_mpz_set_si_fp)
      FAIL_AND_BAIL;

   _mpz_set_str_fp = (_mpz_set_str_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_str");
   if (!_mpz_set_str_fp)
      FAIL_AND_BAIL;

   _mpz_set_ui_fp = (_mpz_set_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_set_ui");
   if (!_mpz_set_ui_fp)
      FAIL_AND_BAIL;

   _mpz_setbit_fp = (_mpz_setbit_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_setbit");
   if (!_mpz_setbit_fp)
      FAIL_AND_BAIL;

   _mpz_sqrt_fp = (_mpz_sqrt_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_sqrt");
   if (!_mpz_sqrt_fp)
      FAIL_AND_BAIL;

   _mpz_sqrtrem_fp = (_mpz_sqrtrem_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_sqrtrem");
   if (!_mpz_sqrtrem_fp)
      FAIL_AND_BAIL;

   _mpz_sub_fp = (_mpz_sub_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_sub");
   if (!_mpz_sub_fp)
      FAIL_AND_BAIL;

   _mpz_sub_ui_fp = (_mpz_sub_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_sub_ui");
   if (!_mpz_sub_ui_fp)
      FAIL_AND_BAIL;

   _mpz_submul_fp = (_mpz_submul_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_submul");
   if (!_mpz_submul_fp)
      FAIL_AND_BAIL;

   _mpz_submul_ui_fp = (_mpz_submul_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_submul_ui");
   if (!_mpz_submul_ui_fp)
      FAIL_AND_BAIL;

   _mpz_swap_fp = (_mpz_swap_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_swap");
   if (!_mpz_swap_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_q_fp = (_mpz_tdiv_q_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_q");
   if (!_mpz_tdiv_q_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_q_2exp_fp = (_mpz_tdiv_q_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_q_2exp");
   if (!_mpz_tdiv_q_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_q_ui_fp = (_mpz_tdiv_q_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_q_ui");
   if (!_mpz_tdiv_q_ui_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_qr_fp = (_mpz_tdiv_qr_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_qr");
   if (!_mpz_tdiv_qr_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_qr_ui_fp = (_mpz_tdiv_qr_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_qr_ui");
   if (!_mpz_tdiv_qr_ui_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_r_fp = (_mpz_tdiv_r_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_r");
   if (!_mpz_tdiv_r_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_r_2exp_fp = (_mpz_tdiv_r_2exp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_r_2exp");
   if (!_mpz_tdiv_r_2exp_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_r_ui_fp = (_mpz_tdiv_r_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_r_ui");
   if (!_mpz_tdiv_r_ui_fp)
      FAIL_AND_BAIL;

   _mpz_ui_pow_ui_fp = (_mpz_ui_pow_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_ui_pow_ui");
   if (!_mpz_ui_pow_ui_fp)
      FAIL_AND_BAIL;

   _mpz_urandomb_fp = (_mpz_urandomb_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_urandomb");
   if (!_mpz_urandomb_fp)
      FAIL_AND_BAIL;

   _mpz_urandomm_fp = (_mpz_urandomm_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_urandomm");
   if (!_mpz_urandomm_fp)
      FAIL_AND_BAIL;

   _mpz_xor_fp = (_mpz_xor_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_xor");
   if (!_mpz_xor_fp)
      FAIL_AND_BAIL;

   _mpz_cdiv_ui_fp = (_mpz_cdiv_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cdiv_ui");
   if (!_mpz_cdiv_ui_fp)
      FAIL_AND_BAIL;

   _mpz_cmp_fp = (_mpz_cmp_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmp");
   if (!_mpz_cmp_fp)
      FAIL_AND_BAIL;

   _mpz_cmp_d_fp = (_mpz_cmp_d_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmp_d");
   if (!_mpz_cmp_d_fp)
      FAIL_AND_BAIL;

   __mpz_cmp_si_fp = (__mpz_cmp_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmp_si");
   if (!__mpz_cmp_si_fp)
      FAIL_AND_BAIL;

   __mpz_cmp_ui_fp = (__mpz_cmp_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmp_ui");
   if (!__mpz_cmp_ui_fp)
      FAIL_AND_BAIL;

   _mpz_cmpabs_fp = (_mpz_cmpabs_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmpabs");
   if (!_mpz_cmpabs_fp)
      FAIL_AND_BAIL;

   _mpz_cmpabs_d_fp = (_mpz_cmpabs_d_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmpabs_d");
   if (!_mpz_cmpabs_d_fp)
      FAIL_AND_BAIL;

   _mpz_cmpabs_ui_fp = (_mpz_cmpabs_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_cmpabs_ui");
   if (!_mpz_cmpabs_ui_fp)
      FAIL_AND_BAIL;

   _mpz_congruent_p_fp = (_mpz_congruent_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_congruent_p");
   if (!_mpz_congruent_p_fp)
      FAIL_AND_BAIL;

   _mpz_congruent_2exp_p_fp = (_mpz_congruent_2exp_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_congruent_2exp_p");
   if (!_mpz_congruent_2exp_p_fp)
      FAIL_AND_BAIL;

   _mpz_congruent_ui_p_fp = (_mpz_congruent_ui_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_congruent_ui_p");
   if (!_mpz_congruent_ui_p_fp)
      FAIL_AND_BAIL;

   _mpz_divisible_p_fp = (_mpz_divisible_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_divisible_p");
   if (!_mpz_divisible_p_fp)
      FAIL_AND_BAIL;

   _mpz_divisible_ui_p_fp = (_mpz_divisible_ui_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_divisible_ui_p");
   if (!_mpz_divisible_ui_p_fp)
      FAIL_AND_BAIL;

   _mpz_divisible_2exp_p_fp = (_mpz_divisible_2exp_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_divisible_2exp_p");
   if (!_mpz_divisible_2exp_p_fp)
      FAIL_AND_BAIL;

   _mpz_fdiv_ui_fp = (_mpz_fdiv_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fdiv_ui");
   if (!_mpz_fdiv_ui_fp)
      FAIL_AND_BAIL;

   _mpz_fits_sint_p_fp = (_mpz_fits_sint_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_sint_p");
   if (!_mpz_fits_sint_p_fp)
      FAIL_AND_BAIL;

   _mpz_fits_slong_p_fp = (_mpz_fits_slong_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_slong_p");
   if (!_mpz_fits_slong_p_fp)
      FAIL_AND_BAIL;

   _mpz_fits_sshort_p_fp = (_mpz_fits_sshort_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_sshort_p");
   if (!_mpz_fits_sshort_p_fp)
      FAIL_AND_BAIL;

   _mpz_fits_uint_p_fp = (_mpz_fits_uint_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_uint_p");
   if (!_mpz_fits_uint_p_fp)
      FAIL_AND_BAIL;

   _mpz_fits_ulong_p_fp = (_mpz_fits_ulong_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_ulong_p");
   if (!_mpz_fits_ulong_p_fp)
      FAIL_AND_BAIL;

   _mpz_fits_ushort_p_fp = (_mpz_fits_ushort_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_fits_ushort_p");
   if (!_mpz_fits_ushort_p_fp)
      FAIL_AND_BAIL;

   _mpz_get_d_fp = (_mpz_get_d_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_get_d");
   if (!_mpz_get_d_fp)
      FAIL_AND_BAIL;

   _mpz_get_si_fp = (_mpz_get_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_get_si");
   if (!_mpz_get_si_fp)
      FAIL_AND_BAIL;

   _mpz_get_ui_fp = (_mpz_get_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_get_ui");
   if (!_mpz_get_ui_fp)
      FAIL_AND_BAIL;

   _mpz_getlimbn_fp = (_mpz_getlimbn_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_getlimbn");
   if (!_mpz_getlimbn_fp)
      FAIL_AND_BAIL;

   _mpz_hamdist_fp = (_mpz_hamdist_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_hamdist");
   if (!_mpz_hamdist_fp)
      FAIL_AND_BAIL;

   _mpz_jacobi_fp = (_mpz_jacobi_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_jacobi");
   if (!_mpz_jacobi_fp)
      FAIL_AND_BAIL;

   _mpz_kronecker_si_fp = (_mpz_kronecker_si_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_kronecker_si");
   if (!_mpz_kronecker_si_fp)
      FAIL_AND_BAIL;

   _mpz_kronecker_ui_fp = (_mpz_kronecker_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_kronecker_ui");
   if (!_mpz_kronecker_ui_fp)
      FAIL_AND_BAIL;

   _mpz_si_kronecker_fp = (_mpz_si_kronecker_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_si_kronecker");
   if (!_mpz_si_kronecker_fp)
      FAIL_AND_BAIL;

   _mpz_ui_kronecker_fp = (_mpz_ui_kronecker_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_ui_kronecker");
   if (!_mpz_ui_kronecker_fp)
      FAIL_AND_BAIL;

   _mpz_millerrabin_fp = (_mpz_millerrabin_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_millerrabin");
   if (!_mpz_millerrabin_fp)
      FAIL_AND_BAIL;

   _mpz_perfect_power_p_fp = (_mpz_perfect_power_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_perfect_power_p");
   if (!_mpz_perfect_power_p_fp)
      FAIL_AND_BAIL;

   _mpz_perfect_square_p_fp = (_mpz_perfect_square_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_perfect_square_p");
   if (!_mpz_perfect_square_p_fp)
      FAIL_AND_BAIL;

   _mpz_popcount_fp = (_mpz_popcount_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_popcount");
   if (!_mpz_popcount_fp)
      FAIL_AND_BAIL;

   _mpz_probab_prime_p_fp = (_mpz_probab_prime_p_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_probab_prime_p");
   if (!_mpz_probab_prime_p_fp)
      FAIL_AND_BAIL;

   _mpz_scan0_fp = (_mpz_scan0_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_scan0");
   if (!_mpz_scan0_fp)
      FAIL_AND_BAIL;

   _mpz_scan1_fp = (_mpz_scan1_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_scan1");
   if (!_mpz_scan1_fp)
      FAIL_AND_BAIL;

   _mpz_size_fp = (_mpz_size_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_size");
   if (!_mpz_size_fp)
      FAIL_AND_BAIL;

   _mpz_sizeinbase_fp = (_mpz_sizeinbase_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_sizeinbase");
   if (!_mpz_sizeinbase_fp)
      FAIL_AND_BAIL;

   _mpz_tdiv_ui_fp = (_mpz_tdiv_ui_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tdiv_ui");
   if (!_mpz_tdiv_ui_fp)
      FAIL_AND_BAIL;

   _mpz_tstbit_fp = (_mpz_tstbit_t)GetProcAddress(h_Gmp4DllInst, "__gmpz_tstbit");
   if (!_mpz_tstbit_fp)
      FAIL_AND_BAIL;

   // YEAH!! We got here, so we got them all.
   g_bGMPUsingDLL = true;
   return;

FailAndBailJump:

   if(h_Gmp4DllInst)
      FreeLibrary(h_Gmp4DllInst);
   h_Gmp4DllInst=0;
   Connect_to_Static_GMP_Functions();
}

void Free_Dynamic_GMP_System()
{
   if (h_Gmp4DllInst)
      FreeLibrary(h_Gmp4DllInst);
   h_Gmp4DllInst = 0;
   bGmp4DllInited = false;          // Note, that once the system is "freed", we want to make it initializable again
   Connect_to_Static_GMP_Functions();  // Make sure the GMP functions point to something valid (just in case)
}

/*static*/ void Connect_to_Static_GMP_Functions()
{
// _mp_set_memory_functions_fp      = __gmp_set_memory_functions;
// _gmp_randinit_fp           = __gmp_randinit;
// _gmp_randinit_default_fp      = __gmp_randinit_default;
// _gmp_randinit_lc_fp           = __gmp_randinit_lc;
// _gmp_randinit_lc_2exp_fp      = __gmp_randinit_lc_2exp;
// _gmp_randinit_lc_2exp_size_fp = __gmp_randinit_lc_2exp_size;
// _gmp_randseed_fp           = __gmp_randseed;
// _gmp_randseed_ui_fp           = __gmp_randseed_ui;
// _gmp_randclear_fp          = __gmp_randclear;
// _gmp_asprintf_fp           = __gmp_asprintf;
// _gmp_fprintf_fp               = __gmp_fprintf;

   // Not sure what to make of these 2
// _gmp_obstack_printf_fp        = gmp_obstack_printf;
// _gmp_obstack_vprintf_fp       = gmp_obstack_vprintf;

// _gmp_printf_fp             = __gmp_printf;
// _gmp_snprintf_fp           = __gmp_snprintf;
// _gmp_sprintf_fp               = __gmp_sprintf;
// _gmp_vasprintf_fp          = __gmp_vasprintf;
// _gmp_vfprintf_fp           = __gmp_vfprintf;
// _gmp_vprintf_fp               = __gmp_vprintf;
// _gmp_vsnprintf_fp          = __gmp_vsnprintf;
// _gmp_vsprintf_fp           = __gmp_vsprintf;
// _gmp_fscanf_fp             = __gmp_fscanf;
// _gmp_scanf_fp              = __gmp_scanf;
// _gmp_sscanf_fp             = __gmp_sscanf;
// _gmp_vfscanf_fp               = __gmp_vfscanf;
// _gmp_vscanf_fp             = __gmp_vscanf;
// _gmp_vsscanf_fp               = __gmp_vsscanf;
// __mpz_realloc_fp           = __gmpz_realloc;
// _mpz_abs_fp                = __gmpz_abs;
   _mpz_add_fp                = __gmpz_add;
   _mpz_add_ui_fp             = __gmpz_add_ui;
// _mpz_addmul_fp             = __gmpz_addmul;
   _mpz_addmul_ui_fp          = __gmpz_addmul_ui;
   _mpz_and_fp                = __gmpz_and;
// _mpz_array_init_fp            = __gmpz_array_init;
// _mpz_bin_ui_fp             = __gmpz_bin_ui;
// _mpz_bin_uiui_fp           = __gmpz_bin_uiui;
// _mpz_cdiv_q_fp             = __gmpz_cdiv_q;
// _mpz_cdiv_q_2exp_fp           = __gmpz_cdiv_q_2exp;
// _mpz_cdiv_q_ui_fp          = __gmpz_cdiv_q_ui;
// _mpz_cdiv_qr_fp               = __gmpz_cdiv_qr;
// _mpz_cdiv_qr_ui_fp            = __gmpz_cdiv_qr_ui;
// _mpz_cdiv_r_fp             = __gmpz_cdiv_r;
// _mpz_cdiv_r_2exp_fp           = __gmpz_cdiv_r_2exp;
// _mpz_cdiv_r_ui_fp          = __gmpz_cdiv_r_ui;
   _mpz_clear_fp              = __gmpz_clear;
// _mpz_clrbit_fp             = __gmpz_clrbit;
// _mpz_com_fp                = __gmpz_com;
// _mpz_divexact_fp           = __gmpz_divexact;
// _mpz_divexact_ui_fp           = __gmpz_divexact_ui;
// _mpz_dump_fp               = __gmpz_dump;
// _mpz_fac_ui_fp             = __gmpz_fac_ui;
// _mpz_fdiv_q_fp             = __gmpz_fdiv_q;
// _mpz_fdiv_q_2exp_fp           = __gmpz_fdiv_q_2exp;
// _mpz_fdiv_q_ui_fp          = __gmpz_fdiv_q_ui;
// _mpz_fdiv_qr_fp               = __gmpz_fdiv_qr;
// _mpz_fdiv_qr_ui_fp            = __gmpz_fdiv_qr_ui;
// _mpz_fdiv_r_fp             = __gmpz_fdiv_r;
// _mpz_fdiv_r_2exp_fp           = __gmpz_fdiv_r_2exp;
   _mpz_fdiv_r_ui_fp          = __gmpz_fdiv_r_ui;
// _mpz_fib_ui_fp             = __gmpz_fib_ui;
// _mpz_fib2_ui_fp               = __gmpz_fib2_ui;
   _mpz_gcd_fp                = __gmpz_gcd;
// _mpz_gcd_ui_fp             = __gmpz_gcd_ui;
// _mpz_gcdext_fp             = __gmpz_gcdext;
// _mpz_get_d_2exp_fp            = __gmpz_get_d_2exp;
   _mpz_get_str_fp               = __gmpz_get_str;
   _mpz_init_fp               = __gmpz_init;
// _mpz_init2_fp              = __gmpz_init2;
   _mpz_init_set_fp           = __gmpz_init_set;
// _mpz_init_set_d_fp            = __gmpz_init_set_d;
   _mpz_init_set_si_fp           = __gmpz_init_set_si;
// _mpz_init_set_str_fp       = __gmpz_init_set_str;
   _mpz_init_set_ui_fp           = __gmpz_init_set_ui;
   _mpz_inp_raw_fp               = __gmpz_inp_raw;
// _mpz_inp_str_fp               = __gmpz_inp_str;
// _mpz_invert_fp             = __gmpz_invert;
   _mpz_ior_fp                = __gmpz_ior;
// _mpz_lcm_fp                = __gmpz_lcm;
// _mpz_lcm_ui_fp             = __gmpz_lcm_ui;
// _mpz_lucnum_ui_fp          = __gmpz_lucnum_ui;
// _mpz_lucnum2_ui_fp            = __gmpz_lucnum2_ui;
// _mpz_mod_fp                = __gmpz_mod;
   _mpz_mul_fp                = __gmpz_mul;
   _mpz_mul_2exp_fp           = __gmpz_mul_2exp;
// _mpz_mul_si_fp             = __gmpz_mul_si;
   _mpz_mul_ui_fp             = __gmpz_mul_ui;
   _mpz_neg_fp                = __gmpz_neg;
// _mpz_nextprime_fp          = __gmpz_nextprime;
   _mpz_out_raw_fp               = __gmpz_out_raw;
   _mpz_out_str_fp               = __gmpz_out_str;
   _mpz_pow_ui_fp             = __gmpz_pow_ui;
   _mpz_powm_fp               = __gmpz_powm;
   _mpz_powm_ui_fp               = __gmpz_powm_ui;
// _mpz_random_fp             = __gmpz_random;
// _mpz_random2_fp               = __gmpz_random2;
// _mpz_realloc2_fp           = __gmpz_realloc2;
// _mpz_remove_fp             = __gmpz_remove;
// _mpz_root_fp               = __gmpz_root;
// _mpz_rrandomb_fp           = __gmpz_rrandomb;
   _mpz_set_fp                = __gmpz_set;
// _mpz_set_d_fp              = __gmpz_set_d;
// _mpz_set_f_fp              = __gmpz_set_f;
// _mpz_set_q_fp              = __gmpz_set_q;
   _mpz_set_si_fp             = __gmpz_set_si;
   _mpz_set_str_fp               = __gmpz_set_str;
   _mpz_set_ui_fp             = __gmpz_set_ui;
// _mpz_setbit_fp             = __gmpz_setbit;
   _mpz_sqrt_fp               = __gmpz_sqrt;
// _mpz_sqrtrem_fp               = __gmpz_sqrtrem;
   _mpz_sub_fp                = __gmpz_sub;
   _mpz_sub_ui_fp             = __gmpz_sub_ui;
// _mpz_submul_fp             = __gmpz_submul;
// _mpz_submul_ui_fp          = __gmpz_submul_ui;
// _mpz_swap_fp               = __gmpz_swap;
   _mpz_tdiv_q_fp             = __gmpz_tdiv_q;
   _mpz_tdiv_q_2exp_fp           = __gmpz_tdiv_q_2exp;
// _mpz_tdiv_q_ui_fp          = __gmpz_tdiv_q_ui;
   _mpz_tdiv_qr_fp               = __gmpz_tdiv_qr;
   _mpz_tdiv_qr_ui_fp            = __gmpz_tdiv_qr_ui;
// _mpz_tdiv_r_fp             = __gmpz_tdiv_r;
// _mpz_tdiv_r_2exp_fp           = __gmpz_tdiv_r_2exp;
// _mpz_tdiv_r_ui_fp          = __gmpz_tdiv_r_ui;
// _mpz_ui_pow_ui_fp          = __gmpz_ui_pow_ui;
// _mpz_urandomb_fp           = __gmpz_urandomb;
// _mpz_urandomm_fp           = __gmpz_urandomm;
// _mpz_xor_fp                = __gmpz_xor;
   _mpz_cmp_fp                = __gmpz_cmp;
// _mpz_cmp_d_fp              = __gmpz_cmp_d;
   __mpz_cmp_si_fp               = __gmpz_cmp_si;
// __mpz_cmp_ui_fp               = __gmpz_cmp_ui;
// _mpz_cmpabs_fp             = __gmpz_cmpabs;
// _mpz_cmpabs_d_fp           = __gmpz_cmpabs_d;
// _mpz_cmpabs_ui_fp          = __gmpz_cmpabs_ui;
// _mpz_congruent_p_fp           = __gmpz_congruent_p;
// _mpz_congruent_2exp_p_fp      = __gmpz_congruent_2exp_p;
// _mpz_congruent_ui_p_fp        = __gmpz_congruent_ui_p;
// _mpz_divisible_p_fp           = __gmpz_divisible_p;
// _mpz_divisible_ui_p_fp        = __gmpz_divisible_ui_p;
// _mpz_divisible_2exp_p_fp      = __gmpz_divisible_2exp_p;
// _mpz_fdiv_ui_fp               = __gmpz_fdiv_ui;
// _mpz_fits_sint_p_fp           = __gmpz_fits_sint_p;
// _mpz_fits_slong_p_fp       = __gmpz_fits_slong_p;
// _mpz_fits_sshort_p_fp         = __gmpz_fits_sshort_p;
// _mpz_fits_uint_p_fp           = __gmpz_fits_uint_p;
// _mpz_fits_ulong_p_fp       = __gmpz_fits_ulong_p;
// _mpz_fits_ushort_p_fp         = __gmpz_fits_ushort_p;
// _mpz_get_d_fp              = __gmpz_get_d;
   _mpz_get_si_fp             = __gmpz_get_si;
   _mpz_get_ui_fp             = __gmpz_get_ui;
   _mpz_getlimbn_fp           = __gmpz_getlimbn;
// _mpz_hamdist_fp               = __gmpz_hamdist;
   _mpz_jacobi_fp             = __gmpz_jacobi;
// _mpz_kronecker_si_fp       = __gmpz_kronecker_si;
// _mpz_kronecker_ui_fp       = __gmpz_kronecker_ui;
// _mpz_si_kronecker_fp       = __gmpz_si_kronecker;
// _mpz_ui_kronecker_fp       = __gmpz_ui_kronecker;
// _mpz_millerrabin_fp           = __gmpz_millerrabin;
// _mpz_perfect_power_p_fp       = __gmpz_perfect_power_p;
   _mpz_perfect_square_p_fp      = __gmpz_perfect_square_p;
// _mpz_popcount_fp           = __gmpz_popcount;
// _mpz_probab_prime_p_fp        = __gmpz_probab_prime_p;
// _mpz_scan0_fp              = __gmpz_scan0;
// _mpz_scan1_fp              = __gmpz_scan1;
   _mpz_size_fp               = __gmpz_size;
   _mpz_sizeinbase_fp            = __gmpz_sizeinbase;
// _mpz_tdiv_ui_fp               = __gmpz_tdiv_ui;
// _mpz_tstbit_fp             = __gmpz_tstbit;

// These are the ONLY functions used by pfgw
//__mpz_add_fp
//__mpz_add_ui_fp
//__mpz_clear_fp
//__mpz_cmp_fp
//__mpz_gcd_fp
//__mpz_get_si_fp
//__mpz_get_str_fp
//__mpz_get_ui_fp
//__mpz_init_fp
//__mpz_init_set_fp
//__mpz_init_set_si_fp
//__mpz_init_set_ui_fp
//__mpz_jacobi_fp
//__mpz_mul_2exp_fp
//__mpz_mul_fp
//__mpz_mul_ui_fp
//__mpz_neg_fp
//__mpz_perfect_square_p_fp
//__mpz_powm_fp
//__mpz_powm_ui_fp
//__mpz_pow_ui_fp
//__mpz_set_fp
//__mpz_set_si_fp
//__mpz_set_str_fp
//__mpz_set_ui_fp
//__mpz_sizeinbase_fp
//__mpz_sqrt_fp
//__mpz_sub_fp
//__mpz_sub_ui_fp
//__mpz_tdiv_qr_fp
//__mpz_tdiv_qr_ui_fp
//__mpz_tdiv_q_2exp_fp
//__mpz_tdiv_q_fp
//___gmp_bits_per_limb
//___mpz_cmp_si_fp

   g_bGMPUsingDLL = false;

}

// Now we declare the function pointers
_mp_set_memory_functions_t    _mp_set_memory_functions_fp;
_gmp_randinit_t               _gmp_randinit_fp;
_gmp_randinit_default_t       _gmp_randinit_default_fp;
_gmp_randinit_lc_t            _gmp_randinit_lc_fp;
_gmp_randinit_lc_2exp_t       _gmp_randinit_lc_2exp_fp;
_gmp_randinit_lc_2exp_size_t  _gmp_randinit_lc_2exp_size_fp;
_gmp_randseed_t               _gmp_randseed_fp;
_gmp_randseed_ui_t            _gmp_randseed_ui_fp;
_gmp_randclear_t           _gmp_randclear_fp;
_gmp_asprintf_t               _gmp_asprintf_fp;
_gmp_fprintf_t             _gmp_fprintf_fp;
_gmp_obstack_printf_t         _gmp_obstack_printf_fp;
_gmp_obstack_vprintf_t        _gmp_obstack_vprintf_fp;
_gmp_printf_t              _gmp_printf_fp;
_gmp_snprintf_t               _gmp_snprintf_fp;
_gmp_sprintf_t             _gmp_sprintf_fp;
_gmp_vasprintf_t           _gmp_vasprintf_fp;
_gmp_vfprintf_t               _gmp_vfprintf_fp;
_gmp_vprintf_t             _gmp_vprintf_fp;
_gmp_vsnprintf_t           _gmp_vsnprintf_fp;
_gmp_vsprintf_t               _gmp_vsprintf_fp;
_gmp_fscanf_t              _gmp_fscanf_fp;
_gmp_scanf_t               _gmp_scanf_fp;
_gmp_sscanf_t              _gmp_sscanf_fp;
_gmp_vfscanf_t             _gmp_vfscanf_fp;
_gmp_vscanf_t              _gmp_vscanf_fp;
_gmp_vsscanf_t             _gmp_vsscanf_fp;
__mpz_realloc_t               __mpz_realloc_fp;
_mpz_abs_t                 _mpz_abs_fp;
_mpz_add_t                 _mpz_add_fp;
_mpz_add_ui_t              _mpz_add_ui_fp;
_mpz_addmul_t              _mpz_addmul_fp;
_mpz_addmul_ui_t           _mpz_addmul_ui_fp;
_mpz_and_t                 _mpz_and_fp;
_mpz_array_init_t          _mpz_array_init_fp;
_mpz_bin_ui_t              _mpz_bin_ui_fp;
_mpz_bin_uiui_t               _mpz_bin_uiui_fp;
_mpz_cdiv_q_t              _mpz_cdiv_q_fp;
_mpz_cdiv_q_2exp_t            _mpz_cdiv_q_2exp_fp;
_mpz_cdiv_q_ui_t           _mpz_cdiv_q_ui_fp;
_mpz_cdiv_qr_t             _mpz_cdiv_qr_fp;
_mpz_cdiv_qr_ui_t          _mpz_cdiv_qr_ui_fp;
_mpz_cdiv_r_t              _mpz_cdiv_r_fp;
_mpz_cdiv_r_2exp_t            _mpz_cdiv_r_2exp_fp;
_mpz_cdiv_r_ui_t           _mpz_cdiv_r_ui_fp;
_mpz_clear_t               _mpz_clear_fp;
_mpz_clrbit_t              _mpz_clrbit_fp;
_mpz_com_t                 _mpz_com_fp;
_mpz_divexact_t               _mpz_divexact_fp;
_mpz_divexact_ui_t            _mpz_divexact_ui_fp;
_mpz_dump_t                _mpz_dump_fp;
_mpz_fac_ui_t              _mpz_fac_ui_fp;
_mpz_fdiv_q_t              _mpz_fdiv_q_fp;
_mpz_fdiv_q_2exp_t            _mpz_fdiv_q_2exp_fp;
_mpz_fdiv_q_ui_t           _mpz_fdiv_q_ui_fp;
_mpz_fdiv_qr_t             _mpz_fdiv_qr_fp;
_mpz_fdiv_qr_ui_t          _mpz_fdiv_qr_ui_fp;
_mpz_fdiv_r_t              _mpz_fdiv_r_fp;
_mpz_fdiv_r_2exp_t            _mpz_fdiv_r_2exp_fp;
_mpz_fdiv_r_ui_t           _mpz_fdiv_r_ui_fp;
_mpz_fib_ui_t              _mpz_fib_ui_fp;
_mpz_fib2_ui_t             _mpz_fib2_ui_fp;
_mpz_gcd_t                 _mpz_gcd_fp;
_mpz_gcd_ui_t              _mpz_gcd_ui_fp;
_mpz_gcdext_t              _mpz_gcdext_fp;
_mpz_get_d_2exp_t          _mpz_get_d_2exp_fp;
_mpz_get_str_t             _mpz_get_str_fp;
_mpz_init_t                _mpz_init_fp;
_mpz_init2_t               _mpz_init2_fp;
_mpz_init_set_t               _mpz_init_set_fp;
_mpz_init_set_d_t          _mpz_init_set_d_fp;
_mpz_init_set_si_t            _mpz_init_set_si_fp;
_mpz_init_set_str_t           _mpz_init_set_str_fp;
_mpz_init_set_ui_t            _mpz_init_set_ui_fp;
_mpz_inp_raw_t             _mpz_inp_raw_fp;
_mpz_inp_str_t             _mpz_inp_str_fp;
_mpz_invert_t              _mpz_invert_fp;
_mpz_ior_t                 _mpz_ior_fp;
_mpz_lcm_t                 _mpz_lcm_fp;
_mpz_lcm_ui_t              _mpz_lcm_ui_fp;
_mpz_lucnum_ui_t           _mpz_lucnum_ui_fp;
_mpz_lucnum2_ui_t          _mpz_lucnum2_ui_fp;
_mpz_mod_t                 _mpz_mod_fp;
_mpz_mul_t                 _mpz_mul_fp;
_mpz_mul_2exp_t               _mpz_mul_2exp_fp;
_mpz_mul_si_t              _mpz_mul_si_fp;
_mpz_mul_ui_t              _mpz_mul_ui_fp;
_mpz_neg_t                 _mpz_neg_fp;
_mpz_nextprime_t           _mpz_nextprime_fp;
_mpz_out_raw_t             _mpz_out_raw_fp;
_mpz_out_str_t             _mpz_out_str_fp;
_mpz_pow_ui_t              _mpz_pow_ui_fp;
_mpz_powm_t                _mpz_powm_fp;
_mpz_powm_ui_t             _mpz_powm_ui_fp;
_mpz_random_t              _mpz_random_fp;
_mpz_random2_t             _mpz_random2_fp;
_mpz_realloc2_t               _mpz_realloc2_fp;
_mpz_remove_t              _mpz_remove_fp;
_mpz_root_t                _mpz_root_fp;
_mpz_rrandomb_t               _mpz_rrandomb_fp;
_mpz_set_t                 _mpz_set_fp;
_mpz_set_d_t               _mpz_set_d_fp;
_mpz_set_f_t               _mpz_set_f_fp;
_mpz_set_q_t               _mpz_set_q_fp;
_mpz_set_si_t              _mpz_set_si_fp;
_mpz_set_str_t             _mpz_set_str_fp;
_mpz_set_ui_t              _mpz_set_ui_fp;
_mpz_setbit_t              _mpz_setbit_fp;
_mpz_sqrt_t                _mpz_sqrt_fp;
_mpz_sqrtrem_t             _mpz_sqrtrem_fp;
_mpz_sub_t                 _mpz_sub_fp;
_mpz_sub_ui_t              _mpz_sub_ui_fp;
_mpz_submul_t              _mpz_submul_fp;
_mpz_submul_ui_t           _mpz_submul_ui_fp;
_mpz_swap_t                _mpz_swap_fp;
_mpz_tdiv_q_t              _mpz_tdiv_q_fp;
_mpz_tdiv_q_2exp_t            _mpz_tdiv_q_2exp_fp;
_mpz_tdiv_q_ui_t           _mpz_tdiv_q_ui_fp;
_mpz_tdiv_qr_t             _mpz_tdiv_qr_fp;
_mpz_tdiv_qr_ui_t          _mpz_tdiv_qr_ui_fp;
_mpz_tdiv_r_t              _mpz_tdiv_r_fp;
_mpz_tdiv_r_2exp_t            _mpz_tdiv_r_2exp_fp;
_mpz_tdiv_r_ui_t           _mpz_tdiv_r_ui_fp;
_mpz_ui_pow_ui_t           _mpz_ui_pow_ui_fp;
_mpz_urandomb_t               _mpz_urandomb_fp;
_mpz_urandomm_t               _mpz_urandomm_fp;
_mpz_xor_t                 _mpz_xor_fp;
_mpz_cdiv_ui_t             _mpz_cdiv_ui_fp;
_mpz_cmp_t                 _mpz_cmp_fp;
_mpz_cmp_d_t               _mpz_cmp_d_fp;
__mpz_cmp_si_t             __mpz_cmp_si_fp;
__mpz_cmp_ui_t             __mpz_cmp_ui_fp;
_mpz_cmpabs_t              _mpz_cmpabs_fp;
_mpz_cmpabs_d_t               _mpz_cmpabs_d_fp;
_mpz_cmpabs_ui_t           _mpz_cmpabs_ui_fp;
_mpz_congruent_p_t            _mpz_congruent_p_fp;
_mpz_congruent_2exp_p_t       _mpz_congruent_2exp_p_fp;
_mpz_congruent_ui_p_t         _mpz_congruent_ui_p_fp;
_mpz_divisible_p_t            _mpz_divisible_p_fp;
_mpz_divisible_ui_p_t         _mpz_divisible_ui_p_fp;
_mpz_divisible_2exp_p_t       _mpz_divisible_2exp_p_fp;
_mpz_fdiv_ui_t             _mpz_fdiv_ui_fp;
_mpz_fits_sint_p_t            _mpz_fits_sint_p_fp;
_mpz_fits_slong_p_t           _mpz_fits_slong_p_fp;
_mpz_fits_sshort_p_t       _mpz_fits_sshort_p_fp;
_mpz_fits_uint_p_t            _mpz_fits_uint_p_fp;
_mpz_fits_ulong_p_t           _mpz_fits_ulong_p_fp;
_mpz_fits_ushort_p_t       _mpz_fits_ushort_p_fp;
_mpz_get_d_t               _mpz_get_d_fp;
_mpz_get_si_t              _mpz_get_si_fp;
_mpz_get_ui_t              _mpz_get_ui_fp;
_mpz_getlimbn_t               _mpz_getlimbn_fp;
_mpz_hamdist_t             _mpz_hamdist_fp;
_mpz_jacobi_t              _mpz_jacobi_fp;
_mpz_kronecker_si_t           _mpz_kronecker_si_fp;
_mpz_kronecker_ui_t           _mpz_kronecker_ui_fp;
_mpz_si_kronecker_t           _mpz_si_kronecker_fp;
_mpz_ui_kronecker_t           _mpz_ui_kronecker_fp;
_mpz_millerrabin_t            _mpz_millerrabin_fp;
_mpz_perfect_power_p_t        _mpz_perfect_power_p_fp;
_mpz_perfect_square_p_t       _mpz_perfect_square_p_fp;
_mpz_popcount_t               _mpz_popcount_fp;
_mpz_probab_prime_p_t         _mpz_probab_prime_p_fp;
_mpz_scan0_t               _mpz_scan0_fp;
_mpz_scan1_t               _mpz_scan1_fp;
_mpz_size_t                _mpz_size_fp;
_mpz_sizeinbase_t          _mpz_sizeinbase_fp;
_mpz_tdiv_ui_t             _mpz_tdiv_ui_fp;
_mpz_tstbit_t              _mpz_tstbit_fp;

#endif // #if !defined (__GMP__INCLUDE__NOTHING)

#endif // #if defined (_MSC_VER)    // Only useful code for VC
