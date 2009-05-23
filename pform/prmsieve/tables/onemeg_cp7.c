#if !defined(_ONEMEG_CP7_H)
#include "onemeg_cp7.h"
#endif

unsigned char onemeg_cp7[] =
{
#include "onemeg.cp7"
};
unsigned int onemeg_cp7_size()
{
    return sizeof(onemeg_cp7);
}
