# local macro definitions for OpenPFGW

dnl  PFGW_CHECK_ASM_UNDERSCORE([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl  Shamelessly borrowed from glibc/gmp
AC_DEFUN(PFGW_CHECK_ASM_UNDERSCORE,
[AC_CACHE_CHECK([if symbols are prefixed by underscore], 
	        pfgw_cv_check_asm_underscore,
[cat > conftest.$ac_ext <<EOF
dnl This sometimes fails to find confdefs.h, for some reason.
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
#include "confdefs.h"
int underscore_test() {
return; }
EOF
if AC_TRY_EVAL(ac_compile); then
  if grep _underscore_test conftest* >/dev/null; then
    pfgw_cv_check_asm_underscore=yes
  else
    pfgw_cv_check_asm_underscore=no
  fi
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
fi
rm -f conftest*
])
if test "$pfgw_cv_check_asm_underscore" = "yes"; then
  ifelse([$1], , :, [$1])
else
  ifelse([$2], , :, [$2])
fi    
])

dnl	PFGW_CHECK_CFLAGS_INTERNAL(FLAGS,ACTION-IF-FOUND,ACTION-IF-NOT_FOUND)
AC_DEFUN(PFGW_CHECK_CFLAGS_INTERNAL,
[
AC_LANG_SAVE
AC_LANG_C
pfgw_check_cflags_save=$CFLAGS
CFLAGS="$CFLAGS $1"

AC_TRY_COMPILE(,[return 0;],pfgw_check_cflags_ok=yes,pfgw_check_cflags_ok=no)

if test "$pfgw_check_cflags_ok" = "yes"; then
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , :, [$3])
fi

CFLAGS=$pfgw_check_cflags_save
AC_LANG_RESTORE
])

dnl	PFGW_CHECK_CXXFLAGS_INTERNAL(FLAGS,ACTION-IF-FOUND,ACTION-IF-NOT_FOUND)
AC_DEFUN(PFGW_CHECK_CXXFLAGS_INTERNAL,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
pfgw_check_cxxflags_save=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $1"

AC_TRY_COMPILE(,[return 0;],pfgw_check_cxxflags_ok=yes,pfgw_check_cxxflags_ok=no)

if test "$pfgw_check_cxxflags_ok" = "yes"; then
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , :, [$3])
fi

CXXFLAGS=$pfgw_check_cxxflags_save
AC_LANG_RESTORE
])

dnl The following is the code that includes a cache variable
dnl and a prompt, flags,cache,prompt,yesaction,noaction
AC_DEFUN(PFGW_CHECK_CFLAGS,
[AC_CACHE_CHECK([$1],[$2],
[
	PFGW_CHECK_CFLAGS_INTERNAL([$3],[$2]=yes,[$2]=no)
])
if test "$[$2]" = "yes"; then
	ifelse([$4], , :, [$4])
else
	ifelse([$5], , :, [$5])
fi
])

AC_DEFUN(PFGW_CHECK_CXXFLAGS,
[AC_CACHE_CHECK([$1],[$2],
[
	PFGW_CHECK_CXXFLAGS_INTERNAL([$3],[$2]=yes,[$2]=no)
])
if test "$[$2]" = "yes"; then
	ifelse([$4], , :, [$4])
else
	ifelse([$5], , :, [$5])
fi
])

AC_DEFUN(PFGW_GETDEFAULT_FLAGS,[
AC_CACHE_CHECK([project-required C flags],pfgw_cv_cflags,[
pfgw_cv_cflags="-ffloat-store"
])
AC_CACHE_CHECK([project-required C++ flags],pfgw_cv_cxxflags,[
pfgw_cv_cxxflags="-ffloat-store"
])
AC_CACHE_CHECK([project-preferred C optimizations],pfgw_cv_coptflags,[
pfgw_cv_coptflags="-funroll-loops -fomit-frame-pointer -malign-double"
])
AC_CACHE_CHECK([project-preferred C++ optimizations],pfgw_cv_cxxoptflags,[
pfgw_cv_cxxoptflags="-funroll-loops -fomit-frame-pointer -malign-double"
])
AC_CACHE_CHECK([project-preferred C warnings],pfgw_cv_cwarnflags,[
pfgw_cv_cwarnflags="-Wall -Wno-multichar -Wno-long-long -Wshadow -Wbad-function-cast -Wcast-qual -Wconversion  -Winline -Wextern-inline  -Wwrite-strings"
])
AC_CACHE_CHECK([project-preferred C++ warnings],pfgw_cv_cxxwarnflags,[
pfgw_cv_cxxwarnflags="-Wall -Wno-multichar -Wno-ctor-dtor-privacy -Wno-long-long -Wshadow -Wbad-function-cast -Wcast-qual -Wconversion  -Winline -Wextern-inline  -Wwrite-strings -Woverloaded-virtual"
])
AC_CACHE_CHECK([extra C warnings],pfgw_cv_xcwarnflags,[
pfgw_cv_xcwarnflags=""
])
AC_CACHE_CHECK([extra C++ warnings],pfgw_cv_xcxxwarnflags,[
pfgw_cv_xcxxwarnflags=""
])
])

AC_DEFUN(PFGW_CHECKDEFAULT_FLAGS,[
pfgwcflags=$pfgw_cv_cflags
pfgwcxxflags=$pfgw_cv_cxxflags
PFGW_CHECK_CFLAGS([[whether preferred optimizations work in C]],pfgw_cv_copt,$pfgw_cv_coptflags,pfgwcoptflags=$pfgw_cv_coptflags,pfgwcoptflags="")
PFGW_CHECK_CXXFLAGS([[whether preferred optimizations work in C++]],pfgw_cv_cxxopt,$pfgw_cv_cxxoptflags,pfgwcxxoptflags=$pfgw_cv_cxxoptflags,pfgwcxxoptflags="")
PFGW_CHECK_CFLAGS([[whether suggested warnings work in C]],pfgw_cv_cwarn,$pfgw_cv_cwarnflags,pfgwcwarnflags=$pfgw_cv_cwarnflags,,pfgwcwarnflags="")
PFGW_CHECK_CXXFLAGS([[whether suggested warnings work in C++]],pfgw_cv_cxxwarn,$pfgw_cv_cxxwarnflags,pfgwcxxwarnflags=$pfgw_cv_cxxwarnflags,pfgwcxxwarnflags="")
PFGW_CHECK_CXXFLAGS([[whether to enable extra C warnings]],pfgw_cv_xcwarn,$pfgw_cv_xcwarnflags,pfgwxcwarnflags=$pfgw_cv_xcwarnflags,pfgwxcwarnflags="")
PFGW_CHECK_CXXFLAGS([[whether to enable extra C++ warnings]],pfgw_cv_xcxxwarn,$pfgw_cv_xcxxwarnflags,pfgwxcxxwarnflags=$pfgw_cv_xcxxwarnflags,pfgwxcxxwarnflags="")
])

AC_DEFUN(PFGW_APPLY_FLAGS,[
CFLAGS="$CFLAGS $pfgwcflags $pfgwcoptflags $pfgwcwarnflags $pfgwxcwarnflags"
CXXFLAGS="$CXXFLAGS $pfgwcxxflags $pfgwcxxoptflags $pfgwcxxwarnflags $pfgwxcxxwarnflags"
])
