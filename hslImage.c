#include "hslType.h"
#include "hslImage.h"

void hslImageCrop8(HSUINT8 *pSrc, int ws, int hs, HSUINT8 *pDst, int xd, int yd, int wd, int hd)
{
    HSUINT16 i,j, x, y;
    HSUINT8 *img;

    img = pSrc + (ws*yd);
    x = xd+wd;  y = yd+hd;
    for (i=yd; i<y; i++)
    {
        img += xd;
        for (j=xd; j<x; j++)
        {
            *pDst++ = *img++;
        }
    }
}

void hslImageLinearScale8(HSUINT8 *pSrc, int ws, int hs, HSUINT8 *pDst, int wd, int hd)
{

}

void hslImageCropScale8(HSUINT8 *pSrc, int ws, int hs, HSUINT8 *pDst, int wd, int hd)
{
    HSUINT16 cropWid, cropHgt;
    HSUINT16 rateWid = ws/wd, rateHgt = hs/hd;

    HSUINT16 i,j; 
    
    cropWid = ws - rateWid * wd;	// 0
    cropHgt = hs - rateHgt * hd;	// 0

    if (cropWid || cropHgt)
    {
        hslImageCrop8(pSrc, ws, hs, pDst, cropWid/2, cropHgt/2, wd, hd);
    }

    for (i=0; i<hd; i++)	// 160
    {
        for (j=0; j<wd; j++)	// 120
        {
            *pDst++ = *pSrc;
            pSrc+= rateWid;		// 320���� Y���� 160���� Y�� ������(�ּҸ� 2�� ������Ű�� �� ������)
        }
        pSrc+= (ws*rateHgt - ws);	// height �� 2ĭ�� �� ������
    }
}

