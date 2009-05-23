// This file allows GMP to be used as a "staticly" linked library, and then at run time to
// try and load a libgmp-3.dll (or cyggmp-3.dll) from the current, %windows%, %winsystem%, or path (in that order).
// If the .DLL is found, then those functions are used.  If not, then the static linked functions are used.
// NOTE include this gmp.cxx file INSTEAD of gmp.h  We redefine ALL of the gmp function names to point to our
// function pointer data, so that calling mpz_init() will actually call the function pointer gmpz_init_fp() which
// will call mpz_init() from the DLL if loaded, else from the static linked mpz_init() function.

// This header (and the associated .cpp file, were written by Jim Fougeron for the OpenPFGW project

#if !defined (__GMP_DYNAMIC_HEADER)
#define __GMP_DYNAMIC_HEADER

#pragma warning ( disable : 4146 )
#include "gmp.h"


#if defined (_MSC_VER)
// ONLY useful for VC, (Borland also, but for now I don't care about Borland).  

#if defined (__cplusplus)
extern "C" {
#endif

// These are the functions which MUST be called first thing by the app, and then last thing before exiting.
// They "setup" the enviroment to call the correct function.  The function may be a staticlly linked GMP4 
// function, or a function from a libgmp-3.dll or a cyggmp-3.dll.  This is done so that a default GMP can
// be linked into the app, and if a user chooses, he can download the "best" DLL for his system, and the
// app will automatically use it without a recompile.
extern void Initialize_Dynamic_GMP_System();
extern void Free_Dynamic_GMP_System();
extern void ShowGMPLinkage();	// Shows if we are using static linked or the DLL.

// Data
//__GMP_DECLSPEC extern __gmp_const int mp_bits_per_limb;
//__GMP_DECLSPEC extern int gmp_errno;
//__GMP_DECLSPEC extern __gmp_const char * __gmp_const gmp_version;


// Functions

// Not sure how to handle these
//__GMP_DECLSPEC extern void * (*__gmp_allocate_func) (size_t));
//__GMP_DECLSPEC extern void * (*__gmp_reallocate_func) (void *, size_t, size_t));
//__GMP_DECLSPEC extern void   (*__gmp_free_func) (void *, size_t));
//__GMP_DECLSPEC std::ostream& operator<< (std::ostream &, mpz_srcptr);
//__GMP_DECLSPEC std::ostream& operator<< (std::ostream &, mpq_srcptr);
//__GMP_DECLSPEC std::ostream& operator<< (std::ostream &, mpf_srcptr);
//__GMP_DECLSPEC std::istream& operator>> (std::istream &, mpz_ptr);
//__GMP_DECLSPEC std::istream& operator>> (std::istream &, mpq_ptr);
//__GMP_DECLSPEC std::istream& operator>> (std::istream &, mpf_ptr);

// First, we typedef the functions

typedef void				__GMP_DECLSPEC (*_mp_set_memory_functions_t)	(void *(*) (size_t), void *(*) (void *, size_t, size_t), void (*) (void *, size_t));
typedef void				__GMP_DECLSPEC (*_gmp_randinit_t)				(gmp_randstate_t, gmp_randalg_t, ...);
typedef void				__GMP_DECLSPEC (*_gmp_randinit_default_t)		(gmp_randstate_t);
typedef void				__GMP_DECLSPEC (*_gmp_randinit_lc_t)			(gmp_randstate_t,mpz_srcptr, unsigned long int, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_gmp_randinit_lc_2exp_t)		(gmp_randstate_t, mpz_srcptr, unsigned long int, unsigned long int);
typedef int					__GMP_DECLSPEC (*_gmp_randinit_lc_2exp_size_t)	(gmp_randstate_t, unsigned long);
typedef void				__GMP_DECLSPEC (*_gmp_randseed_t)				(gmp_randstate_t, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_gmp_randseed_ui_t)			(gmp_randstate_t, unsigned long int);
typedef void				__GMP_DECLSPEC (*_gmp_randclear_t)				(gmp_randstate_t);
typedef int					__GMP_DECLSPEC (*_gmp_asprintf_t)				(char **, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_fprintf_t)				(FILE *, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_obstack_printf_t)			(struct obstack *, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_obstack_vprintf_t)		(struct obstack *, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_printf_t)					(const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_snprintf_t)				(char *, size_t, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_sprintf_t)				(char *, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_vasprintf_t)				(char **, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vfprintf_t)				(FILE *, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vprintf_t)				(const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vsnprintf_t)				(char *, size_t, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vsprintf_t)				(char *, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_fscanf_t)					(FILE *, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_scanf_t)					(const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_sscanf_t)					(const char *, const char *, ...);
typedef int					__GMP_DECLSPEC (*_gmp_vfscanf_t)				(FILE *, const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vscanf_t)					(const char *, va_list);
typedef int					__GMP_DECLSPEC (*_gmp_vsscanf_t)					(const char *, const char *, va_list);
typedef void				__GMP_DECLSPEC*(*__mpz_realloc_t)				(mpz_ptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpz_abs_t)					(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_add_t)						(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_add_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_addmul_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_addmul_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_and_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_array_init_t)				(mpz_ptr, mp_size_t, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpz_bin_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_bin_uiui_t)				(mpz_ptr, unsigned long int, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_cdiv_q_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_cdiv_q_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_cdiv_q_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_cdiv_qr_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_cdiv_qr_ui_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_cdiv_r_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_cdiv_r_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_cdiv_r_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_clear_t)					(mpz_ptr);
typedef void				__GMP_DECLSPEC (*_mpz_clrbit_t)					(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_com_t)					(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_divexact_t)				(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_divexact_ui_t)			(mpz_ptr, mpz_srcptr, unsigned long);
typedef void				__GMP_DECLSPEC (*_mpz_dump_t)					(mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_fac_ui_t)					(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_fdiv_q_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_fdiv_q_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long int);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_fdiv_q_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_fdiv_qr_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_fdiv_qr_ui_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_fdiv_r_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_fdiv_r_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long int);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_fdiv_r_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_fib_ui_t)					(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_fib2_ui_t)				(mpz_ptr, mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_gcd_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_gcd_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_gcdext_t)					(mpz_ptr, mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef double				__GMP_DECLSPEC (*_mpz_get_d_2exp_t)				(signed long int *, mpz_srcptr);
typedef char				__GMP_DECLSPEC*(*_mpz_get_str_t)				(char *, int, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_init_t)					(mpz_ptr);
typedef void				__GMP_DECLSPEC (*_mpz_init2_t)					(mpz_ptr, unsigned long);
typedef void				__GMP_DECLSPEC (*_mpz_init_set_t)				(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_init_set_d_t)				(mpz_ptr, double);
typedef void				__GMP_DECLSPEC (*_mpz_init_set_si_t)			(mpz_ptr, signed long int);
typedef int					__GMP_DECLSPEC (*_mpz_init_set_str_t)			(mpz_ptr, __gmp_const char *, int);
typedef void				__GMP_DECLSPEC (*_mpz_init_set_ui_t)			(mpz_ptr, unsigned long int);
typedef size_t				__GMP_DECLSPEC (*_mpz_inp_raw_t)				(mpz_ptr, FILE *);
typedef size_t				__GMP_DECLSPEC (*_mpz_inp_str_t)				(mpz_ptr, FILE *, int);
typedef int					__GMP_DECLSPEC (*_mpz_invert_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_ior_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_lcm_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_lcm_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long);
typedef void				__GMP_DECLSPEC (*_mpz_lucnum_ui_t)				(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_lucnum2_ui_t)				(mpz_ptr, mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_mod_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_mul_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_mul_2exp_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_mul_si_t)					(mpz_ptr, mpz_srcptr, long int);
typedef void				__GMP_DECLSPEC (*_mpz_mul_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_neg_t)					(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_nextprime_t)				(mpz_ptr, mpz_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpz_out_raw_t)				(FILE *, mpz_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpz_out_str_t)				(FILE *, int, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_pow_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_powm_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_powm_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_random_t)					(mpz_ptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpz_random2_t)				(mpz_ptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpz_realloc2_t)				(mpz_ptr, unsigned long);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_remove_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_root_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_rrandomb_t)				(mpz_ptr, gmp_randstate_t, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_set_t)					(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_set_d_t)					(mpz_ptr, double);
typedef void				__GMP_DECLSPEC (*_mpz_set_f_t)					(mpz_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_set_q_t)					(mpz_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_set_si_t)					(mpz_ptr, signed long int);
typedef int					__GMP_DECLSPEC (*_mpz_set_str_t)				(mpz_ptr, __gmp_const char *, int);
typedef void				__GMP_DECLSPEC (*_mpz_set_ui_t)					(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_setbit_t)					(mpz_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_sqrt_t)					(mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_sqrtrem_t)				(mpz_ptr, mpz_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_sub_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_sub_ui_t)					(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_submul_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_submul_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_swap_t)					(mpz_ptr, mpz_ptr);
typedef void				__GMP_DECLSPEC (*_mpz_tdiv_q_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_tdiv_q_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long int);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_tdiv_q_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_tdiv_qr_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_tdiv_qr_ui_t)				(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_tdiv_r_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_tdiv_r_2exp_t)			(mpz_ptr, mpz_srcptr, unsigned long int);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_tdiv_r_ui_t)				(mpz_ptr, mpz_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_ui_pow_ui_t)				(mpz_ptr, unsigned long int, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_urandomb_t)				(mpz_ptr, gmp_randstate_t, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpz_urandomm_t)				(mpz_ptr, gmp_randstate_t, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpz_xor_t)					(mpz_ptr, mpz_srcptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_abs_t)					(mpq_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_add_t)					(mpq_ptr, mpq_srcptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_canonicalize_t)			(mpq_ptr);
typedef void				__GMP_DECLSPEC (*_mpq_clear_t)					(mpq_ptr);
typedef void				__GMP_DECLSPEC (*_mpq_div_t)					(mpq_ptr, mpq_srcptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_div_2exp_t)				(mpq_ptr, mpq_srcptr, unsigned long);
typedef void				__GMP_DECLSPEC (*_mpq_get_num_t)				(mpz_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_get_den_t)				(mpz_ptr, mpq_srcptr);
typedef char				__GMP_DECLSPEC*(*_mpq_get_str_t)				(char *, int, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_init_t)					(mpq_ptr);
typedef size_t				__GMP_DECLSPEC (*_mpq_inp_str_t)				(mpq_ptr, FILE *, int);
typedef void				__GMP_DECLSPEC (*_mpq_inv_t)					(mpq_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_mul_t)					(mpq_ptr, mpq_srcptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_mul_2exp_t)				(mpq_ptr, mpq_srcptr, unsigned long);
typedef void				__GMP_DECLSPEC (*_mpq_neg_t)					(mpq_ptr, mpq_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpq_out_str_t)				(FILE *, int, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_set_t)					(mpq_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_set_d_t)					(mpq_ptr, double);
typedef void				__GMP_DECLSPEC (*_mpq_set_den_t)				(mpq_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_set_f_t)					(mpq_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_set_num_t)				(mpq_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_set_si_t)					(mpq_ptr, signed long int, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpq_set_str_t)				(mpq_ptr, const char *, int);
typedef void				__GMP_DECLSPEC (*_mpq_set_ui_t)					(mpq_ptr, unsigned long int, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpq_set_z_t)					(mpq_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_sub_t)					(mpq_ptr, mpq_srcptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpq_swap_t)					(mpq_ptr, mpq_ptr);
typedef void				__GMP_DECLSPEC (*_mpf_abs_t)					(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_add_t)					(mpf_ptr, mpf_srcptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_add_ui_t)					(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_ceil_t)					(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_clear_t)					(mpf_ptr);
typedef void				__GMP_DECLSPEC (*_mpf_div_t)					(mpf_ptr, mpf_srcptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_div_2exp_t)				(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_div_ui_t)					(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_dump_t)					(mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_floor_t)					(mpf_ptr, mpf_srcptr);
typedef double				__GMP_DECLSPEC (*_mpf_get_d_2exp_t)				(signed long int *, mpf_srcptr);
typedef char				__GMP_DECLSPEC*(*_mpf_get_str_t)				(char *, mp_exp_t *, int, size_t, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_init_t)					(mpf_ptr);
typedef void				__GMP_DECLSPEC (*_mpf_init2_t)					(mpf_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_init_set_t)				(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_init_set_d_t)				(mpf_ptr, double);
typedef void				__GMP_DECLSPEC (*_mpf_init_set_si_t)			(mpf_ptr, signed long int);
typedef int					__GMP_DECLSPEC (*_mpf_init_set_str_t)			(mpf_ptr, __gmp_const char *, int);
typedef void				__GMP_DECLSPEC (*_mpf_init_set_ui_t)			(mpf_ptr, unsigned long int);
typedef size_t				__GMP_DECLSPEC (*_mpf_inp_str_t)				(mpf_ptr, FILE *, int);
typedef void				__GMP_DECLSPEC (*_mpf_mul_t)					(mpf_ptr, mpf_srcptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_mul_2exp_t)				(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_mul_ui_t)					(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_neg_t)					(mpf_ptr, mpf_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpf_out_str_t)				(FILE *, int, size_t, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_pow_ui_t)					(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_random2_t)				(mpf_ptr, mp_size_t, mp_exp_t);
typedef void				__GMP_DECLSPEC (*_mpf_reldiff_t)				(mpf_ptr, mpf_srcptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_set_t)					(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_set_d_t)					(mpf_ptr, double);
typedef void				__GMP_DECLSPEC (*_mpf_set_default_prec_t)		(unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_set_prec_t)				(mpf_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_set_prec_raw_t)			(mpf_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_set_q_t)					(mpf_ptr, mpq_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_set_si_t)					(mpf_ptr, signed long int);
typedef int					__GMP_DECLSPEC (*_mpf_set_str_t)				(mpf_ptr, __gmp_const char *, int);
typedef void				__GMP_DECLSPEC (*_mpf_set_ui_t)					(mpf_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_set_z_t)					(mpf_ptr, mpz_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_sqrt_t)					(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_sqrt_ui_t)				(mpf_ptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_sub_t)					(mpf_ptr, mpf_srcptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_sub_ui_t)					(mpf_ptr, mpf_srcptr, unsigned long int);
typedef void				__GMP_DECLSPEC (*_mpf_swap_t)					(mpf_ptr, mpf_ptr);
typedef void				__GMP_DECLSPEC (*_mpf_trunc_t)					(mpf_ptr, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_ui_div_t)					(mpf_ptr, unsigned long int, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_ui_sub_t)					(mpf_ptr, unsigned long int, mpf_srcptr);
typedef void				__GMP_DECLSPEC (*_mpf_urandomb_t)				(mpf_t, gmp_randstate_t, unsigned long int);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_add_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr,mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_add_1_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_add_n_t)					(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_add_nc_t)					(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_addmul_1_t)				(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_addmul_1c_t)				(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_addsub_n_t)				(mp_ptr, mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_addsub_nc_t)				(mp_ptr, mp_ptr, mp_srcptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_bdivmod_t)				(mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t, unsigned long int);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_divexact_by3c_t)			(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_divrem_t)					(mp_ptr, mp_size_t, mp_ptr, mp_size_t, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_divrem_1_t)				(mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_divrem_1c_t)				(mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_divrem_2_t)				(mp_ptr, mp_size_t, mp_ptr, mp_size_t, mp_srcptr);
typedef void				__GMP_DECLSPEC (*_mpn_dump_t)					(mp_srcptr, mp_size_t);
typedef mp_size_t			__GMP_DECLSPEC (*_mpn_gcd_t)					(mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
typedef mp_size_t			__GMP_DECLSPEC (*_mpn_gcdext_t)					(mp_ptr, mp_ptr, mp_size_t *, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
typedef size_t				__GMP_DECLSPEC (*_mpn_get_str_t)				(unsigned char *, int, mp_ptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_lshift_t)					(mp_ptr, mp_srcptr, mp_size_t, unsigned int);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_mul_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_mul_1_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_mul_1c_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef void				__GMP_DECLSPEC (*_mpn_mul_basecase_t)			(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpn_mul_n_t)					(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpn_random_t)					(mp_ptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpn_random2_t)				(mp_ptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_rshift_t)					(mp_ptr, mp_srcptr, mp_size_t, unsigned int);
typedef mp_size_t			__GMP_DECLSPEC (*_mpn_set_str_t)				(mp_ptr, __gmp_const unsigned char *, size_t, int);
typedef void				__GMP_DECLSPEC (*_mpn_sqr_n_t)					(mp_ptr, mp_srcptr, mp_size_t);
typedef void				__GMP_DECLSPEC (*_mpn_sqr_basecase_t)			(mp_ptr, mp_srcptr, mp_size_t);
typedef mp_size_t			__GMP_DECLSPEC (*_mpn_sqrtrem_t)				(mp_ptr, mp_ptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_sub_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr,mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_sub_1_t)					(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_sub_n_t)					(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_sub_nc_t)					(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_submul_1_t)				(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_submul_1c_t)				(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef void				__GMP_DECLSPEC (*_mpn_tdiv_qr_t)					(mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
// GMP "pure" fuctions  (See gmp.h for a description of __GMP_ATTRIBUTE_PURE functions.  For VC, this is meaningless)
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_cdiv_ui_t)				(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_cmp_t)					(mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_cmp_d_t)					(mpz_srcptr, double);
typedef int					__GMP_DECLSPEC (*__mpz_cmp_si_t)				(mpz_srcptr, signed long int);
typedef int					__GMP_DECLSPEC (*__mpz_cmp_ui_t)				(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_cmpabs_t)					(mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_cmpabs_d_t)				(mpz_srcptr, double);
typedef int					__GMP_DECLSPEC (*_mpz_cmpabs_ui_t)				(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_congruent_p_t)			(mpz_srcptr, mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_congruent_2exp_p_t)		(mpz_srcptr, mpz_srcptr, unsigned long);
typedef int					__GMP_DECLSPEC (*_mpz_congruent_ui_p_t)			(mpz_srcptr, unsigned long, unsigned long);
typedef int					__GMP_DECLSPEC (*_mpz_divisible_p_t)			(mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_divisible_ui_p_t)			(mpz_srcptr, unsigned long);
typedef int					__GMP_DECLSPEC (*_mpz_divisible_2exp_p_t)		(mpz_srcptr, unsigned long);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_fdiv_ui_t)				(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_fits_sint_p_t)			(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_fits_slong_p_t)			(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_fits_sshort_p_t)			(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_fits_uint_p_t)			(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_fits_ulong_p_t)			(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_fits_ushort_p_t)			(mpz_srcptr);
typedef double				__GMP_DECLSPEC (*_mpz_get_d_t)					(mpz_srcptr);
typedef /*signed*/ long int	__GMP_DECLSPEC (*_mpz_get_si_t)					(mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_get_ui_t)					(mpz_srcptr);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpz_getlimbn_t)				(mpz_srcptr, mp_size_t);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_hamdist_t)				(mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_jacobi_t)					(mpz_srcptr, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_kronecker_si_t)			(mpz_srcptr, long);
typedef int					__GMP_DECLSPEC (*_mpz_kronecker_ui_t)			(mpz_srcptr, unsigned long);
typedef int					__GMP_DECLSPEC (*_mpz_si_kronecker_t)			(long, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_ui_kronecker_t)			(unsigned long, mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_millerrabin_t)			(mpz_srcptr, int);
typedef int					__GMP_DECLSPEC (*_mpz_perfect_power_p_t)		(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_perfect_square_p_t)		(mpz_srcptr);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_popcount_t)				(mpz_srcptr);
typedef int					__GMP_DECLSPEC (*_mpz_probab_prime_p_t)			(mpz_srcptr, int);
typedef unsigned long int	__GMP_DECLSPEC (*_mpz_scan0_t)					(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_scan1_t)					(mpz_srcptr, unsigned long int);
typedef size_t				__GMP_DECLSPEC (*_mpz_size_t)					(mpz_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpz_sizeinbase_t)				(mpz_srcptr, int);
typedef int					__GMP_DECLSPEC (*_mpz_tdiv_ui_t)				(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpz_tstbit_t)					(mpz_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpq_cmp_t)					(mpq_srcptr, mpq_srcptr);
typedef int					__GMP_DECLSPEC (*__mpq_cmp_si_t)				(mpq_srcptr, long, unsigned long);
typedef int					__GMP_DECLSPEC (*__mpq_cmp_ui_t)				(mpq_srcptr, unsigned long int, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpq_equal_t)					(mpq_srcptr, mpq_srcptr);
typedef double				__GMP_DECLSPEC (*_mpq_get_d_t)					(mpq_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_cmp_t)					(mpf_srcptr, mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_cmp_d_t)					(mpf_srcptr, double);
typedef int					__GMP_DECLSPEC (*_mpf_cmp_si_t)					(mpf_srcptr, signed long int);
typedef int					__GMP_DECLSPEC (*_mpf_cmp_ui_t)					(mpf_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpf_eq_t)						(mpf_srcptr, mpf_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpf_fits_sint_p_t)			(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_fits_slong_p_t)			(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_fits_sshort_p_t)			(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_fits_uint_p_t)			(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_fits_ulong_p_t)			(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_fits_ushort_p_t)			(mpf_srcptr);
typedef double				__GMP_DECLSPEC (*_mpf_get_d_t)					(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_get_default_prec_t)		(void);
typedef int					__GMP_DECLSPEC (*_mpf_get_prec_t)				(mpf_srcptr);
typedef long				__GMP_DECLSPEC (*_mpf_get_si_t)					(mpf_srcptr);
typedef unsigned long		__GMP_DECLSPEC (*_mpf_get_ui_t)					(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpf_integer_p_t)				(mpf_srcptr);
typedef size_t				__GMP_DECLSPEC (*_mpf_size_t)					(mpf_srcptr);
typedef int					__GMP_DECLSPEC (*_mpn_cmp_t)					(mp_srcptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_gcd_1_t)					(mp_srcptr, mp_size_t, mp_limb_t);
typedef int					__GMP_DECLSPEC (*_mpn_hamdist_t)				(mp_srcptr, mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_mod_1_t)					(mp_srcptr, mp_size_t, mp_limb_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_mod_1c_t)					(mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef int					__GMP_DECLSPEC (*_mpn_perfect_square_p_t)		(mp_srcptr, mp_size_t);
typedef int					__GMP_DECLSPEC (*_mpn_popcount_t)				(mp_srcptr, mp_size_t);
typedef mp_limb_t			__GMP_DECLSPEC (*_mpn_preinv_mod_1_t)			(mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);
typedef int					__GMP_DECLSPEC (*_mpn_scan0_t)					(mp_srcptr, unsigned long int);
typedef int					__GMP_DECLSPEC (*_mpn_scan1_t)					(mp_srcptr, unsigned long int);

// Now we define the function pointers
extern _mp_set_memory_functions_t	_mp_set_memory_functions_fp;
extern _gmp_randinit_t				_gmp_randinit_fp;
extern _gmp_randinit_default_t		_gmp_randinit_default_fp;
extern _gmp_randinit_lc_t			_gmp_randinit_lc_fp;
extern _gmp_randinit_lc_2exp_t		_gmp_randinit_lc_2exp_fp;
extern _gmp_randinit_lc_2exp_size_t	_gmp_randinit_lc_2exp_size_fp;
extern _gmp_randseed_t				_gmp_randseed_fp;
extern _gmp_randseed_ui_t			_gmp_randseed_ui_fp;
extern _gmp_randclear_t				_gmp_randclear_fp;
extern _gmp_asprintf_t				_gmp_asprintf_fp;
extern _gmp_fprintf_t				_gmp_fprintf_fp;
extern _gmp_obstack_printf_t		_gmp_obstack_printf_fp;
extern _gmp_obstack_vprintf_t		_gmp_obstack_vprintf_fp;
extern _gmp_printf_t				_gmp_printf_fp;
extern _gmp_snprintf_t				_gmp_snprintf_fp;
extern _gmp_sprintf_t				_gmp_sprintf_fp;
extern _gmp_vasprintf_t				_gmp_vasprintf_fp;
extern _gmp_vfprintf_t				_gmp_vfprintf_fp;
extern _gmp_vprintf_t				_gmp_vprintf_fp;
extern _gmp_vsnprintf_t				_gmp_vsnprintf_fp;
extern _gmp_vsprintf_t				_gmp_vsprintf_fp;
extern _gmp_fscanf_t				_gmp_fscanf_fp;
extern _gmp_scanf_t					_gmp_scanf_fp;
extern _gmp_sscanf_t				_gmp_sscanf_fp;
extern _gmp_vfscanf_t				_gmp_vfscanf_fp;
extern _gmp_vscanf_t				_gmp_vscanf_fp;
extern _gmp_vsscanf_t				_gmp_vsscanf_fp;
extern __mpz_realloc_t				__mpz_realloc_fp;
extern _mpz_abs_t					_mpz_abs_fp;
extern _mpz_add_t					_mpz_add_fp;
extern _mpz_add_ui_t				_mpz_add_ui_fp;
extern _mpz_addmul_t				_mpz_addmul_fp;
extern _mpz_addmul_ui_t				_mpz_addmul_ui_fp;
extern _mpz_and_t					_mpz_and_fp;
extern _mpz_array_init_t			_mpz_array_init_fp;
extern _mpz_bin_ui_t				_mpz_bin_ui_fp;
extern _mpz_bin_uiui_t				_mpz_bin_uiui_fp;
extern _mpz_cdiv_q_t				_mpz_cdiv_q_fp;
extern _mpz_cdiv_q_2exp_t			_mpz_cdiv_q_2exp_fp;
extern _mpz_cdiv_q_ui_t				_mpz_cdiv_q_ui_fp;
extern _mpz_cdiv_qr_t				_mpz_cdiv_qr_fp;
extern _mpz_cdiv_qr_ui_t			_mpz_cdiv_qr_ui_fp;
extern _mpz_cdiv_r_t				_mpz_cdiv_r_fp;
extern _mpz_cdiv_r_2exp_t			_mpz_cdiv_r_2exp_fp;
extern _mpz_cdiv_r_ui_t				_mpz_cdiv_r_ui_fp;
extern _mpz_clear_t					_mpz_clear_fp;
extern _mpz_clrbit_t				_mpz_clrbit_fp;
extern _mpz_com_t					_mpz_com_fp;
extern _mpz_divexact_t				_mpz_divexact_fp;
extern _mpz_divexact_ui_t			_mpz_divexact_ui_fp;
extern _mpz_dump_t					_mpz_dump_fp;
extern _mpz_fac_ui_t				_mpz_fac_ui_fp;
extern _mpz_fdiv_q_t				_mpz_fdiv_q_fp;
extern _mpz_fdiv_q_2exp_t			_mpz_fdiv_q_2exp_fp;
extern _mpz_fdiv_q_ui_t				_mpz_fdiv_q_ui_fp;
extern _mpz_fdiv_qr_t				_mpz_fdiv_qr_fp;
extern _mpz_fdiv_qr_ui_t			_mpz_fdiv_qr_ui_fp;
extern _mpz_fdiv_r_t				_mpz_fdiv_r_fp;
extern _mpz_fdiv_r_2exp_t			_mpz_fdiv_r_2exp_fp;
extern _mpz_fdiv_r_ui_t				_mpz_fdiv_r_ui_fp;
extern _mpz_fib_ui_t				_mpz_fib_ui_fp;
extern _mpz_fib2_ui_t				_mpz_fib2_ui_fp;
extern _mpz_gcd_t					_mpz_gcd_fp;
extern _mpz_gcd_ui_t				_mpz_gcd_ui_fp;
extern _mpz_gcdext_t				_mpz_gcdext_fp;
extern _mpz_get_d_2exp_t			_mpz_get_d_2exp_fp;
extern _mpz_get_str_t				_mpz_get_str_fp;
extern _mpz_init_t					_mpz_init_fp;
extern _mpz_init2_t					_mpz_init2_fp;
extern _mpz_init_set_t				_mpz_init_set_fp;
extern _mpz_init_set_d_t			_mpz_init_set_d_fp;
extern _mpz_init_set_si_t			_mpz_init_set_si_fp;
extern _mpz_init_set_str_t			_mpz_init_set_str_fp;
extern _mpz_init_set_ui_t			_mpz_init_set_ui_fp;
extern _mpz_inp_raw_t				_mpz_inp_raw_fp;
extern _mpz_inp_str_t				_mpz_inp_str_fp;
extern _mpz_invert_t				_mpz_invert_fp;
extern _mpz_ior_t					_mpz_ior_fp;
extern _mpz_lcm_t					_mpz_lcm_fp;
extern _mpz_lcm_ui_t				_mpz_lcm_ui_fp;
extern _mpz_lucnum_ui_t				_mpz_lucnum_ui_fp;
extern _mpz_lucnum2_ui_t			_mpz_lucnum2_ui_fp;
extern _mpz_mod_t					_mpz_mod_fp;
extern _mpz_mul_t					_mpz_mul_fp;
extern _mpz_mul_2exp_t				_mpz_mul_2exp_fp;
extern _mpz_mul_si_t				_mpz_mul_si_fp;
extern _mpz_mul_ui_t				_mpz_mul_ui_fp;
extern _mpz_neg_t					_mpz_neg_fp;
extern _mpz_nextprime_t				_mpz_nextprime_fp;
extern _mpz_out_raw_t				_mpz_out_raw_fp;
extern _mpz_out_str_t				_mpz_out_str_fp;
extern _mpz_pow_ui_t				_mpz_pow_ui_fp;
extern _mpz_powm_t					_mpz_powm_fp;
extern _mpz_powm_ui_t				_mpz_powm_ui_fp;
extern _mpz_random_t				_mpz_random_fp;
extern _mpz_random2_t				_mpz_random2_fp;
extern _mpz_realloc2_t				_mpz_realloc2_fp;
extern _mpz_remove_t				_mpz_remove_fp;
extern _mpz_root_t					_mpz_root_fp;
extern _mpz_rrandomb_t				_mpz_rrandomb_fp;
extern _mpz_set_t					_mpz_set_fp;
extern _mpz_set_d_t					_mpz_set_d_fp;
extern _mpz_set_f_t					_mpz_set_f_fp;
extern _mpz_set_q_t					_mpz_set_q_fp;
extern _mpz_set_si_t				_mpz_set_si_fp;
extern _mpz_set_str_t				_mpz_set_str_fp;
extern _mpz_set_ui_t				_mpz_set_ui_fp;
extern _mpz_setbit_t				_mpz_setbit_fp;
extern _mpz_sqrt_t					_mpz_sqrt_fp;
extern _mpz_sqrtrem_t				_mpz_sqrtrem_fp;
extern _mpz_sub_t					_mpz_sub_fp;
extern _mpz_sub_ui_t				_mpz_sub_ui_fp;
extern _mpz_submul_t				_mpz_submul_fp;
extern _mpz_submul_ui_t				_mpz_submul_ui_fp;
extern _mpz_swap_t					_mpz_swap_fp;
extern _mpz_tdiv_q_t				_mpz_tdiv_q_fp;
extern _mpz_tdiv_q_2exp_t			_mpz_tdiv_q_2exp_fp;
extern _mpz_tdiv_q_ui_t				_mpz_tdiv_q_ui_fp;
extern _mpz_tdiv_qr_t				_mpz_tdiv_qr_fp;
extern _mpz_tdiv_qr_ui_t			_mpz_tdiv_qr_ui_fp;
extern _mpz_tdiv_r_t				_mpz_tdiv_r_fp;
extern _mpz_tdiv_r_2exp_t			_mpz_tdiv_r_2exp_fp;
extern _mpz_tdiv_r_ui_t				_mpz_tdiv_r_ui_fp;
extern _mpz_ui_pow_ui_t				_mpz_ui_pow_ui_fp;
extern _mpz_urandomb_t				_mpz_urandomb_fp;
extern _mpz_urandomm_t				_mpz_urandomm_fp;
extern _mpz_xor_t					_mpz_xor_fp;
extern _mpq_abs_t					_mpq_abs_fp;
extern _mpq_add_t					_mpq_add_fp;
extern _mpq_canonicalize_t			_mpq_canonicalize_fp;
extern _mpq_clear_t					_mpq_clear_fp;
extern _mpq_div_t					_mpq_div_fp;
extern _mpq_div_2exp_t				_mpq_div_2exp_fp;
extern _mpq_get_num_t				_mpq_get_num_fp;
extern _mpq_get_den_t				_mpq_get_den_fp;
extern _mpq_get_str_t				_mpq_get_str_fp;
extern _mpq_init_t					_mpq_init_fp;
extern _mpq_inp_str_t				_mpq_inp_str_fp;
extern _mpq_inv_t					_mpq_inv_fp;
extern _mpq_mul_t					_mpq_mul_fp;
extern _mpq_mul_2exp_t				_mpq_mul_2exp_fp;
extern _mpq_neg_t					_mpq_neg_fp;
extern _mpq_out_str_t				_mpq_out_str_fp;
extern _mpq_set_t					_mpq_set_fp;
extern _mpq_set_d_t					_mpq_set_d_fp;
extern _mpq_set_den_t				_mpq_set_den_fp;
extern _mpq_set_f_t					_mpq_set_f_fp;
extern _mpq_set_num_t				_mpq_set_num_fp;
extern _mpq_set_si_t				_mpq_set_si_fp;
extern _mpq_set_str_t				_mpq_set_str_fp;
extern _mpq_set_ui_t				_mpq_set_ui_fp;
extern _mpq_set_z_t					_mpq_set_z_fp;
extern _mpq_sub_t					_mpq_sub_fp;
extern _mpq_swap_t					_mpq_swap_fp;
extern _mpf_abs_t					_mpf_abs_fp;
extern _mpf_add_t					_mpf_add_fp;
extern _mpf_add_ui_t				_mpf_add_ui_fp;
extern _mpf_ceil_t					_mpf_ceil_fp;
extern _mpf_clear_t					_mpf_clear_fp;
extern _mpf_div_t					_mpf_div_fp;
extern _mpf_div_2exp_t				_mpf_div_2exp_fp;
extern _mpf_div_ui_t				_mpf_div_ui_fp;
extern _mpf_dump_t					_mpf_dump_fp;
extern _mpf_floor_t					_mpf_floor_fp;
extern _mpf_get_d_2exp_t			_mpf_get_d_2exp_fp;
extern _mpf_get_str_t				_mpf_get_str_fp;
extern _mpf_init_t					_mpf_init_fp;
extern _mpf_init2_t					_mpf_init2_fp;
extern _mpf_init_set_t				_mpf_init_set_fp;
extern _mpf_init_set_d_t			_mpf_init_set_d_fp;
extern _mpf_init_set_si_t			_mpf_init_set_si_fp;
extern _mpf_init_set_str_t			_mpf_init_set_str_fp;
extern _mpf_init_set_ui_t			_mpf_init_set_ui_fp;
extern _mpf_inp_str_t				_mpf_inp_str_fp;
extern _mpf_mul_t					_mpf_mul_fp;
extern _mpf_mul_2exp_t				_mpf_mul_2exp_fp;
extern _mpf_mul_ui_t				_mpf_mul_ui_fp;
extern _mpf_neg_t					_mpf_neg_fp;
extern _mpf_out_str_t				_mpf_out_str_fp;
extern _mpf_pow_ui_t				_mpf_pow_ui_fp;
extern _mpf_random2_t				_mpf_random2_fp;
extern _mpf_reldiff_t				_mpf_reldiff_fp;
extern _mpf_set_t					_mpf_set_fp;
extern _mpf_set_d_t					_mpf_set_d_fp;
extern _mpf_set_default_prec_t		_mpf_set_default_prec_fp;
extern _mpf_set_prec_t				_mpf_set_prec_fp;
extern _mpf_set_prec_raw_t			_mpf_set_prec_raw_fp;
extern _mpf_set_q_t					_mpf_set_q_fp;
extern _mpf_set_si_t				_mpf_set_si_fp;
extern _mpf_set_str_t				_mpf_set_str_fp;
extern _mpf_set_ui_t				_mpf_set_ui_fp;
extern _mpf_set_z_t					_mpf_set_z_fp;
extern _mpf_sqrt_t					_mpf_sqrt_fp;
extern _mpf_sqrt_ui_t				_mpf_sqrt_ui_fp;
extern _mpf_sub_t					_mpf_sub_fp;
extern _mpf_sub_ui_t				_mpf_sub_ui_fp;
extern _mpf_swap_t					_mpf_swap_fp;
extern _mpf_trunc_t					_mpf_trunc_fp;
extern _mpf_ui_div_t				_mpf_ui_div_fp;
extern _mpf_ui_sub_t				_mpf_ui_sub_fp;
extern _mpf_urandomb_t				_mpf_urandomb_fp;
extern _mpn_add_t					_mpn_add_fp;
extern _mpn_add_1_t					_mpn_add_1_fp;
extern _mpn_add_n_t					_mpn_add_n_fp;
extern _mpn_add_nc_t				_mpn_add_nc_fp;
extern _mpn_addmul_1_t				_mpn_addmul_1_fp;
extern _mpn_addmul_1c_t				_mpn_addmul_1c_fp;
extern _mpn_addsub_n_t				_mpn_addsub_n_fp;
extern _mpn_addsub_nc_t				_mpn_addsub_nc_fp;
extern _mpn_bdivmod_t				_mpn_bdivmod_fp;
extern _mpn_divexact_by3c_t			_mpn_divexact_by3c_fp;
extern _mpn_divrem_t				_mpn_divrem_fp;
extern _mpn_divrem_1_t				_mpn_divrem_1_fp;
extern _mpn_divrem_1c_t				_mpn_divrem_1c_fp;
extern _mpn_divrem_2_t				_mpn_divrem_2_fp;
extern _mpn_dump_t					_mpn_dump_fp;
extern _mpn_gcd_t					_mpn_gcd_fp;
extern _mpn_gcdext_t				_mpn_gcdext_fp;
extern _mpn_get_str_t				_mpn_get_str_fp;
extern _mpn_lshift_t				_mpn_lshift_fp;
extern _mpn_mul_t					_mpn_mul_fp;
extern _mpn_mul_1_t					_mpn_mul_1_fp;
extern _mpn_mul_1c_t				_mpn_mul_1c_fp;
extern _mpn_mul_basecase_t			_mpn_mul_basecase_fp;
extern _mpn_mul_n_t					_mpn_mul_n_fp;
extern _mpn_random_t				_mpn_random_fp;
extern _mpn_random2_t				_mpn_random2_fp;
extern _mpn_rshift_t				_mpn_rshift_fp;
extern _mpn_set_str_t				_mpn_set_str_fp;
extern _mpn_sqr_n_t					_mpn_sqr_n_fp;
extern _mpn_sqr_basecase_t			_mpn_sqr_basecase_fp;
extern _mpn_sqrtrem_t				_mpn_sqrtrem_fp;
extern _mpn_sub_t					_mpn_sub_fp;
extern _mpn_sub_1_t					_mpn_sub_1_fp;
extern _mpn_sub_n_t					_mpn_sub_n_fp;
extern _mpn_sub_nc_t				_mpn_sub_nc_fp;
extern _mpn_submul_1_t				_mpn_submul_1_fp;
extern _mpn_submul_1c_t				_mpn_submul_1c_fp;
extern _mpn_tdiv_qr_t				_mpn_tdiv_qr_fp;
extern _mpz_cdiv_ui_t				_mpz_cdiv_ui_fp;
extern _mpz_cmp_t					_mpz_cmp_fp;
extern _mpz_cmp_d_t					_mpz_cmp_d_fp;
extern __mpz_cmp_si_t				__mpz_cmp_si_fp;
extern __mpz_cmp_ui_t				__mpz_cmp_ui_fp;
extern _mpz_cmpabs_t				_mpz_cmpabs_fp;
extern _mpz_cmpabs_d_t				_mpz_cmpabs_d_fp;
extern _mpz_cmpabs_ui_t				_mpz_cmpabs_ui_fp;
extern _mpz_congruent_p_t			_mpz_congruent_p_fp;
extern _mpz_congruent_2exp_p_t		_mpz_congruent_2exp_p_fp;
extern _mpz_congruent_ui_p_t		_mpz_congruent_ui_p_fp;
extern _mpz_divisible_p_t			_mpz_divisible_p_fp;
extern _mpz_divisible_ui_p_t		_mpz_divisible_ui_p_fp;
extern _mpz_divisible_2exp_p_t		_mpz_divisible_2exp_p_fp;
extern _mpz_fdiv_ui_t				_mpz_fdiv_ui_fp;
extern _mpz_fits_sint_p_t			_mpz_fits_sint_p_fp;
extern _mpz_fits_slong_p_t			_mpz_fits_slong_p_fp;
extern _mpz_fits_sshort_p_t			_mpz_fits_sshort_p_fp;
extern _mpz_fits_uint_p_t			_mpz_fits_uint_p_fp;
extern _mpz_fits_ulong_p_t			_mpz_fits_ulong_p_fp;
extern _mpz_fits_ushort_p_t			_mpz_fits_ushort_p_fp;
extern _mpz_get_d_t					_mpz_get_d_fp;
extern _mpz_get_si_t				_mpz_get_si_fp;
extern _mpz_get_ui_t				_mpz_get_ui_fp;
extern _mpz_getlimbn_t				_mpz_getlimbn_fp;
extern _mpz_hamdist_t				_mpz_hamdist_fp;
extern _mpz_jacobi_t				_mpz_jacobi_fp;
extern _mpz_kronecker_si_t			_mpz_kronecker_si_fp;
extern _mpz_kronecker_ui_t			_mpz_kronecker_ui_fp;
extern _mpz_si_kronecker_t			_mpz_si_kronecker_fp;
extern _mpz_ui_kronecker_t			_mpz_ui_kronecker_fp;
extern _mpz_millerrabin_t			_mpz_millerrabin_fp;
extern _mpz_perfect_power_p_t		_mpz_perfect_power_p_fp;
extern _mpz_perfect_square_p_t		_mpz_perfect_square_p_fp;
extern _mpz_popcount_t				_mpz_popcount_fp;
extern _mpz_probab_prime_p_t		_mpz_probab_prime_p_fp;
extern _mpz_scan0_t					_mpz_scan0_fp;
extern _mpz_scan1_t					_mpz_scan1_fp;
extern _mpz_size_t					_mpz_size_fp;
extern _mpz_sizeinbase_t			_mpz_sizeinbase_fp;
extern _mpz_tdiv_ui_t				_mpz_tdiv_ui_fp;
extern _mpz_tstbit_t				_mpz_tstbit_fp;
extern _mpq_cmp_t					_mpq_cmp_fp;
extern __mpq_cmp_si_t				__mpq_cmp_si_fp;
extern __mpq_cmp_ui_t				__mpq_cmp_ui_fp;
extern _mpq_equal_t					_mpq_equal_fp;
extern _mpq_get_d_t					_mpq_get_d_fp;
extern _mpf_cmp_t					_mpf_cmp_fp;
extern _mpf_cmp_d_t					_mpf_cmp_d_fp;
extern _mpf_cmp_si_t				_mpf_cmp_si_fp;
extern _mpf_cmp_ui_t				_mpf_cmp_ui_fp;
extern _mpf_eq_t					_mpf_eq_fp;
extern _mpf_fits_sint_p_t			_mpf_fits_sint_p_fp;
extern _mpf_fits_slong_p_t			_mpf_fits_slong_p_fp;
extern _mpf_fits_sshort_p_t			_mpf_fits_sshort_p_fp;
extern _mpf_fits_uint_p_t			_mpf_fits_uint_p_fp;
extern _mpf_fits_ulong_p_t			_mpf_fits_ulong_p_fp;
extern _mpf_fits_ushort_p_t			_mpf_fits_ushort_p_fp;
extern _mpf_get_d_t					_mpf_get_d_fp;
extern _mpf_get_default_prec_t		_mpf_get_default_prec_fp;
extern _mpf_get_prec_t				_mpf_get_prec_fp;
extern _mpf_get_si_t				_mpf_get_si_fp;
extern _mpf_get_ui_t				_mpf_get_ui_fp;
extern _mpf_integer_p_t				_mpf_integer_p_fp;
extern _mpf_size_t					_mpf_size_fp;
extern _mpn_cmp_t					_mpn_cmp_fp;
extern _mpn_gcd_1_t					_mpn_gcd_1_fp;
extern _mpn_hamdist_t				_mpn_hamdist_fp;
extern _mpn_mod_1_t					_mpn_mod_1_fp;
extern _mpn_mod_1c_t				_mpn_mod_1c_fp;
extern _mpn_perfect_square_p_t		_mpn_perfect_square_p_fp;
extern _mpn_popcount_t				_mpn_popcount_fp;
extern _mpn_preinv_mod_1_t			_mpn_preinv_mod_1_fp;
extern _mpn_scan0_t					_mpn_scan0_fp;
extern _mpn_scan1_t					_mpn_scan1_fp;


#if defined (__cplusplus)
}
#endif


#if !defined (_GMP_STATIC_DYNAMIC_FUNCTIONS_)

// NOTE the gmp_dynamic.cpp will NOT get into this code!  It needs to know the "real" names of the GMP functions

// Undefine what was done in gmp.h since we want all calls to these defines to go to our function pointers (then to GMP)
#undef mp_set_memory_functions
#undef gmp_randinit
#undef gmp_randinit_default
#undef gmp_randinit_lc
#undef gmp_randinit_lc_2exp
#undef gmp_randinit_lc_2exp_size
#undef gmp_randseed
#undef gmp_randseed_ui
#undef gmp_randclear
#undef gmp_asprintf
#undef gmp_fprintf
#undef gmp_obstack_printf
#undef gmp_obstack_vprintf
#undef gmp_printf
#undef gmp_snprintf
#undef gmp_sprintf
#undef gmp_vasprintf
#undef gmp_vfprintf
#undef gmp_vprintf
#undef gmp_vsnprintf
#undef gmp_vsprintf
#undef gmp_fscanf
#undef gmp_scanf
#undef gmp_sscanf
#undef gmp_vfscanf
#undef gmp_vscanf
#undef gmp_vsscanf
#undef _mpz_realloc
#undef mpz_abs
#undef mpz_add
#undef mpz_add_ui
#undef mpz_addmul
#undef mpz_addmul_ui
#undef mpz_and
#undef mpz_array_init
#undef mpz_bin_ui
#undef mpz_bin_uiui
#undef mpz_cdiv_q
#undef mpz_cdiv_q_2exp
#undef mpz_cdiv_q_ui
#undef mpz_cdiv_qr
#undef mpz_cdiv_qr_ui
#undef mpz_cdiv_r
#undef mpz_cdiv_r_2exp
#undef mpz_cdiv_r_ui
#undef mpz_clear
#undef mpz_clrbit
#undef mpz_com
#undef mpz_divexact
#undef mpz_divexact_ui
#undef mpz_dump
#undef mpz_fac_ui
#undef mpz_fdiv_q
#undef mpz_fdiv_q_2exp
#undef mpz_fdiv_q_ui
#undef mpz_fdiv_qr
#undef mpz_fdiv_qr_ui
#undef mpz_fdiv_r
#undef mpz_fdiv_r_2exp
#undef mpz_fdiv_r_ui
#undef mpz_fib_ui
#undef mpz_fib2_ui
#undef mpz_gcd
#undef mpz_gcd_ui
#undef mpz_gcdext
#undef mpz_get_d_2exp
#undef mpz_get_str
#undef mpz_init
#undef mpz_init2
#undef mpz_init_set
#undef mpz_init_set_d
#undef mpz_init_set_si
#undef mpz_init_set_str
#undef mpz_init_set_ui
#undef mpz_inp_raw
#undef mpz_inp_str
#undef mpz_invert
#undef mpz_ior
#undef mpz_lcm
#undef mpz_lcm_ui
#undef mpz_lucnum_ui
#undef mpz_lucnum2_ui
#undef mpz_mod
#undef mpz_mul
#undef mpz_mul_2exp
#undef mpz_mul_si
#undef mpz_mul_ui
#undef mpz_neg
#undef mpz_nextprime
#undef mpz_out_raw
#undef mpz_out_str
#undef mpz_pow_ui
#undef mpz_powm
#undef mpz_powm_ui
#undef mpz_random
#undef mpz_random2
#undef mpz_realloc2
#undef mpz_remove
#undef mpz_root
#undef mpz_rrandomb
#undef mpz_set
#undef mpz_set_d
#undef mpz_set_f
#undef mpz_set_q
#undef mpz_set_si
#undef mpz_set_str
#undef mpz_set_ui
#undef mpz_setbit
#undef mpz_sqrt
#undef mpz_sqrtrem
#undef mpz_sub
#undef mpz_sub_ui
#undef mpz_submul
#undef mpz_submul_ui
#undef mpz_swap
#undef mpz_tdiv_q
#undef mpz_tdiv_q_2exp
#undef mpz_tdiv_q_ui
#undef mpz_tdiv_qr
#undef mpz_tdiv_qr_ui
#undef mpz_tdiv_r
#undef mpz_tdiv_r_2exp
#undef mpz_tdiv_r_ui
#undef mpz_ui_pow_ui
#undef mpz_urandomb
#undef mpz_urandomm
#undef mpz_xor
#undef mpq_abs
#undef mpq_add
#undef mpq_canonicalize
#undef mpq_clear
#undef mpq_div
#undef mpq_div_2exp
#undef mpq_get_num
#undef mpq_get_den
#undef mpq_get_str
#undef mpq_init
#undef mpq_inp_str
#undef mpq_inv
#undef mpq_mul
#undef mpq_mul_2exp
#undef mpq_neg
#undef mpq_out_str
#undef mpq_set
#undef mpq_set_d
#undef mpq_set_den
#undef mpq_set_f
#undef mpq_set_num
#undef mpq_set_si
#undef mpq_set_str
#undef mpq_set_ui
#undef mpq_set_z
#undef mpq_sub
#undef mpq_swap
#undef mpf_abs
#undef mpf_add
#undef mpf_add_ui
#undef mpf_ceil
#undef mpf_clear
#undef mpf_div
#undef mpf_div_2exp
#undef mpf_div_ui
#undef mpf_dump
#undef mpf_floor
#undef mpf_get_d_2exp
#undef mpf_get_str
#undef mpf_init
#undef mpf_init2
#undef mpf_init_set
#undef mpf_init_set_d
#undef mpf_init_set_si
#undef mpf_init_set_str
#undef mpf_init_set_ui
#undef mpf_inp_str
#undef mpf_mul
#undef mpf_mul_2exp
#undef mpf_mul_ui
#undef mpf_neg
#undef mpf_out_str
#undef mpf_pow_ui
#undef mpf_random2
#undef mpf_reldiff
#undef mpf_set
#undef mpf_set_d
#undef mpf_set_default_prec
#undef mpf_set_prec
#undef mpf_set_prec_raw
#undef mpf_set_q
#undef mpf_set_si
#undef mpf_set_str
#undef mpf_set_ui
#undef mpf_set_z
#undef mpf_sqrt
#undef mpf_sqrt_ui
#undef mpf_sub
#undef mpf_sub_ui
#undef mpf_swap
#undef mpf_trunc
#undef mpf_ui_div
#undef mpf_ui_sub
#undef mpf_urandomb
#undef mpn_add
#undef mpn_add_1
#undef mpn_add_n
#undef mpn_add_nc
#undef mpn_addmul_1
#undef mpn_addmul_1c
#undef mpn_addsub_n
#undef mpn_addsub_nc
#undef mpn_bdivmod
#undef mpn_divexact_by3c
#undef mpn_divrem
#undef mpn_divrem_1
#undef mpn_divrem_1c
#undef mpn_divrem_2
#undef mpn_dump
#undef mpn_gcd
#undef mpn_gcdext
#undef mpn_get_str
#undef mpn_lshift
#undef mpn_mul
#undef mpn_mul_1
#undef mpn_mul_1c
#undef mpn_mul_basecase
#undef mpn_mul_n
#undef mpn_random
#undef mpn_random2
#undef mpn_rshift
#undef mpn_set_str
#undef mpn_sqr_n
#undef mpn_sqr_basecase
#undef mpn_sqrtrem
#undef mpn_sub
#undef mpn_sub_1
#undef mpn_sub_n
#undef mpn_sub_nc
#undef mpn_submul_1
#undef mpn_submul_1c
#undef mpn_tdiv_qr
#undef mpz_cdiv_ui
#undef mpz_cmp
#undef mpz_cmp_d
#undef _mpz_cmp_si
#undef _mpz_cmp_ui
#undef mpz_cmpabs
#undef mpz_cmpabs_d
#undef mpz_cmpabs_ui
#undef mpz_congruent_p
#undef mpz_congruent_2exp_p
#undef mpz_congruent_ui_p
#undef mpz_divisible_p
#undef mpz_divisible_ui_p
#undef mpz_divisible_2exp_p
#undef mpz_fdiv_ui
#undef mpz_fits_sint_p
#undef mpz_fits_slong_p
#undef mpz_fits_sshort_p
#undef mpz_fits_uint_p
#undef mpz_fits_ulong_p
#undef mpz_fits_ushort_p
#undef mpz_get_d
#undef mpz_get_si
#undef mpz_get_ui
#undef mpz_getlimbn
#undef mpz_hamdist
#undef mpz_jacobi
#undef mpz_kronecker_si
#undef mpz_kronecker_ui
#undef mpz_si_kronecker
#undef mpz_ui_kronecker
#undef mpz_millerrabin
#undef mpz_perfect_power_p
#undef mpz_perfect_square_p
#undef mpz_popcount
#undef mpz_probab_prime_p
#undef mpz_scan0
#undef mpz_scan1
#undef mpz_size
#undef mpz_sizeinbase
#undef mpz_tdiv_ui
#undef mpz_tstbit
#undef mpq_cmp
#undef _mpq_cmp_si
#undef _mpq_cmp_ui
#undef mpq_equal
#undef mpq_get_d
#undef mpf_cmp
#undef mpf_cmp_d
#undef mpf_cmp_si
#undef mpf_cmp_ui
#undef mpf_eq
#undef mpf_fits_sint_p
#undef mpf_fits_slong_p
#undef mpf_fits_sshort_p
#undef mpf_fits_uint_p
#undef mpf_fits_ulong_p
#undef mpf_fits_ushort_p
#undef mpf_get_d
#undef mpf_get_default_prec
#undef mpf_get_prec
#undef mpf_get_si
#undef mpf_get_ui
#undef mpf_integer_p
#undef mpf_size
#undef mpn_cmp
#undef mpn_gcd_1
#undef mpn_hamdist
#undef mpn_mod_1
#undef mpn_mod_1c
#undef mpn_perfect_square_p
#undef mpn_popcount
#undef mpn_preinv_mod_1
#undef mpn_scan0
#undef mpn_scan1

// NOW redefine the GMP functions to call our function pointers

#define mp_set_memory_functions		_mp_set_memory_functions_fp
#define gmp_randinit				_gmp_randinit_fp
#define gmp_randinit_default		_gmp_randinit_default_fp
#define gmp_randinit_lc				_gmp_randinit_lc_fp
#define gmp_randinit_lc_2exp		_gmp_randinit_lc_2exp_fp
#define gmp_randinit_lc_2exp_size	_gmp_randinit_lc_2exp_size_fp
#define gmp_randseed				_gmp_randseed_fp
#define gmp_randseed_ui				_gmp_randseed_ui_fp
#define gmp_randclear				_gmp_randclear_fp
#define gmp_asprintf				_gmp_asprintf_fp
#define gmp_fprintf					_gmp_fprintf_fp
#define gmp_obstack_printf			_gmp_obstack_printf_fp
#define gmp_obstack_vprintf			_gmp_obstack_vprintf_fp
#define gmp_printf					_gmp_printf_fp
#define gmp_snprintf				_gmp_snprintf_fp
#define gmp_sprintf					_gmp_sprintf_fp
#define gmp_vasprintf				_gmp_vasprintf_fp
#define gmp_vfprintf				_gmp_vfprintf_fp
#define gmp_vprintf					_gmp_vprintf_fp
#define gmp_vsnprintf				_gmp_vsnprintf_fp
#define gmp_vsprintf				_gmp_vsprintf_fp
#define gmp_fscanf					_gmp_fscanf_fp
#define gmp_scanf					_gmp_scanf_fp
#define gmp_sscanf					_gmp_sscanf_fp
#define gmp_vfscanf					_gmp_vfscanf_fp
#define gmp_vscanf					_gmp_vscanf_fp
#define gmp_vsscanf					_gmp_vsscanf_fp
#define _mpz_realloc				__mpz_realloc_fp
#define mpz_abs						_mpz_abs_fp
#define mpz_add						_mpz_add_fp
#define mpz_add_ui					_mpz_add_ui_fp
#define mpz_addmul					_mpz_addmul_fp
#define mpz_addmul_ui				_mpz_addmul_ui_fp
#define mpz_and						_mpz_and_fp
#define mpz_array_init				_mpz_array_init_fp
#define mpz_bin_ui					_mpz_bin_ui_fp
#define mpz_bin_uiui				_mpz_bin_uiui_fp
#define mpz_cdiv_q					_mpz_cdiv_q_fp
#define mpz_cdiv_q_2exp				_mpz_cdiv_q_2exp_fp
#define mpz_cdiv_q_ui				_mpz_cdiv_q_ui_fp
#define mpz_cdiv_qr					_mpz_cdiv_qr_fp
#define mpz_cdiv_qr_ui				_mpz_cdiv_qr_ui_fp
#define mpz_cdiv_r					_mpz_cdiv_r_fp
#define mpz_cdiv_r_2exp				_mpz_cdiv_r_2exp_fp
#define mpz_cdiv_r_ui				_mpz_cdiv_r_ui_fp
#define mpz_clear					_mpz_clear_fp
#define mpz_clrbit					_mpz_clrbit_fp
#define mpz_com						_mpz_com_fp
#define mpz_divexact				_mpz_divexact_fp
#define mpz_divexact_ui				_mpz_divexact_ui_fp
#define mpz_dump					_mpz_dump_fp
#define mpz_fac_ui					_mpz_fac_ui_fp
#define mpz_fdiv_q					_mpz_fdiv_q_fp
#define mpz_fdiv_q_2exp				_mpz_fdiv_q_2exp_fp
#define mpz_fdiv_q_ui				_mpz_fdiv_q_ui_fp
#define mpz_fdiv_qr					_mpz_fdiv_qr_fp
#define mpz_fdiv_qr_ui				_mpz_fdiv_qr_ui_fp
#define mpz_fdiv_r					_mpz_fdiv_r_fp
#define mpz_fdiv_r_2exp				_mpz_fdiv_r_2exp_fp
#define mpz_fdiv_r_ui				_mpz_fdiv_r_ui_fp
#define mpz_fib_ui					_mpz_fib_ui_fp
#define mpz_fib2_ui					_mpz_fib2_ui_fp
#define mpz_gcd						_mpz_gcd_fp
#define mpz_gcd_ui					_mpz_gcd_ui_fp
#define mpz_gcdext					_mpz_gcdext_fp
#define mpz_get_d_2exp				_mpz_get_d_2exp_fp
#define mpz_get_str					_mpz_get_str_fp
#define mpz_init					_mpz_init_fp
#define mpz_init2					_mpz_init2_fp
#define mpz_init_set				_mpz_init_set_fp
#define mpz_init_set_d				_mpz_init_set_d_fp
#define mpz_init_set_si				_mpz_init_set_si_fp
#define mpz_init_set_str			_mpz_init_set_str_fp
#define mpz_init_set_ui				_mpz_init_set_ui_fp
#define mpz_inp_raw					_mpz_inp_raw_fp
#define mpz_inp_str					_mpz_inp_str_fp
#define mpz_invert					_mpz_invert_fp
#define mpz_ior						_mpz_ior_fp
#define mpz_lcm						_mpz_lcm_fp
#define mpz_lcm_ui					_mpz_lcm_ui_fp
#define mpz_lucnum_ui				_mpz_lucnum_ui_fp
#define mpz_lucnum2_ui				_mpz_lucnum2_ui_fp
#define mpz_mod						_mpz_mod_fp
#define mpz_mul						_mpz_mul_fp
#define mpz_mul_2exp				_mpz_mul_2exp_fp
#define mpz_mul_si					_mpz_mul_si_fp
#define mpz_mul_ui					_mpz_mul_ui_fp
#define mpz_neg						_mpz_neg_fp
#define mpz_nextprime				_mpz_nextprime_fp
#define mpz_out_raw					_mpz_out_raw_fp
#define mpz_out_str					_mpz_out_str_fp
#define mpz_pow_ui					_mpz_pow_ui_fp
#define mpz_powm					_mpz_powm_fp
#define mpz_powm_ui					_mpz_powm_ui_fp
#define mpz_random					_mpz_random_fp
#define mpz_random2					_mpz_random2_fp
#define mpz_realloc2				_mpz_realloc2_fp
#define mpz_remove					_mpz_remove_fp
#define mpz_root					_mpz_root_fp
#define mpz_rrandomb				_mpz_rrandomb_fp
#define mpz_set						_mpz_set_fp
#define mpz_set_d					_mpz_set_d_fp
#define mpz_set_f					_mpz_set_f_fp
#define mpz_set_q					_mpz_set_q_fp
#define mpz_set_si					_mpz_set_si_fp
#define mpz_set_str					_mpz_set_str_fp
#define mpz_set_ui					_mpz_set_ui_fp
#define mpz_setbit					_mpz_setbit_fp
#define mpz_sqrt					_mpz_sqrt_fp
#define mpz_sqrtrem					_mpz_sqrtrem_fp
#define mpz_sub						_mpz_sub_fp
#define mpz_sub_ui					_mpz_sub_ui_fp
#define mpz_submul					_mpz_submul_fp
#define mpz_submul_ui				_mpz_submul_ui_fp
#define mpz_swap					_mpz_swap_fp
#define mpz_tdiv_q					_mpz_tdiv_q_fp
#define mpz_tdiv_q_2exp				_mpz_tdiv_q_2exp_fp
#define mpz_tdiv_q_ui				_mpz_tdiv_q_ui_fp
#define mpz_tdiv_qr					_mpz_tdiv_qr_fp
#define mpz_tdiv_qr_ui				_mpz_tdiv_qr_ui_fp
#define mpz_tdiv_r					_mpz_tdiv_r_fp
#define mpz_tdiv_r_2exp				_mpz_tdiv_r_2exp_fp
#define mpz_tdiv_r_ui				_mpz_tdiv_r_ui_fp
#define mpz_ui_pow_ui				_mpz_ui_pow_ui_fp
#define mpz_urandomb				_mpz_urandomb_fp
#define mpz_urandomm				_mpz_urandomm_fp
#define mpz_xor						_mpz_xor_fp
#define mpq_abs						_mpq_abs_fp
#define mpq_add						_mpq_add_fp
#define mpq_canonicalize			_mpq_canonicalize_fp
#define mpq_clear					_mpq_clear_fp
#define mpq_div						_mpq_div_fp
#define mpq_div_2exp				_mpq_div_2exp_fp
#define mpq_get_num					_mpq_get_num_fp
#define mpq_get_den					_mpq_get_den_fp
#define mpq_get_str					_mpq_get_str_fp
#define mpq_init					_mpq_init_fp
#define mpq_inp_str					_mpq_inp_str_fp
#define mpq_inv						_mpq_inv_fp
#define mpq_mul						_mpq_mul_fp
#define mpq_mul_2exp				_mpq_mul_2exp_fp
#define mpq_neg						_mpq_neg_fp
#define mpq_out_str					_mpq_out_str_fp
#define mpq_set						_mpq_set_fp
#define mpq_set_d					_mpq_set_d_fp
#define mpq_set_den					_mpq_set_den_fp
#define mpq_set_f					_mpq_set_f_fp
#define mpq_set_num					_mpq_set_num_fp
#define mpq_set_si					_mpq_set_si_fp
#define mpq_set_str					_mpq_set_str_fp
#define mpq_set_ui					_mpq_set_ui_fp
#define mpq_set_z					_mpq_set_z_fp
#define mpq_sub						_mpq_sub_fp
#define mpq_swap					_mpq_swap_fp
#define mpf_abs						_mpf_abs_fp
#define mpf_add						_mpf_add_fp
#define mpf_add_ui					_mpf_add_ui_fp
#define mpf_ceil					_mpf_ceil_fp
#define mpf_clear					_mpf_clear_fp
#define mpf_div						_mpf_div_fp
#define mpf_div_2exp				_mpf_div_2exp_fp
#define mpf_div_ui					_mpf_div_ui_fp
#define mpf_dump					_mpf_dump_fp
#define mpf_floor					_mpf_floor_fp
#define mpf_get_d_2exp				_mpf_get_d_2exp_fp
#define mpf_get_str					_mpf_get_str_fp
#define mpf_init					_mpf_init_fp
#define mpf_init2					_mpf_init2_fp
#define mpf_init_set				_mpf_init_set_fp
#define mpf_init_set_d				_mpf_init_set_d_fp
#define mpf_init_set_si				_mpf_init_set_si_fp
#define mpf_init_set_str			_mpf_init_set_str_fp
#define mpf_init_set_ui				_mpf_init_set_ui_fp
#define mpf_inp_str					_mpf_inp_str_fp
#define mpf_mul						_mpf_mul_fp
#define mpf_mul_2exp				_mpf_mul_2exp_fp
#define mpf_mul_ui					_mpf_mul_ui_fp
#define mpf_neg						_mpf_neg_fp
#define mpf_out_str					_mpf_out_str_fp
#define mpf_pow_ui					_mpf_pow_ui_fp
#define mpf_random2					_mpf_random2_fp
#define mpf_reldiff					_mpf_reldiff_fp
#define mpf_set						_mpf_set_fp
#define mpf_set_d					_mpf_set_d_fp
#define mpf_set_default_prec		_mpf_set_default_prec_fp
#define mpf_set_prec				_mpf_set_prec_fp
#define mpf_set_prec_raw			_mpf_set_prec_raw_fp
#define mpf_set_q					_mpf_set_q_fp
#define mpf_set_si					_mpf_set_si_fp
#define mpf_set_str					_mpf_set_str_fp
#define mpf_set_ui					_mpf_set_ui_fp
#define mpf_set_z					_mpf_set_z_fp
#define mpf_sqrt					_mpf_sqrt_fp
#define mpf_sqrt_ui					_mpf_sqrt_ui_fp
#define mpf_sub						_mpf_sub_fp
#define mpf_sub_ui					_mpf_sub_ui_fp
#define mpf_swap					_mpf_swap_fp
#define mpf_trunc					_mpf_trunc_fp
#define mpf_ui_div					_mpf_ui_div_fp
#define mpf_ui_sub					_mpf_ui_sub_fp
#define mpf_urandomb				_mpf_urandomb_fp
#define mpn_add						_mpn_add_fp
#define mpn_add_1					_mpn_add_1_fp
#define mpn_add_n					_mpn_add_n_fp
#define mpn_add_nc					_mpn_add_nc_fp
#define mpn_addmul_1				_mpn_addmul_1_fp
#define mpn_addmul_1c				_mpn_addmul_1c_fp
#define mpn_addsub_n				_mpn_addsub_n_fp
#define mpn_addsub_nc				_mpn_addsub_nc_fp
#define mpn_bdivmod					_mpn_bdivmod_fp
#define mpn_divexact_by3c			_mpn_divexact_by3c_fp
#define mpn_divrem					_mpn_divrem_fp
#define mpn_divrem_1				_mpn_divrem_1_fp
#define mpn_divrem_1c				_mpn_divrem_1c_fp
#define mpn_divrem_2				_mpn_divrem_2_fp
#define mpn_dump					_mpn_dump_fp
#define mpn_gcd						_mpn_gcd_fp
#define mpn_gcdext					_mpn_gcdext_fp
#define mpn_get_str					_mpn_get_str_fp
#define mpn_lshift					_mpn_lshift_fp
#define mpn_mul						_mpn_mul_fp
#define mpn_mul_1					_mpn_mul_1_fp
#define mpn_mul_1c					_mpn_mul_1c_fp
#define mpn_mul_basecase			_mpn_mul_basecase_fp
#define mpn_mul_n					_mpn_mul_n_fp
#define mpn_random					_mpn_random_fp
#define mpn_random2					_mpn_random2_fp
#define mpn_rshift					_mpn_rshift_fp
#define mpn_set_str					_mpn_set_str_fp
#define mpn_sqr_n					_mpn_sqr_n_fp
#define mpn_sqr_basecase			_mpn_sqr_basecase_fp
#define mpn_sqrtrem					_mpn_sqrtrem_fp
#define mpn_sub						_mpn_sub_fp
#define mpn_sub_1					_mpn_sub_1_fp
#define mpn_sub_n					_mpn_sub_n_fp
#define mpn_sub_nc					_mpn_sub_nc_fp
#define mpn_submul_1				_mpn_submul_1_fp
#define mpn_submul_1c				_mpn_submul_1c_fp
#define mpn_tdiv_qr					_mpn_tdiv_qr_fp
#define mpz_cdiv_ui					_mpz_cdiv_ui_fp
#define mpz_cmp						_mpz_cmp_fp
#define mpz_cmp_d					_mpz_cmp_d_fp
#define _mpz_cmp_si					__mpz_cmp_si_fp
#define _mpz_cmp_ui					__mpz_cmp_ui_fp
#define mpz_cmpabs					_mpz_cmpabs_fp
#define mpz_cmpabs_d				_mpz_cmpabs_d_fp
#define mpz_cmpabs_ui				_mpz_cmpabs_ui_fp
#define mpz_congruent_p				_mpz_congruent_p_fp
#define mpz_congruent_2exp_p		_mpz_congruent_2exp_p_fp
#define mpz_congruent_ui_p			_mpz_congruent_ui_p_fp
#define mpz_divisible_p				_mpz_divisible_p_fp
#define mpz_divisible_ui_p			_mpz_divisible_ui_p_fp
#define mpz_divisible_2exp_p		_mpz_divisible_2exp_p_fp
#define mpz_fdiv_ui					_mpz_fdiv_ui_fp
#define mpz_fits_sint_p				_mpz_fits_sint_p_fp
#define mpz_fits_slong_p			_mpz_fits_slong_p_fp
#define mpz_fits_sshort_p			_mpz_fits_sshort_p_fp
#define mpz_fits_uint_p				_mpz_fits_uint_p_fp
#define mpz_fits_ulong_p			_mpz_fits_ulong_p_fp
#define mpz_fits_ushort_p			_mpz_fits_ushort_p_fp
#define mpz_get_d					_mpz_get_d_fp
#define mpz_get_si					_mpz_get_si_fp
#define mpz_get_ui					_mpz_get_ui_fp
#define mpz_getlimbn				_mpz_getlimbn_fp
#define mpz_hamdist					_mpz_hamdist_fp
#define mpz_jacobi					_mpz_jacobi_fp
#define mpz_kronecker_si			_mpz_kronecker_si_fp
#define mpz_kronecker_ui			_mpz_kronecker_ui_fp
#define mpz_si_kronecker			_mpz_si_kronecker_fp
#define mpz_ui_kronecker			_mpz_ui_kronecker_fp
#define mpz_millerrabin				_mpz_millerrabin_fp
#define mpz_perfect_power_p			_mpz_perfect_power_p_fp
#define mpz_perfect_square_p		_mpz_perfect_square_p_fp
#define mpz_popcount				_mpz_popcount_fp
#define mpz_probab_prime_p			_mpz_probab_prime_p_fp
#define mpz_scan0					_mpz_scan0_fp
#define mpz_scan1					_mpz_scan1_fp
#define mpz_size					_mpz_size_fp
#define mpz_sizeinbase				_mpz_sizeinbase_fp
#define mpz_tdiv_ui					_mpz_tdiv_ui_fp
#define mpz_tstbit					_mpz_tstbit_fp
#define mpq_cmp						_mpq_cmp_fp
#define _mpq_cmp_si					__mpq_cmp_si_fp
#define _mpq_cmp_ui					__mpq_cmp_ui_fp
#define mpq_equal					_mpq_equal_fp
#define mpq_get_d					_mpq_get_d_fp
#define mpf_cmp						_mpf_cmp_fp
#define mpf_cmp_d					_mpf_cmp_d_fp
#define mpf_cmp_si					_mpf_cmp_si_fp
#define mpf_cmp_ui					_mpf_cmp_ui_fp
#define mpf_eq						_mpf_eq_fp
#define mpf_fits_sint_p				_mpf_fits_sint_p_fp
#define mpf_fits_slong_p			_mpf_fits_slong_p_fp
#define mpf_fits_sshort_p			_mpf_fits_sshort_p_fp
#define mpf_fits_uint_p				_mpf_fits_uint_p_fp
#define mpf_fits_ulong_p			_mpf_fits_ulong_p_fp
#define mpf_fits_ushort_p			_mpf_fits_ushort_p_fp
#define mpf_get_d					_mpf_get_d_fp
#define mpf_get_default_prec		_mpf_get_default_prec_fp
#define mpf_get_prec				_mpf_get_prec_fp
#define mpf_get_si					_mpf_get_si_fp
#define mpf_get_ui					_mpf_get_ui_fp
#define mpf_integer_p				_mpf_integer_p_fp
#define mpf_size					_mpf_size_fp
#define mpn_cmp						_mpn_cmp_fp
#define mpn_gcd_1					_mpn_gcd_1_fp
#define mpn_hamdist					_mpn_hamdist_fp
#define mpn_mod_1					_mpn_mod_1_fp
#define mpn_mod_1c					_mpn_mod_1c_fp
#define mpn_perfect_square_p		_mpn_perfect_square_p_fp
#define mpn_popcount				_mpn_popcount_fp
#define mpn_preinv_mod_1			_mpn_preinv_mod_1_fp
#define mpn_scan0					_mpn_scan0_fp
#define mpn_scan1					_mpn_scan1_fp

#endif // #if !defined (_GMP_STATIC_DYNAMIC_FUNCTIONS_)

// This class (and static objects) make SURE that GMP is intiallized, and initialized
// once BEFORE any static variables which access a GMP object can happen.  This CAN and
// does happen before main, with globals and statics.
class GMP__Constructor_Class
{
	static unsigned m_nCnt;
	public:
		// not 100% thread safe usage of m_nCnt, but should not hurt anything.
	GMP__Constructor_Class() {if(m_nCnt++ == 0)Initialize_Dynamic_GMP_System();}
	~GMP__Constructor_Class() {if(--m_nCnt == 0)Free_Dynamic_GMP_System();}
};
// now EACH module will get one of these objects.  They are are static to the module, and there
// is NOTHING more than a small constructor call done.  This assures us that one of the modules
// will call the GMP allocator BEFORE ANY accesses can happen at all to the GMP system (unless
// some module includes "gmp.h"
static GMP__Constructor_Class GMP__Constructor_Class_build_me_first_thing;

// FINALLY we are done!
#endif  // #if defined (_MSC_VER)

#endif  // #if !defined (__GMP_DYNAMIC_HEADER)
