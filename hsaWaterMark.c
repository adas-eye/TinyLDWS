#include "hslType.h"
#include "hslMath.h"
#include "hslFilter.h"

typedef struct _RECT
{
	HSUINT16 left;
	HSUINT16 right;
	HSUINT16 top;
	HSUINT16 bottom;
} HM_RECT;

extern HSUINT8*  gHmMaskBuf;					 // Mask Image
#define PROFILE_LENGTH 50

HSUINT8 flgSecureCheck = 1;
HSUINT8 flgSecureLock = 0;
int gPrfX[PROFILE_LENGTH], gPrfY[PROFILE_LENGTH];

void linearScale1D(int* src, int srcLen, int* dst, int dstLen);
void bounding(int* pData, int len, int limit);
void measureProfile(HSUINT8* img);


typedef struct
{
 HSUINT32 centerX;
 HSUINT32 centerY;
} SECURE_RECT;

#define SECURE_MARGIN	5

#include "sProfile.h"
#if 0
int checkSecure(HSUINT8* img)
{
	int absDiffX=0, absDiffY=0, i, sumX = 0, sumY = 0;

	measureProfile(img);

	for (i=0;i<PROFILE_LENGTH;i++)
	{
		sumX += sProfile[i][0];
		sumY += sProfile[i][1];
		absDiffX+=hslAbs(sProfile[i][0] - gPrfX[i]);
		absDiffY+=hslAbs(sProfile[i][1] - gPrfY[i]);
	}
	
	return (absDiffX*100/sumX) + (absDiffY*100/sumY);
}
#endif
void hmInitMask(void);
int checkSecure(HSUINT8* src, SECURE_RECT* rect)
{
	int i, j, ret=0;
	unsigned int sum=0, cnt=0;

	for (i=20; i<(rect->centerY-SECURE_MARGIN); i++)
	{
		for (j=20; j<(rect->centerX-SECURE_MARGIN); j++)
		{
			sum += src[i*160+j];
			cnt++;
		}
	}
	
	if (cnt > 0)
		if ( (((sum/cnt)*100)/255) <= 10)	ret+=25;

	sum = cnt = 0;
	for (i=20; i<(rect->centerY-SECURE_MARGIN); i++)
	{
		for (j=(rect->centerX+SECURE_MARGIN); j<140; j++)
		{
			sum += src[i*160+j];
			cnt++;
		}
	}
	
	if (cnt > 0)
		if ( (((sum/cnt)*100)/255) >= 90)	ret+=25;

	sum = cnt = 0;
	for (i=(rect->centerY+SECURE_MARGIN); i<100; i++)
	{
		for (j=20; j<(rect->centerX-SECURE_MARGIN); j++)
		{
			sum += src[i*160+j];
			cnt++;
		}
	}
	if (cnt > 0)
		if ( (((sum/cnt)*100)/255) >= 90)	ret+=25;

	sum = cnt = 0;
	for (i=(rect->centerY+SECURE_MARGIN); i<100; i++)
	{
		for (j=(rect->centerX+SECURE_MARGIN); j<140; j++)
		{
			sum += src[i*160+j];
			cnt++;
		}
	}
	if (cnt > 0)
		if ( (((sum/cnt)*100)/255) <= 10)	ret+=25;

	return ret;
}
#if 1
void gray2Bin(HSUINT8* src)
{
	int i, j;

	for (i=20; i<100; i++)
	{
		for (j=20; j<140; j++)
		{
			if (src[i*160+j] >= 80)    src[i*160+j] = 0xff;
			else						src[i*160+j] = 0x0;
		}
	}
}

void searchEdge(HSUINT8* src, SECURE_RECT* rect)
{
	int i, j;
	unsigned int sum=0, cnt=0;  // 20130723 need a test for momory leak jdy

	for (i=20; i<100; i++)
	{
		for (j=51; j<110; j++)
		{
			if ( (src[i*160+j] - src[i*160+(j-1)]) != 0)	// edge check
			{
				sum += j;
				cnt++;
				break;
			}
		}
	}
	if (cnt > 0)   // 20130415_modify
	{
		rect->centerX = sum/cnt;
		sum = cnt = 0;

		for (j=20; j<140; j++)
		{
			for (i=41; i<80; i++)
			{
				if ( (src[i*160+j] - src[(i-1)*160+j]) != 0)	// edge check
				{
					sum += i;
					cnt++;
					break;
				}
			}
		}
    	   if (cnt > 0)   // 20130415_modify
	       {
			rect->centerY = sum/cnt;
           }
		else
           {
			rect->centerX = rect->centerY = 0;
	}
        }         
	else
	{
		rect->centerX = rect->centerY = 0;
	}
}

void hmSecureLock(HSUINT8* img)
{
	int i,j;
	static HSUINT8 count=230;
	SECURE_RECT rect;
#if 1
	//---------------------------------
	// 상,하,좌,우, 20 pixel 씩 지움
	for (i=0; i<20; i++)
	{
		for (j=0; j<160; j++)
		{
			img[i*160+j] = 0xff;
			img[(100+i)*160+j] = 0xff;
		}
	}

	for (i=20; i<100; i++)
	{
		for (j=0; j<20; j++)
		{
			img[i*160+j] = 0xff;
			img[i*160+(140+j)] = 0xff;
		}
	}
	//---------------------------------
#endif
	gray2Bin(img);
	searchEdge(img, &rect);
	if (count++ && !flgSecureLock)
	{
		if (rect.centerX & rect.centerY)	// 20130415_modify
		{
			if (checkSecure(img, &rect) == 100) flgSecureLock = 1;
		}
	}
	else if (flgSecureCheck)
	{
		flgSecureCheck = 0;
		hmInitMask();
	}
}
#endif

void measureProfile(HSUINT8* img)
{
	int i, j;
	int profileX[160]={0,};
	int profileY[120]={0,};
	HM_RECT profileArea;
	int profileWid, profileHgt;
	HSUINT8 *sobel = gHmMaskBuf;

	hslMaskSobel(img, sobel, 160, 120, 0, 0, 160, 120);

	for (i=0;i<120;i++)
	{
		for (j=0;j<160;j++)
		{
			if ((*sobel) > 128)
			{
				profileX[j]++;
				profileY[i]++;
			}
			sobel++;
		}
	}

	profileArea.left = 0;
	for (i=1;i<160 && profileArea.left==0;i++)
	{
		if (profileX[i]) profileArea.left = i;
	}

	profileArea.right = 0;
	for (i=159;i>0 && profileArea.right==0;i--)
	{
		if (profileX[i-2]) profileArea.right = i-1;
	}

	profileArea.top = 0;
	for (i=1;i<120 && profileArea.top==0;i++)
	{
		if (profileY[i]) profileArea.top = i;
	}

	profileArea.bottom = 0;
	for (i=119;i>0 && profileArea.bottom==0;i--)
	{
		if (profileY[i]) profileArea.bottom = i-1;
	}
	
	profileWid = profileArea.right-profileArea.left;
	profileHgt = profileArea.bottom-profileArea.top;

	linearScale1D(profileX+profileArea.left, profileWid, gPrfX, PROFILE_LENGTH);
	linearScale1D(profileY+profileArea.top, profileHgt, gPrfY, PROFILE_LENGTH);
	bounding(gPrfX, PROFILE_LENGTH, 255);
	bounding(gPrfY, PROFILE_LENGTH, 255);
}

void linearScale1D(int* src, int srcLen, int* dst, int dstLen)
{
	float ratio = (float)srcLen/(float)dstLen;
	float stepRatio, val;
	int step, i;

	for (i=0;i<dstLen; i++)
	{
		step = (int)((float)i*ratio);
		stepRatio = (float)i*ratio - (float)step;;

		val = src[step]*(1-stepRatio) + src[step+1]*stepRatio;
		dst[i] = (int)val;
	}
}

void bounding(int* pData, int len, int limit)
{
	int max=0, i;

	for (i=0;i<len;i++)
	{
		if (max<pData[i]) max = pData[i];
	}

	for (i=0;i<len && max;i++)
	{
		pData[i] = pData[i]*limit/max;
	}
}

