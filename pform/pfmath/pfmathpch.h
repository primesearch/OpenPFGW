// Precompiled header file for use with the pfmath library
#include "config.h"
#include "pflib.h"
#include "pfmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

template<class T>
inline T abs(const T& t)
{
	return((t>=0)?t:-t);
}
