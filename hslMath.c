#include "hslMath.h"

/***************************************************/
/*       QUE Functions                             */
/***************************************************/

int hslGetBitResolution(int val)
{
    int i=1;

    while(val>>=1) i++;

    return i;
}

int hslAbs(int val)
{
    if (val>0) return val;
    return -val;
}