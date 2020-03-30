#include "hslType.h"
#include "hslFilter.h"
#include "hslMath.h"

void hslMask3x3(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx)
{
    HSUINT16 i,j;
    HSUINT8 *ptrIn, *ptrOut, *ptr;
    HSINT16 val;
    HSINT16 divide = mtrx[0]+mtrx[1]+mtrx[2]+mtrx[3]+mtrx[4]+mtrx[5]+mtrx[6]+mtrx[7]+mtrx[8];

    if (divide<2) divide = 1;

    for (i=1;i<validHgt-1;i++)
    {
        ptrIn  = in + wid*(offsetY+i);
        ptrOut = out + validWid*i;
        for (j=1;j<validWid-1;j++)
        {
            ptr = ptrIn + j + offsetX;
            val = *(ptr-wid-1) * mtrx[0] + *(ptr-wid) * mtrx[1] + *(ptr-wid+1) * mtrx[2]
                + *(ptr-1)     * mtrx[3] + *(ptr)     * mtrx[4] + *(ptr+1)     * mtrx[5]
                + *(ptr+wid-1) * mtrx[6] + *(ptr+wid) * mtrx[7] + *(ptr+wid+1) * mtrx[8];
            *(ptrOut+j) = (HSUINT8)hslAbs(val/divide);
        }
        ptrOut[0] = ptrOut[1];
        ptrOut[validWid-1] = ptrOut[validWid-2];
    }
    for (i=0;i<validWid;i++) 
    {
        out[i] = out[i+validWid];
        out[validHgt*validWid-validWid+i] = out[validHgt*validWid-validWid*2+i];
    }
}

void hslMask3x1(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx)
{
    HSUINT16 i,j;
    HSUINT8 *ptrIn, *ptrOut, *ptr;
    HSINT16 val;
    HSINT16 divide = mtrx[0]+mtrx[1]+mtrx[2];

    if (divide<2) divide = 1;

    for (i=0;i<validHgt;i++)
    {
        ptrIn  = in + wid*(offsetY+i);
        ptrOut = out + validWid*i;
        for (j=1;j<validWid-1;j++)
        {
            ptr = ptrIn + j + offsetX;
            val = *(ptr-wid-1) * mtrx[0] + *(ptr-wid) * mtrx[1] + *(ptr-wid+1) * mtrx[2];
            *(ptrOut+j) = (HSUINT8)hslAbs(val/divide);
        }
        ptrOut[0] = ptrOut[1];
        ptrOut[validWid-2] = ptrOut[validWid-1];
    }
}

void hslMask9x1(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx)
{
    HSUINT16 i,j;
    HSUINT8 *ptrIn, *ptrOut, *ptr;
    HSINT32 val;
    HSINT16 divide = mtrx[0]+mtrx[1]+mtrx[2]+mtrx[3]+mtrx[4]+mtrx[5]+mtrx[6]+mtrx[7]+mtrx[8];

    if (divide<2) divide = 1;

    for (i=0;i<validHgt;i++)
    {
        ptrIn  = in + wid*(offsetY+i);
        ptrOut = out + validWid*i;
        for (j=4;j<validWid-4;j++)
        {
            ptr = ptrIn + j + offsetX;
            val = (HSUINT32)(*(ptr-4) * mtrx[0]) + *(ptr-3) * mtrx[1] + *(ptr-2) * mtrx[2] 
                + *(ptr-1) * mtrx[3] + *(ptr)   * mtrx[4] + *(ptr+1) * mtrx[5]
                + *(ptr+2) * mtrx[6] + *(ptr+3) * mtrx[7] + *(ptr+4) * mtrx[8];
            *(ptrOut+j) = (HSUINT8)hslAbs(val/divide);
        }
        ptrOut[0] = ptrOut[1] = ptrOut[2]= ptrOut[3] = ptrOut[4];
        ptrOut[validWid-1] = ptrOut[validWid-2] = ptrOut[validWid-3] = ptrOut[validWid-4] = ptrOut[validWid-5];
    }
}

void hslMaskSobel(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt)
{
    HSUINT16 i,j;
    HSUINT8 *ptrIn, *ptrOut, *ptr;
    HSINT32 hval, vval, val;

    for (i=1;i<validHgt-1;i++)
    {
        ptrIn  = in + wid*(offsetY+i);
        ptrOut = out + validWid*i;
        for (j=1;j<validWid-1;j++)
        {
            ptr = ptrIn + j + offsetX;
            hval = *(ptr-wid-1) * (-1) + *(ptr-wid) * (-2) + *(ptr-wid+1) * (-1)
                 + *(ptr+wid-1) * (+1) + *(ptr+wid) * (+2) + *(ptr+wid+1) * (+1);

            vval = *(ptr-wid-1) * (-1) + *(ptr-wid+1) * (+1)
                 + *(ptr-1)     * (-2) + *(ptr+1)     * (+2)
                 + *(ptr+wid-1) * (-1) + *(ptr+wid+1) * (+1);

            *(ptrOut+j) = (HSUINT8)hslAbs(hval/2)+(HSUINT8)hslAbs(vval/2);
        }
        ptrOut[0] = ptrOut[1];
        ptrOut[validWid-1] = ptrOut[validWid-2];
    }
    for (i=0;i<validWid;i++) 
    {
        out[i] = out[i+validWid];
        out[validHgt*validWid-validWid+i] = out[validHgt*validWid-validWid*2+i];
    }
}

