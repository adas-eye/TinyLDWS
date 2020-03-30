// Adas_LDWS.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
/*
 *******************************************************************************
 *	(C) Copyright HiveMotion Ltd. 2012 All right reserved.
 *	  Confidential Information
 *	
 *	All parts of the HiveMotion Program Source are protected by copyright law 
 *	and all rights are reserved. 
 *	This documentation may not, in whole or in part, be copied, photocopied, 
 *	reproduced, translated, or reduced to any electronic medium or machine 
 *	readable form without prior consent, in writing, from HiveMotion. 
 *******************************************************************************
 *	Project 	: ADAS LDWS
 *	Description : LDWS Function.
 *
 *******************************************************************************
 *	Version control information
 *	2012.02.29 - create version
 *
 *******************************************************************************
 *	Modification History
 *
 *******************************************************************************
 */

#define BOUNDARY_ANGLE		10		 /* Unit is 1 degree */   // org30 jdy
#define MIN_ANGLE			(BOUNDARY_ANGLE)
#define MAX_ANGLE			(180-BOUNDARY_ANGLE)
#define LDWS_STATUS_CALIBRATION 2
/*******************************************************************************
 * INCLUDE FILES														   
 ******************************************************************************/
#include "hslType.h"
#include "hscAttachLdws.h"
#include "hsaLdws.h"
#include "hscSetting.h"
#include "hslGraphics.h"
#include "hslMath.h"
#include "hslFifo.h"
#include "hsaWaterMark.h"

//-------------------------------------------------------------------------
// 0 ~ PI 각도에 해당하는 sin, cos 함수의 값을 룩업테이블에서 읽어옮
//-------------------------------------------------------------------------
#include "hsaLutSinCos180.h"

//-------------------------------------------------------------------------
// Lib 의 버전 확인
//-------------------------------------------------------------------------
#ifdef LIB_CORELOGIC_LOCK
HSUINT8 *hmVersion={"HM_VERSION_CORELOGIC_LOCK:2013_11_18"};
#else
HSUINT8 *hmVersion={"HM_VERSION_CORELOGIC_UNLOCK:2013_11_18"};
#endif

#define ANGLE_COUNT			180
/*******************************************************************************
 * DEFINED FUNCTIONS											  
 ******************************************************************************/
#define HTL_NUM				8		/* The count of Lines found after doing Hough Transform */
#define LT_SIN(x) lutSinCos[x][0]
#define LT_COS(x) lutSinCos[x][1]

void hsaHoughtransform_f(HSUINT8 *greyBuf, HS_STRUCT_POLAR_COORD *ml, hsaFpDecideRightLane_CALLBACK rightLane)
{
	int i,j;
	int m, n;
	HS_STRUCT_POLAR_COORD htTmp, ht;

	for (i=0;i<HTL_NUM;i++) 
	{
		ml[i].cnt = 0;
		ml[i].ang = 0;
		ml[i].rho = 0;
	}

	greyBuf+=gHmLdwsSet.heightGap * gHmLdwsSet.width;

	for( j = gHmLdwsSet.heightGap ; j < gHmLdwsSet.heightValid ; j++ )
	{
		greyBuf+=gHmLdwsSet.widthOffset;
		for( i = gHmLdwsSet.widthOffset; i < gHmLdwsSet.width-gHmLdwsSet.widthGap; i++ )
		{
			if( *greyBuf++)
			{
				for( n = MIN_ANGLE; n < MAX_ANGLE; n+=(180/ANGLE_COUNT))
				{
					m = (LT_SIN(n) * i + LT_COS(n) * j + 512)/1024;
					if (m>0 && m<gHmLdwsSet.width) gHmHoughBuf[m*180+n]++;
				}
			}
		}
		greyBuf+=gHmLdwsSet.widthGap;
	}

	for( m = gHmLdwsSet.minRho ; m < gHmLdwsSet.maxRho ; m++ )
	{
		for( n = MIN_ANGLE ; n < 90 ; n+=(180/ANGLE_COUNT) )
		{
			ht.cnt = gHmHoughBuf[m*180+n];
			ht.rho = m;
			ht.ang = n;
			if (ht.cnt < gHmLdwsSet.pointLimit) continue;
			
			for (i=0;i<HTL_NUM/2;i++)
			{
				if( ht.cnt > ml[i].cnt)
				{
					if ((*rightLane)(ht.rho, ht.ang))
					{
						htTmp.cnt = ml[i].cnt;
						htTmp.rho = ml[i].rho;
						htTmp.ang = ml[i].ang;
						
						ml[i].cnt = ht.cnt;
						ml[i].rho = ht.rho;
						ml[i].ang = ht.ang;

						ht.cnt = htTmp.cnt;
						ht.rho = htTmp.rho;
						ht.ang = htTmp.ang;
					}
				}
			}
		}

		for(; n < MAX_ANGLE ; n+=(180/ANGLE_COUNT) )
		{
			ht.cnt = gHmHoughBuf[m*180+n];
			ht.rho = m;
			ht.ang = n;
			if (ht.cnt < gHmLdwsSet.pointLimit) continue;

			for (i=HTL_NUM/2;i<HTL_NUM;i++)
			{
				if( ht.cnt > ml[i].cnt) 
				{
					if ((*rightLane)(ht.rho, ht.ang))
					{
						htTmp.cnt = ml[i].cnt;
						htTmp.rho = ml[i].rho;
						htTmp.ang = ml[i].ang;
						
						ml[i].cnt = ht.cnt;
						ml[i].rho = ht.rho;
						ml[i].ang = ht.ang;

						ht.cnt = htTmp.cnt;
						ht.rho = htTmp.rho;
						ht.ang = htTmp.ang;
					}
				}
			}
		}
	}
}

void hsaDrawLineRhoThetaY8(HSUINT8 *ptr, int rho, int theta, HSUINT8 c)
{
	HSINT16 x1, y1, x2, y2;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);
	hslDrawLine8(ptr, gHmLdwsSet.width, gHmLdwsSet.heightValid, x1, y1+gHmLdwsSet.heightOffset, x2, y2+gHmLdwsSet.heightOffset, c);
}

void hsaDrawLineRhoThetaY8_test(HSUINT8 *ptr, int rho, int theta, HSUINT8 c)
{
	HSINT16 x1, y1, x2, y2;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = 8;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);
	hslDrawLine8(ptr, gHmLdwsSet.width, 8, x1, y1, x2, y2, c);
}

void hsaDrawLineRhoThetaY8Scale(int w, int h, HSUINT8 *ptr, int rho, int theta, HSUINT8 c)
{
	int scaleX, scaleY;
	HSINT16 x1, y1, x2, y2;

	scaleX = (w*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
	scaleY = (h*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);
	hslDrawLine8(ptr, w, h, x1*scaleX/1024, (y1+gHmLdwsSet.heightOffset)*scaleY/1024, x2*scaleX/1024, (y2+gHmLdwsSet.heightOffset)*scaleY/1024, c);
}

void hsaDrawLineRhoThetaY8Scale_(int w, int h, HSUINT8 *ptr, int rho, int theta, HSUINT8 c)
{
	int scaleX, scaleY;
	HSINT16 x1, y1, x2, y2;
	HSINT16 sx1, sy1, sx2, sy2;

	scaleX = (w*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
	scaleY = (h*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);

	sx1 = x1*scaleX/1024;	sx2 = x2*scaleX/1024;
	sy1 = (y1+gHmLdwsSet.heightOffset)*scaleY/1024;
	sy2 = (y2+gHmLdwsSet.heightOffset)*scaleY/1024;
	hslDrawLine8(ptr, w, h, sx1-1, sy1, sx2-1, sy2, c);
	hslDrawLine8(ptr, w, h, sx1, sy1, sx2, sy2, c);
	hslDrawLine8(ptr, w, h, sx1+1, sy1, sx2+1, sy2, c);

}

void hsaDrawLineRhoThetaY16(HSUINT16 *ptr, int rho, int theta, HSUINT16 c)
{
	HSINT16 x1, y1, x2, y2;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);
	hslDrawLine16(ptr, gHmLdwsSet.width, gHmLdwsSet.heightValid, x1, y1+gHmLdwsSet.heightOffset, x2, y2+gHmLdwsSet.heightOffset, c);
}

void hsaDrawLineRhoThetaY16Scale(int w, int h, HSUINT16 *ptr, int rho, int theta, HSUINT16 c)
{
	int scaleX, scaleY;
	HSINT16 x1, y1, x2, y2;

	scaleX = (w*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
	scaleY = (h*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);
	hslDrawLine16(ptr, w, h, x1*scaleX/1024, (y1+gHmLdwsSet.heightOffset)*scaleY/1024, x2*scaleX/1024, (y2+gHmLdwsSet.heightOffset)*scaleY/1024, c);
}

void hsaDrawLineRhoThetaY16Scale_(int w, int h, HSUINT16 *ptr, int rho, int theta, HSUINT16 c)
{
	int scaleX, scaleY;
	HSINT16 x1, y1, x2, y2;
	HSINT16 sx1, sy1, sx2, sy2;

	scaleX = (w*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
	scaleY = (h*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;

	y1 = 0;
	x1 = hsaRhoTheta2XY(rho, theta, &y1);
	y2 = gHmLdwsSet.heightValid-1;
	x2 = hsaRhoTheta2XY(rho, theta, &y2);

	sx1 = x1*scaleX/1024;	sx2 = x2*scaleX/1024;
	sy1 = (y1+gHmLdwsSet.heightOffset)*scaleY/1024;
	sy2 = (y2+gHmLdwsSet.heightOffset)*scaleY/1024;
	hslDrawLine16(ptr, w, h, sx1-1, sy1, sx2-1, sy2, c);
	hslDrawLine16(ptr, w, h, sx1, sy1, sx2, sy2, c);
	hslDrawLine16(ptr, w, h, sx1+1, sy1, sx2+1, sy2, c);
}

HSINT16 hsaRhoTheta2XY(int rho, int ang, HSINT16 *y)
{
	HSINT16 x;

	x = (rho*1024 - (*y)*LT_COS(ang) + LT_SIN(ang)/2) / LT_SIN(ang);
	
	if (x<0)
	{
		x = 0;
		*y = (rho * 1024 + LT_COS(ang)/2) / LT_COS(ang);	
	}
	else if (x>=gHmLdwsSet.width)
	{
		x = gHmLdwsSet.width-1;
		*y = (rho * 1024 - x * LT_SIN(ang) + LT_COS(ang)/2) / LT_COS(ang);	
	}

	return x;
}

void hsaAvgLane(HS_STRUCT_POLAR_COORD *mostLine, HS_STRUCT_POLAR_COORD *avgLine)
{
	int i;
	HSUINT32 diffSum[2]={0,0};
	HSINT32 diffRho=0, diffAng=0;
	
	avgLine[0].cnt = 0;  avgLine[1].cnt = 0;
	avgLine[0].ang = 0;  avgLine[1].ang = 0;
	avgLine[0].rho = 0;  avgLine[1].rho = 0;
	
	for (i=0;i<HTL_NUM/2;i++)
	{
		if (mostLine[i].cnt)
		{
				avgLine[0].rho += mostLine[i].rho;
				avgLine[0].ang += mostLine[i].ang;
				avgLine[0].cnt ++;
		}
	}
	
	for (i=HTL_NUM/2;i<HTL_NUM;i++)
	{
		if (mostLine[i].cnt)
		{
				avgLine[1].rho += mostLine[i].rho;
				avgLine[1].ang += mostLine[i].ang;
				avgLine[1].cnt ++;
		}
	}

	if (avgLine[0].cnt)
	{
		avgLine[0].rho = avgLine[0].rho/avgLine[0].cnt;
		avgLine[0].ang = avgLine[0].ang/avgLine[0].cnt;
	}

	if (avgLine[1].cnt)
	{
		avgLine[1].rho = avgLine[1].rho/avgLine[1].cnt;
		avgLine[1].ang = avgLine[1].ang/avgLine[1].cnt;
	}

	for (i=0;i<HTL_NUM/2 && avgLine[0].cnt;i++)
	{
		if (mostLine[i].cnt)
		{
			diffRho = avgLine[0].rho - mostLine[i].rho;
			diffAng = avgLine[0].ang - mostLine[i].ang;
			diffSum[0] += (diffRho*diffRho + diffAng*diffAng);
		}
	}
	
	for (i=HTL_NUM/2;i<HTL_NUM && avgLine[1].cnt;i++)
	{
		if (mostLine[i].cnt)
		{
			diffRho = avgLine[1].rho - mostLine[i].rho;
			diffAng = avgLine[1].ang - mostLine[i].ang;
			   diffSum[1] += (diffRho*diffRho + diffAng*diffAng);
		}
	}

	if (diffSum[0]>DIFFSUM_MARGIN) 
	{
			avgLine[0].ang = 0; 
			avgLine[0].rho = 0;
			avgLine[0].cnt = 180;
	}
	else
	{
			avgLine[0].rho = mostLine[0].rho;
			avgLine[0].ang = mostLine[0].ang;
	}
	if (diffSum[1]>DIFFSUM_MARGIN)
	{
			avgLine[1].ang = 0;
			avgLine[1].rho = 0;
			avgLine[1].cnt = 180;
	}
	else
	{
			avgLine[1].rho = mostLine[HTL_NUM/2].rho;
			avgLine[1].ang = mostLine[HTL_NUM/2].ang;
	}
}

void hsaGetRhoFromRule(HSINT16 ang, HSINT16 *rho)
{
	*rho = (gHmLdwsSet.centerPoint * LT_SIN(ang)+512)/1024;
}

int hsaIsRightLane(HSINT16 rho, HSINT16 ang)
{
	HSINT16 ruleRho;

	if (ang<(BOUNDARY_ANGLE) || ang>(180-BOUNDARY_ANGLE))
	{
		return 0;
	}

	  hsaGetRhoFromRule(ang, &ruleRho);
	  
	if ((rho > (ruleRho+gHmLdwsSet.marginRho)) || (rho < (ruleRho-gHmLdwsSet.marginRho)))
	{
		return 0;
		}
	else
	{
		return 1;
	}

}
// add kys
void hsaLengthFilter_CAL(HSUINT8* ldwsBuf, HSUINT16 wid, HSUINT16 hei)
{
	HSUINT16 i, j, sum=0;

	for (i=5; i<hei-5; i++)
	{
		for (j=5; j<wid-5; j++)
		{
			if (ldwsBuf[i*wid+j])
			{
				sum = ldwsBuf[(i-1)*wid+(j-2)] + ldwsBuf[(i-1)*wid+(j-1)] + ldwsBuf[(i-1)*wid+(j  )] + ldwsBuf[(i-1)*wid+(j+1)] + ldwsBuf[(i-1)*wid+(j+2)] +
						ldwsBuf[(i+1)*wid+(j-2)] + ldwsBuf[(i+1)*wid+(j-1)] + ldwsBuf[(i+1)*wid+(j	)] + ldwsBuf[(i+1)*wid+(j+1)] + ldwsBuf[(i+1)*wid+(j+2)];
				if (!sum)
					ldwsBuf[i*wid+j]=0;
				else
				{
					sum = ldwsBuf[(i-2)*wid+(j-4)] + ldwsBuf[(i-2)*wid+(j-3)] + ldwsBuf[(i-2)*wid+(j-2)] + ldwsBuf[(i-2)*wid+(j-1)] + ldwsBuf[(i-2)*wid+(j+1)] + ldwsBuf[(i-2)*wid+(j+2)] + ldwsBuf[(i-2)*wid+(j+3)] + ldwsBuf[(i-2)*wid+(j+4)] +
							ldwsBuf[(i+2)*wid+(j-4)] + ldwsBuf[(i+2)*wid+(j-3)] + ldwsBuf[(i+2)*wid+(j-2)] + ldwsBuf[(i+2)*wid+(j-1)] + ldwsBuf[(i+2)*wid+(j+1)] + ldwsBuf[(i+2)*wid+(j+2)] + ldwsBuf[(i+2)*wid+(j+3)] + ldwsBuf[(i+2)*wid+(j+4)];					
					if (sum < 2)
						ldwsBuf[i*wid+j]=0;
				}
			}
		}
	}
}

void hsaLengthFilter(HSUINT8* ldwsBuf, HSUINT16 wid, HSUINT16 hei)
{
    HSUINT16 i, j,k,l, sum=0;
    HSUINT8 filter_wid = 20;
    HSUINT8 width_cnt;
	HSUINT16 verttical_cnt[160] = {0,};
    for (i=5; i<hei-5; i++)
    {
        for (j=5; j < gHmLdwsSet.centerPoint - 5; j++)
        //for (j=5; j <= (wid / 2) + 5; j++)
        {
            if (ldwsBuf[i*wid+j])
            {
                verttical_cnt[j]++; //add jdy

                if(j >= filter_wid && j <= wid-1-filter_wid)
                {
                    width_cnt = 0;
                    for(k = j-filter_wid;k < j ;k++)
                    {
                        if (ldwsBuf[i*wid+k]) width_cnt++;
                    }
                    if(width_cnt > 5)
                    {
                        for(k = j-filter_wid;k < j ;k++)
                        {
                            ldwsBuf[i*wid+k] = 0;
                        }
                    }
                }
                sum = ldwsBuf[(i-1)*wid+(j	)] + ldwsBuf[(i-1)*wid+(j+1)] + ldwsBuf[(i-1)*wid+(j+2)] +
                        ldwsBuf[(i+1)*wid+(j-2)] + ldwsBuf[(i+1)*wid+(j-1)] + ldwsBuf[(i+1)*wid+(j	)];
                        
				if (!sum)
					ldwsBuf[i*wid+j]=0;
				else
				{
					sum = ldwsBuf[(i-2)*wid+(j+1)] + ldwsBuf[(i-2)*wid+(j+2)] + ldwsBuf[(i-2)*wid+(j+3)] + ldwsBuf[(i-2)*wid+(j+4)] +
							ldwsBuf[(i+2)*wid+(j-4)] + ldwsBuf[(i+2)*wid+(j-3)] + ldwsBuf[(i+2)*wid+(j-2)] + ldwsBuf[(i+2)*wid+(j-1)];					

					if (sum < 2)
						ldwsBuf[i*wid+j]=0;
				}
				
				if(verttical_cnt[j] >= 4) // add jdy
				{
					verttical_cnt[j] = 0;
					for(l = i; l >= i - 4;l--)
					{
						ldwsBuf[l * wid + j] = 0;
					}
				}
				//}
				
			}
			else
			{
					  verttical_cnt[j] = 0; //add jdy
			}
		}
        for (j=gHmLdwsSet.centerPoint+5; j< wid - 5; j++)
        {
            if (ldwsBuf[i*wid+j])
            {
                verttical_cnt[j]++; //add jdy
                if(j >= filter_wid && j <= wid-1-filter_wid)
                {
                    width_cnt = 0;
                    for(k = j-filter_wid;k < j;k++)
                    {
                        if (ldwsBuf[i*wid+k]) width_cnt++;
                    }
                    if(width_cnt > 5)
                    {
                        for(k = j-filter_wid;k < j;k++)
                        {
                            ldwsBuf[i*wid+k] = 0;
                        }
                    }
                }
				sum = ldwsBuf[(i-1)*wid+(j	)] + ldwsBuf[(i-1)*wid+(j-1)] + ldwsBuf[(i-1)*wid+(j-2)] +
						ldwsBuf[(i+1)*wid+(j+2)] + ldwsBuf[(i+1)*wid+(j+1)] + ldwsBuf[(i+1)*wid+(j	)];

				if (!sum)
					ldwsBuf[i*wid+j]=0;
				else
				{
					sum = ldwsBuf[(i-2)*wid+(j-1)] + ldwsBuf[(i-2)*wid+(j-2)] + ldwsBuf[(i-2)*wid+(j-3)] + ldwsBuf[(i-2)*wid+(j-4)] +
							ldwsBuf[(i+2)*wid+(j+4)] + ldwsBuf[(i+2)*wid+(j+3)] + ldwsBuf[(i+2)*wid+(j+2)] + ldwsBuf[(i+2)*wid+(j+1)];					

					if (sum < 2)
						ldwsBuf[i*wid+j]=0;
				}
				if(verttical_cnt[j] >= 4) // add jdy
				{
                    verttical_cnt[j] = 0;					
				    for(l = i; l >= i - 4;l--)
					{
						ldwsBuf[l * wid + j] = 0;
					}
				}		
					
				//}
			}
			else
			{
				verttical_cnt[j] = 0; //add jdy
			}
		}
		// add jdy
		
	}

	//for (j=(wid / 2) - 5; j < (wid / 2) + 5; j++)
	for (j=gHmLdwsSet.centerPoint - 5; j < gHmLdwsSet.centerPoint + 5; j++)     // 20130529 hold     
	{
		for (i=5; i<hei-5; i++)		
		{
			if (ldwsBuf[i*wid+j])
			{
				   verttical_cnt[j]++; //add jdy
				   if(verttical_cnt[j] >= 4) // add jdy
				{
					verttical_cnt[j] = 0;				
					for(l = i; l >= i - 4;l--)
					{
						ldwsBuf[l * wid + j] = 0;
					}
				}	
			}
			else
			{
					  verttical_cnt[j] = 0; //add jdy
			}
		}
	}
}


void hsaLengthDetect(HSUINT8 *pYBuffer, HSUINT8 *pSetData, HSUINT8 flag)
{
	int i, j, k, diff;
	int upEdge=0, validCnt=0;
	int step = (gHmLdwsSet.laneMax-gHmLdwsSet.laneMin)*1024/(gHmLdwsSet.height-gHmLdwsSet.heightOffset);
	int maxwide, minwide, maxDiff=0;
	int diffHisto[256]={0,}, sum, highLimit, max;
	HSUINT8 *mask = gHmMaskBuf;
	HSUINT8 x1, x2, errX1[10], errX2[10], errY[10], errCnt=0;
	float g1, g2;
	HSUINT8 turn_flag = 0, turn_cnt = 0;
	/*/
	if(flag) //jdy mask 
	{
		//if((gHmLdwsSet.heightValid - gHmLdwsSet.heightGap)/2 < i)
		g1 = (float)((gHmLdwsSet.width/2-2) -gHmLdwsSet.warningX[0]) / (float)gHmLdwsSet.centerPoint;
		g2 = (float)(gHmLdwsSet.warningX[1] -(gHmLdwsSet.width/2+2)) / (float)gHmLdwsSet.centerPoint;
	}
	//*/
	max = 0;
	for (i=gHmLdwsSet.heightGap, k=0;i<gHmLdwsSet.heightValid;i++,k++)
	{
		//maxwide=gHmLdwsSet.laneMin+((step*i)>>10); //del jdy
		maxwide=gHmLdwsSet.laneMax+((step*i)>>10); //add jdy
		minwide=maxwide>>2;

		
		for (j=gHmLdwsSet.widthOffset+1;j<gHmLdwsSet.width-1-gHmLdwsSet.widthGap;j++)
		{

			pSetData[i*gHmLdwsSet.width+j] = 0;
			if (!mask[i*gHmLdwsSet.width+j-1]) continue;

			x1 = (pYBuffer[i*gHmLdwsSet.width+j-1]>>1) + (pYBuffer[i*gHmLdwsSet.width+j]>>1);
			x2 = (pYBuffer[i*gHmLdwsSet.width+j+1]>>1) + (pYBuffer[i*gHmLdwsSet.width+j]>>1);
//			  diff = pYBuffer[i*gHmLdwsSet.width+j+1] - pYBuffer[i*gHmLdwsSet.width+j-1];

			diff = x2-x1;

			if (diff>0) 
			{
				diffHisto[diff]++;
				max++;
			}

			
			if (diff>gHmLdwsSet.laneYGap) 
			{
				upEdge = j;
			}
			else if (diff<-gHmLdwsSet.laneYGap)
			{
				if (upEdge) 
				{	
					validCnt = j - upEdge;
					upEdge = 0;

					if (validCnt<maxwide) 
					{
						pSetData[i*gHmLdwsSet.width+j] = pYBuffer[i*gHmLdwsSet.width+j];
//						  pSetData[i*gHmLdwsSet.width+j-1] = pYBuffer[i*gHmLdwsSet.width+j-1];
						//j+=maxwide;
					}

				}
				validCnt = 0;
			}
		}

		pSetData[i*gHmLdwsSet.width+gHmLdwsSet.widthOffset] = pSetData[i*gHmLdwsSet.width+gHmLdwsSet.width-gHmLdwsSet.widthGap] = 0;
	}

	//20130529

	if(gHmLdwsSet.stsLdws == LDWS_STATUS_CALIBRATION) //20130527 for Calibration and Filter center point
    {
		hsaLengthFilter_CAL(pSetData, gHmLdwsSet.width, gHmLdwsSet.height);  //add kys
	}
	else
	{
		hsaLengthFilter(pSetData, gHmLdwsSet.width, gHmLdwsSet.height);  //add kys
	}

	//add jdy 기존 위치 정보를 이용하여, 차선이 점선일 경우 차선을 못 찾는 경우를 줄이기 위함.
	for(i = 0; i < 2; i++)
	{
		if(gHmLdwsSet.result_Ang[i])
		{
			//hsaDrawLineRhoThetaY8Scale_test(gHmLdwsSet.width, gHmLdwsSet.height, pSetData, gHmLdwsSet.result_Rho[i], gHmLdwsSet.result_Ang[i], 255);
			//hsaDrawLineRhoThetaY8Scale(gHmLdwsSet.width, gHmLdwsSet.height, pSetData, gHmLdwsSet.result_Rho[i], gHmLdwsSet.result_Ang[i], 255);
			hsaDrawLineRhoThetaY8_test(pSetData, gHmLdwsSet.result_Rho[i], gHmLdwsSet.result_Ang[i], 255);
		}
	}
	
	
}

#define ARROW_FREEZE  10
#define STATE_NORMAL  0
#define STATE_WARNING 1
#define STATE_ERROR   2
#define STATE_NONE	  3
#define STATE_ARROW   4
#define STATE_LIGHT   5

#define CENTER_MARGIN 3
HSUINT8 hsaDecideTwoLane(HS_STRUCT_POLAR_COORD *mostLine, HS_STRUCT_POLAR_COORD *avgLine)
{
	HSINT16 xLoc[2]={0,0}, yLoc;
	HSUINT8 ret=RET_WARNING_NONE;

	static int errFreeze[2]={0,0};
	static int /*flgReset[2]={1,1}, */flgWarning[2]={0,0}, pre_xLoc[2]={255,255};
	static HSINT16 ldwsState[2] = {STATE_NORMAL, STATE_NORMAL};
//	  static HSINT16 pre_dx[2]={0,0};
	HSUINT16 lane_distance = gHmLdwsSet.warningX[1] - gHmLdwsSet.warningX[0];
	static HS_STRUCT_QUE qDx[2]={0,};

	HSINT16 centerDiff;						// add kys
	HSUINT8 notLane;							// add kys
	static HSUINT8 pre_xLocCenter[2]={255,255}; //	add kys
	HSINT16 x1, y1, x2, y2;									 // add kys
	
	HSINT16 i,j,k, dx[2]={0,0};
	
	HSINT16 L_wave, R_wave;
	static HS_STRUCT_QUE pre_dx[2]={0,};
	HSUINT8 qIdx;
	static HSUINT8 pre_dxInit=1, warningFlg[2]={1,1};

	if (pre_dxInit)  // add kys
	{
		hslInitQ(&pre_dx[0], 4);
		hslInitQ(&pre_dx[1], 4);
		pre_dxInit=0;
	}
	
	hsaAvgLane(mostLine, avgLine);
	
	// add jdy
	L_wave	=  R_wave = 0;
	
	for (i=0;i<2;i++)
	{
		   // add jdy
		   if(i == 0 && qDx[i].qNum >= 4)
		   {
			   for(k = 1; k < qDx[i].qNum; k++)
			{
				L_wave += hslAbs(qDx[i].que[k-1] - qDx[i].que[k]);
			   }
		   }
		if(i == 1 && qDx[i].qNum >= 4)
		   {
			   for(k = 1; k < qDx[i].qNum; k++)
			{
				R_wave += hslAbs(qDx[i].que[k-1] - qDx[i].que[k]);
			   }
		   }

		
		if (pre_xLoc[i] == 255)
		{
			hslInitQ(&qDx[i], QMAX);
		}
		// Decide State
        if (avgLine[i].ang)// && !errFreeze[i])           // Angle and Freeze
		{
            yLoc = gHmLdwsSet.heightValid;
            xLoc[i] = hsaRhoTheta2XY(avgLine[i].rho, avgLine[i].ang, &yLoc);
			
    		hmRhoTheta2XY(avgLine[i].rho, avgLine[i].ang,160,120, &x1, &y1, &x2, &y2);	//add kys
    		if (x1 > gHmLdwsSet.centerPoint)	centerDiff = x1-gHmLdwsSet.centerPoint;    //add kys
    		else								centerDiff = gHmLdwsSet.centerPoint-x1;			  //add kys
		
            if(xLoc[i] < 255 && pre_xLoc[i] < 255)  //add jdy
            {
                dx[i] = xLoc[i] - pre_xLoc[i];
                hslPushQ(&pre_dx[i], dx[i]); // add kys
            }	  
            else
            {
                dx[i] = 0;
            }

			pre_xLoc[i] = xLoc[i];
			pre_xLocCenter[i] = centerDiff; //add kys
			switch (i)
			{
			case 0:
				if (xLoc[0] < gHmLdwsSet.warningX[0])	   // X Location
				{
					ldwsState[0] = STATE_NORMAL;
					// add jdy : 위치 저장 하여, 추후 hough Trangform하기 전에 사용 하기 위함.
					gHmLdwsSet.result_Ang[i] = avgLine[i].ang;
					gHmLdwsSet.result_Rho[i] = avgLine[i].rho;
			  
				}
				else if (xLoc[0] > (gHmLdwsSet.centerPoint+CENTER_MARGIN))
				{
					ldwsState[0] = STATE_LIGHT; 
				}
				else if (dx[0]>gHmLdwsSet.errorXMargin || avgLine[0].cnt==180)
				{
					ldwsState[0] = STATE_ARROW;
					dx[0] = 0;
				}
                else if (qDx[0].qSum > 3 && dx[0] != 0 && qDx[0].qNum >= 3) //add jdy
				{
					ldwsState[0] = STATE_WARNING;
				}
				
		  if((qDx[0].qSum > 1 &&  qDx[1].qSum >= 0)  && (hslAbs((int)gHmLdwsSet.warningX[0] -(int)xLoc[0]) <= 5) )
                {
		      if(dx[0] > 0  && dx[0] < gHmLdwsSet.errorXMargin && qDx[0].qNum >= 3)
                        {
    		          if( gHmLdwsSet.warningAngle[0] - 6 <= avgLine[i].ang)//  && (gHmLdwsSet.warningX[0] + 5) >= xLoc[0]) //add 20130527 jdy del
                            ldwsState[0] = STATE_WARNING;
                        
                    }
                } 
                if(xLoc[0] > gHmLdwsSet.warningX[0] + 10)
                {
                    ldwsState[0] = STATE_ARROW;
                    dx[0] = 0;
                }
                if( gHmLdwsSet.warningAngle[0] + 10 <= avgLine[i].ang )
                {
                    ldwsState[0] = STATE_ARROW;
                    dx[0] = 0;
                }
                break;
			case 1:
				if (xLoc[1] > gHmLdwsSet.warningX[1])	   // X Location
				{
					ldwsState[1] = STATE_NORMAL;
					// add jdy : 위치 저장 하여, 추후 hough Trangform하기 전에 사용 하기 위함.
					gHmLdwsSet.result_Ang[i] = avgLine[i].ang;
					gHmLdwsSet.result_Rho[i] = avgLine[i].rho;					  
				}
				else if (xLoc[1] < (gHmLdwsSet.centerPoint-CENTER_MARGIN))
				{ 
					ldwsState[1] = STATE_LIGHT;
				}
				else if (dx[1]<-gHmLdwsSet.errorXMargin || avgLine[1].cnt==180)
				{
					ldwsState[1] = STATE_ARROW;
					dx[1] = 0;
				}
                else if (qDx[1].qSum < -3 && dx[1] != 0 && qDx[1].qNum >= 3) //add jdy
				{
					ldwsState[1] = STATE_WARNING;
				}
				
		  if((qDx[0].qSum <= 0 && qDx[1].qSum < -1)  && (hslAbs((int)gHmLdwsSet.warningX[1] -(int)xLoc[1]) <= 5))
                {
      		      if(dx[1] < 0 && dx[1] > -gHmLdwsSet.errorXMargin && qDx[1].qNum >= 3) 
                        {
      		          if( gHmLdwsSet.warningAngle[1] + 6 >= avgLine[i].ang)// && (gHmLdwsSet.warningX[1] - 5) <= xLoc[1] )//add 20130527 jdy del
                            ldwsState[1] = STATE_WARNING;		  
                    }
                }
                if(xLoc[1] <= gHmLdwsSet.warningX[1] - 10)
                {
                    ldwsState[1] = STATE_ARROW;
                    dx[1] = 0;
                }
                if( gHmLdwsSet.warningAngle[1] - 10 >= avgLine[i].ang )
                {
                    ldwsState[1] = STATE_ARROW;
                    dx[1] = 0;
                }
                break;
            }   // switch end

			if (dx[i])
			{
				hslPushQ(&qDx[i], dx[i]);
			}

			if (hslAbs(dx[i])>gHmLdwsSet.errorXMargin)
			{
				hslInitQ(&qDx[i], QMAX);
			}
		}
		else
		{
			gHmLdwsSet.result_Ang[i] = 0;
			gHmLdwsSet.result_Rho[i] = 0;	  
					
    		notLane = 255;
    		if(ldwsState[i] != STATE_WARNING)
    		{
    			ldwsState[i] = STATE_NONE;
    			switch (i)
    			{
    				case 0:
    				{
    					if (warningFlg[i])
    						{
    							warningFlg[i]=0;
    							if (pre_xLoc[i] < gHmLdwsSet.warningX[i] && pre_xLocCenter[i] <= 7)
    							{
    								notLane = gHmLdwsSet.warningX[i] - pre_xLoc[i];
									if ( (qDx[0].qSum > 6 && qDx[1].qSum > 6 && qDx[0].qNum >= 7) && (notLane < 10)) //20130603 add 
    								{
    									for (qIdx=0; qIdx<4; qIdx++)
    									{
    										if (hslAbs(pre_dx[i].que[qIdx]) > gHmLdwsSet.errorXMargin)
    											break;
    									}
    									if (qIdx >= 4)
    									{
    										ldwsState[i] = STATE_WARNING;
    								    }
    								}
    							}
    						}
    						break;
    				} // case 0 end
    				case 1:
    				{
    					if (warningFlg[i])
    						{
    							warningFlg[i]=0;
    							if (pre_xLoc[i] > gHmLdwsSet.warningX[i] && pre_xLocCenter[i] <= 7)
    							{
    								notLane = pre_xLoc[i]-gHmLdwsSet.warningX[i];
									if ((qDx[0].qSum < -6 && qDx[1].qSum < -6 && qDx[1].qNum >= 7 ) && (notLane < 10)) // 20130603
    								{
    									for (qIdx=0; qIdx<4; qIdx++)
    									{
    										if (hslAbs(pre_dx[i].que[qIdx]) > gHmLdwsSet.errorXMargin)
    											break;
    									}
    									if (qIdx >= 4)
    									{
    										ldwsState[i] = STATE_WARNING;
    									}
    								}
    							}
    						}
    						break;
    				} // case 1 end
    			} // switch end
    		} // if end
		} // else end

	// Operation

				  
		switch (ldwsState[i])
		{
			case STATE_NORMAL:
				warningFlg[i] = 1;
				if(errFreeze[i]) errFreeze[i]--;
//				errFreeze[i]=0;
			case STATE_NONE:
			case STATE_ERROR:
//				  if (errFreeze[i]) errFreeze[i]--;
				if (flgWarning[i]) flgWarning[i]--;
				break;

			case STATE_ARROW:
			case STATE_LIGHT:
				errFreeze[i] = ARROW_FREEZE;
				break;

			case STATE_WARNING:
				//if (errFreeze[i] || ldwsState[1-i]==STATE_NORMAL)
				if (errFreeze[i])
				{
					//ldwsState[i] = STATE_NORMAL; //del jdy
					ldwsState[i] = STATE_NONE;	//add jdy
					flgWarning[0] = 0;	//add jdy
					flgWarning[1] = 0; // add jdy
				}
				else
				{
                    flgWarning[i]=1;
                    errFreeze[0]++; // add jdy
                    errFreeze[1]++; // add jdy
			  
//					  errFreeze[1-i] = ARROW_FREEZE*2;	  // Prevent the opposite warning bell.

				}
				break;
		}
	}   // for end
	
    if(xLoc[1] > gHmLdwsSet.centerPoint && xLoc[0] < gHmLdwsSet.centerPoint)
    {
        if( (xLoc[1] - xLoc[0]) <= (lane_distance + 10))
        {
            flgWarning[0] = flgWarning[1] = 0;				 // adjust 20130304
            errFreeze[0]++; 
            errFreeze[1]++; 
        }
    }
	if(L_wave >= 50)  errFreeze[0]++;
	if(R_wave >= 50) errFreeze[1]++;
	if (errFreeze[0]) ret = RET_FREEZE_LEFT;
	if (errFreeze[1]) ret |= RET_FREEZE_RIGHT;

    //if (ret == RET_WARNING_NONE) //del jdy
    {
        if (flgWarning[0]) 
        {
            ret = RET_WARNING_LEFT;
            pre_xLoc[0] = 255;
            ldwsState[0] = STATE_NONE;
            ldwsState[1] = STATE_NONE;
        }
        else if (flgWarning[1]) 
        {
            ret = RET_WARNING_RIGHT;
            pre_xLoc[1] = 255;		
            ldwsState[0] = STATE_NONE;
            ldwsState[1] = STATE_NONE;
        }
    }

	return ret;
}

HSUINT8 hsaGetLaneInfo(HSUINT8* src, int *r1, int *a1, int *r2, int *a2)
{
	HSINT16 x, y;
	HSUINT8 ret;
	static HSUINT32 dueCount=0;
	HS_STRUCT_POLAR_COORD mostLine[HTL_NUM];
	HS_STRUCT_POLAR_COORD avgLine[2];
	static HS_STRUCT_POLAR_COORD valid[2]={{0,0,0}, {0,0,0}};

	if (flgSecureCheck) 
	{
		hmSecureLock(src-gHmLdwsSet.width*gHmLdwsSet.heightOffset);
		return RET_WARNING_NONE;
	}

	if (flgSecureLock) return RET_WARNING_LEFT;
#ifdef EXPIRED_DUE_FRAME
	if (dueCount>EXPIRED_DUE_FRAME) return RET_EXPIRED;   // for protection
#endif
	dueCount++;
	hsaLengthDetect(src, gHmLdwsBuf,1);
	hsaHoughtransform_f(gHmLdwsBuf, mostLine, hsaIsRightLane);
	ret = hsaDecideTwoLane(mostLine, avgLine);

	if (avgLine[0].ang)
	{
		*r1 = valid[0].rho = avgLine[0].rho;
		*a1 = valid[0].ang = avgLine[0].ang;
	}

	if (avgLine[1].ang)
	{
		*r2 = valid[1].rho = avgLine[1].rho;
		*a2 = valid[1].ang = avgLine[1].ang;
	}
/*
	*r1 = avgLine[0].rho;
	*a1 = avgLine[0].ang;
	*r2 = avgLine[1].rho;
	*a2 = avgLine[1].ang;*/
#if 1
	if (!(dueCount&0x3F) && ret == RET_WARNING_NONE)
	{
		hsaGetVertex(valid[0], valid[1], &x, &y);

		if (x<gHmLdwsSet.centerPoint) x = gHmLdwsSet.centerPoint-1;
		else if (x>gHmLdwsSet.centerPoint) x = gHmLdwsSet.centerPoint+1;

		if (y<0) y = -1;
		else if (y>0) y = 1;
		
		hsaChangeSettingXY(x, y);
        gHmLdwsSet.cal_x = gHmLdwsSet.centerPoint;
        gHmLdwsSet.cal_y = gHmLdwsSet.heightOffset+ gHmLdwsSet.heightGap+y;
		  
	 //mv_UartPrintf("cal x : %d	cal y :  %d  center : %d\r\n", gHmLdwsSet.centerPoint, gHmLdwsSet.heightOffset+ gHmLdwsSet.heightGap+y, gHmLdwsSet.centerPoint);  //add jdy
	}
#endif

	return ret;
}

HSUINT8 hsaAutoCalib(HSUINT8* src, int *r1, int *a1, int *r2, int *a2, HSUINT16 calibCntSet)
{
	HSINT16 x, y;
	HSUINT8 ret=RET_CALIBRATION;
	static HSUINT32 centerX=0, centerY=0;
	static HSUINT32 calibCnt=0, frameCnt=0, errorCnt=0;
	HS_STRUCT_POLAR_COORD mostLine[HTL_NUM];
	HS_STRUCT_POLAR_COORD avgLine[2];
	static HS_STRUCT_POLAR_COORD valid[2]={{0,0,0}, {0,0,0}};
	int rho_temp;

	if (flgSecureLock) 
	{
		return RET_WARNING_NONE;
	}
	
	if (flgSecureCheck)
	{
		hmSecureLock(src-gHmLdwsSet.width*gHmLdwsSet.heightOffset);   // 20130415	
		(void)displaySystemMaker();
		return RET_CALIBRATION;
	}

	gHmLdwsSet.heightValid = 100-gHmLdwsSet.heightOffset;
	hsaLengthDetect(src, gHmLdwsBuf,0);
	hsaHoughtransform_f(gHmLdwsBuf, mostLine, hsaIsRightLane);
	hsaAvgLane(mostLine, avgLine);
	if (avgLine[0].ang)
	{
		*r1 = valid[0].rho = avgLine[0].rho;
		*a1 = valid[0].ang = avgLine[0].ang;
	}

	if (avgLine[1].ang)
	{
		*r2 = valid[1].rho = avgLine[1].rho;
		*a2 = valid[1].ang = avgLine[1].ang;
	}
/*
    *r1 = avgLine[0].rho;
	*a1 = avgLine[0].ang;
	*r2 = avgLine[1].rho;
	*a2 = avgLine[1].ang;	 
*/
    if (calibCnt<calibCntSet)
	{
        // gHmLdwsSet.errorXMargin = 0;
        // if (avgLine[0].ang || avgLine[1].ang)
		if (avgLine[0].ang && avgLine[1].ang)	 //jdy
		{
			hsaGetVertex(valid[0], valid[1], &x, &y);
			x = x - gHmLdwsSet.centerPoint;
            // if (x<gHmLdwsSet.centerPoint) x = -1;
            // else if (x>gHmLdwsSet.centerPoint) x = +1;
			if (x<-2) x/=2;
			else if (x<0) x=-1;
			else if (x>2) x/=2;
			else if (x>0) x=1;

			if (y<-2) y/=2;
			else if (y<0) y=-1;
			else if (y>2) y/=2;
			else if (y>0) y=1;

			calibCnt++;


			if (hslAbs(gHmLdwsSet.heightOffset + y - gHmLdwsSet.height/2) > gHmLdwsSet.calib_vertexBox_Hgap  //20130528
			 || hslAbs(gHmLdwsSet.centerPoint + x - gHmLdwsSet.width/2) > gHmLdwsSet.calib_vertexBox_Wgap)   //20130528
			{

			    if (calibCnt)   calibCnt--; // 20130510_modify_kys
				
				if (errorCnt++ > 100)
				{
					ret = RET_CALIB_TIMEOUT;
					frameCnt = 0;
					errorCnt = 0;
					calibCnt = 0;
					centerX = 0; centerY = 0;
					gHmLdwsSet.heightOffset = gHmLdwsSet.height/2;
					gHmLdwsSet.centerPoint = gHmLdwsSet.width/2;
				}
			}
			else
			{
				hsaChangeSettingXY(gHmLdwsSet.centerPoint+x, y);
                gHmLdwsSet.cal_x = gHmLdwsSet.centerPoint+x; //add jdy
                gHmLdwsSet.cal_y = gHmLdwsSet.heightOffset+ gHmLdwsSet.heightGap+y; //add jdy

				// centerX+=gHmLdwsSet.centerPoint;
				// centerY+=gHmLdwsSet.heightOffset;
			}
		}

		if (calibCnt==calibCntSet)  // test 20130723 jdy >= -> == modify
		{
			// hsaSetVertexInfo(centerX/calibCnt, centerY/calibCnt);
			calibCnt = 0;
			// centerX = 0; centerY = 0;
			ret = RET_WARNING_NONE;
			gHmLdwsSet.heightValid = 35;

			hsaGetRhoFromRule(gHmLdwsSet.warningAngle[0], &rho_temp);
			y = gHmLdwsSet.heightValid-1;
			gHmLdwsSet.warningX[0] = hsaRhoTheta2XY(rho_temp, gHmLdwsSet.warningAngle[0], &y);
			
			hsaGetRhoFromRule(gHmLdwsSet.warningAngle[1], &rho_temp);
			y = gHmLdwsSet.heightValid-1;
			gHmLdwsSet.warningX[1] = hsaRhoTheta2XY(rho_temp, gHmLdwsSet.warningAngle[1], &y);
		}
	}

	if (frameCnt++>(calibCntSet*10))
	{
		ret = RET_CALIB_TIMEOUT;
		frameCnt = 0;
		errorCnt = 0;
		calibCnt = 0;
		centerX = 0; centerY = 0;
		gHmLdwsSet.heightOffset = gHmLdwsSet.height/2;
		gHmLdwsSet.centerPoint = gHmLdwsSet.width/2;
	}
	
	return ret;
}

void hsaGetVertexInfo(HSINT16 *x, HSINT16 *y)
{
	*x = gHmLdwsSet.centerPoint;
	*y = gHmLdwsSet.heightOffset;
}

void hsaSetVertexInfo(HSINT16 x, HSINT16 y)
{
	hsaChangeSettingXY(x, y-gHmLdwsSet.heightOffset);
	hmInitMask();
}

void hsaGetVertex(HS_STRUCT_POLAR_COORD l1, HS_STRUCT_POLAR_COORD l2, HSINT16 *x, HSINT16 *y)
{
	int a1, a2, b1, b2;

	a1 = (LT_SIN(l1.ang)*1024+LT_COS(l1.ang)/2)/LT_COS(l1.ang);
	a2 = (LT_SIN(l2.ang)*1024+LT_COS(l2.ang)/2)/LT_COS(l2.ang);
	b1 = (l1.rho*1024*1024+LT_COS(l1.ang)/2)/LT_COS(l1.ang);
	b2 = (l2.rho*1024*1024+LT_COS(l2.ang)/2)/LT_COS(l2.ang);

	if (a1 == a2) *x = gHmLdwsSet.centerPoint;
	else *x = (b1-b2+(a1-a2)/2)/(a1-a2);
	
	*y = (l1.rho*1024 - (*x)*LT_SIN(l1.ang)+LT_COS(l1.ang)/2)/LT_COS(l1.ang);
}

void hsaChangeSettingXY(HSINT16 x, HSINT16 y)
{
	HS_STRUCT_POLAR_COORD l1, l2; 
	
	if(x > (gHmLdwsSet.width/2 + gHmLdwsSet.calib_vertexBox_Wgap))		//add jdy 
		x = (gHmLdwsSet.width/2) + gHmLdwsSet.calib_vertexBox_Wgap-1;	//add jdy
	else if(x < (gHmLdwsSet.width/2 - gHmLdwsSet.calib_vertexBox_Wgap)) //add jdy
		x = (gHmLdwsSet.width/2) - gHmLdwsSet.calib_vertexBox_Wgap-1;	//add jdy
	else gHmLdwsSet.centerPoint = x;					//add jdy
	
	if((gHmLdwsSet.heightOffset + y) > ((gHmLdwsSet.height/2) + gHmLdwsSet.calib_vertexBox_Hgap))		  //add jdy
		gHmLdwsSet.heightOffset = (gHmLdwsSet.height/2 + gHmLdwsSet.calib_vertexBox_Hgap-1);			//add jdy
	else if((gHmLdwsSet.heightOffset + y) < ((gHmLdwsSet.height/2) - gHmLdwsSet.calib_vertexBox_Hgap))	  //add jdy
		gHmLdwsSet.heightOffset = (gHmLdwsSet.height/2 - gHmLdwsSet.calib_vertexBox_Hgap-1);			//add jdy
	else gHmLdwsSet.heightOffset = gHmLdwsSet.heightOffset + y; 						//add jdy
	
	//gHmLdwsSet.centerPoint = x;  //del jdy
	//gHmLdwsSet.heightOffset = gHmLdwsSet.heightOffset + y; // delete jdy
	
	if (hslAbs(gHmLdwsSet.heightOffset - gHmLdwsSet.height/2)>gHmLdwsSet.calib_vertexBox_Hgap) 
	{
		gHmLdwsSet.heightOffset -= y;
		if(gHmLdwsSet.heightOffset > (gHmLdwsSet.height/2 + gHmLdwsSet.calib_vertexBox_Hgap))//jdy
			gHmLdwsSet.heightOffset = gHmLdwsSet.height/2 + gHmLdwsSet.calib_vertexBox_Hgap-1;//jdy
		if(gHmLdwsSet.heightOffset < (gHmLdwsSet.height/2 - gHmLdwsSet.calib_vertexBox_Hgap))//jdy
			gHmLdwsSet.heightOffset = gHmLdwsSet.height/2 - gHmLdwsSet.calib_vertexBox_Hgap-1;//jdy
	
	}
	if (hslAbs(gHmLdwsSet.centerPoint - gHmLdwsSet.width/2)>gHmLdwsSet.calib_vertexBox_Wgap)
	{
		gHmLdwsSet.centerPoint -= x;
		//if(gHmLdwsSet.centerPoint > ((gHmLdwsSet.width/2) + VERTEX_BOX_HGAP))//jdy
		//	  gHmLdwsSet.centerPoint = (gHmLdwsSet.width/2) + VERTEX_BOX_HGAP-1;//jdy
		//if(gHmLdwsSet.centerPoint < ((gHmLdwsSet.width/2) - VERTEX_BOX_HGAP))//jdy
		//	  gHmLdwsSet.centerPoint = (gHmLdwsSet.width/2) - VERTEX_BOX_HGAP-1;  //jdy
	}

	gHmLdwsSet.minRho = (gHmLdwsSet.centerPoint+1)/2;
	gHmLdwsSet.maxRho = gHmLdwsSet.centerPoint+gHmLdwsSet.marginRho;

	if (gHmLdwsSet.pointLimit<7) gHmLdwsSet.pointLimit = 7;

	hsaGetRhoFromRule(gHmLdwsSet.warningAngle[0], &l1.rho); 
	//y = gHmLdwsSet.heightValid-1;  //del jdy
	//y = gHmLdwsSet.height-gHmLdwsSet.heightOffset - 21; //add jdy
	y = gHmLdwsSet.heightValid; //add jdy
	gHmLdwsSet.warningX[0] = hsaRhoTheta2XY(l1.rho, gHmLdwsSet.warningAngle[0], &y);

   //  mv_UartPrintf("change setting y : %d\r\n", y);  //add jdy

	hsaGetRhoFromRule(gHmLdwsSet.warningAngle[1], &l2.rho);
	//y = gHmLdwsSet.heightValid-1; //del jdy
	//y = gHmLdwsSet.height-gHmLdwsSet.heightOffset - 21; //add jdy
	y = gHmLdwsSet.heightValid;
	gHmLdwsSet.warningX[1] = hsaRhoTheta2XY(l2.rho, gHmLdwsSet.warningAngle[1], &y);;

	gHmLdwsSet.widthOffset = gHmLdwsSet.centerPoint - gHmLdwsSet.widthValid/2;
	gHmLdwsSet.widthGap = gHmLdwsSet.width - gHmLdwsSet.centerPoint - gHmLdwsSet.widthValid/2;

	if (gHmLdwsSet.widthValid>gHmLdwsSet.width) gHmLdwsSet.widthValid = gHmLdwsSet.width;
	if (gHmLdwsSet.widthOffset>gHmLdwsSet.width) gHmLdwsSet.widthOffset = 0;
	if (gHmLdwsSet.widthGap>gHmLdwsSet.width) gHmLdwsSet.widthGap = 0;
}

/////////////////////////////////////////////////////////////////////

HSUINT16 hmGetRescaledAngle(HSUINT16 orgAngle, HSUINT16 orgWid, HSUINT16 orgHgt, HSUINT16 destWid, HSUINT16 destHgt)
{
	HSINT32 a, b, tanDest, tan;
	HSUINT32 diff1=-1, diff2;
	HSUINT16 i, ang=0;

	a = (destWid*1024+orgWid/2)/orgWid;
	b = (destHgt*1024+orgHgt/2)/orgHgt;

	tanDest = (b*LT_SIN(orgAngle)*1024+a*LT_COS(orgAngle)/2)/(a*LT_COS(orgAngle));

	for (i=0;i<180;i+=(180/ANGLE_COUNT))
	{
		tan = (LT_SIN(i)*1024+LT_COS(i)/2)/LT_COS(i);
		diff2 = hslAbs(tan-tanDest);
		if (diff2<diff1)
		{
			ang = i;
			diff1 = diff2;
		}
	}

	return ang;
}

void hmRescaleSetting(HS_STRUCT_LDWS_PARAMETER src, HS_STRUCT_LDWS_PARAMETER *dest, HSUINT16 width, HSUINT16 height)
{
	HSUINT16 ceilingW, ceilingH;

	ceilingW = src.width/2;
	ceilingH = src.height/2;
	
	dest->width = width;
	dest->widthOffset = (src.widthOffset * dest->width + ceilingW) / src.width;
	dest->widthGap = (src.widthGap * dest->width + ceilingW)/ src.width;
	dest->widthValid = (src.widthValid * dest->width + ceilingW)/ src.width;
	dest->centerPoint = (src.centerPoint * dest->width + ceilingW)/ src.width;

	dest->height = height;
	dest->heightOffset = (src.heightOffset * dest->height + ceilingH)/ src.height;
	dest->heightValid = (src.heightValid * dest->height + ceilingH)/ src.height;
	dest->heightGap = (src.heightGap * dest->height + ceilingH)/ src.height;
	
	dest->initAngle[0] = hmGetRescaledAngle(src.initAngle[0], src.width, src.height, width, height);
	dest->initAngle[1] = hmGetRescaledAngle(src.initAngle[1], src.width, src.height, width, height);
	dest->warningAngle[0] = hmGetRescaledAngle(src.warningAngle[0], src.width, src.height, width, height);
	dest->warningAngle[1] = hmGetRescaledAngle(src.warningAngle[1], src.width, src.height, width, height);
	dest->warningX[0] = (src.warningX[0] * dest->width + ceilingW)/ src.width;
	dest->warningX[1] = (src.warningX[1] * dest->width + ceilingW)/ src.width;

	dest->laneYGap = src.laneYGap;
	dest->laneMin = (src.laneMin * dest->width + ceilingW)/ src.width;
	dest->laneMax = (src.laneMax * dest->width + ceilingW)/ src.width;
	dest->pointLimit = (src.pointLimit * dest->height + ceilingH)/ src.height;

	if (dest->pointLimit<2) dest->pointLimit = 2;

	dest->minRho = (src.minRho * dest->width + ceilingW)/ src.width;
	dest->maxRho = (src.maxRho * dest->width + ceilingW)/ src.width;
	dest->marginRho = (src.marginRho * dest->width + ceilingW)/ src.width;
}

int displaySystemMaker(void)
{
    //hmDebug("HiveMotion Demo System : %d\r\n", 1211);
    //return 1116;
}

