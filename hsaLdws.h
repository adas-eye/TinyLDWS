// Adas_LDWS.h : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
/*
 *******************************************************************************
 *  (C) Copyright HiveMotion Ltd. 2012 All right reserved.
 *    Confidential Information
 *  
 *  All parts of the HiveMotion Program Source are protected by copyright law 
 *  and all rights are reserved. 
 *  This documentation may not, in whole or in part, be copied, photocopied, 
 *  reproduced, translated, or reduced to any electronic medium or machine 
 *  readable form without prior consent, in writing, from HiveMotion. 
 *******************************************************************************
 *  Project     : ADAS LDWS
 *  Description : LDWS Function.
 *
 *******************************************************************************
 *  Version control information
 *  2012.02.29 - create version
 *
 *******************************************************************************
 *  Modification History
 *
 *******************************************************************************
 */
/*******************************************************************************
 * DEFINED LITERALS & DEFINITIONS                                              
 ******************************************************************************/
#include "hslType.h"

/*****************************************/
// 코아로직 설정
/*****************************************/
//#define LIB_CORELOGIC_LOCK
#define HM_YEAR    2013
#define HM_MONTH   11
#define HM_DAY     18

 
//#define FRC_ENABLE     /* Multi-frame calculation per 1 LDWS */

#define RET_WARNING_NONE    0
#define RET_WARNING_LEFT    1
#define RET_WARNING_RIGHT   2
#define RET_CALIBRATION     4
#define RET_CALIB_TIMEOUT   8
#define RET_FREEZE_LEFT     16
#define RET_FREEZE_RIGHT    32
#define RET_EXPIRED         64

#ifdef LIB_CORELOGIC_LOCK
#define EXPIRED_DUE_FRAME   30/*fps*/*60/*second*/*60/*minute*/ 
#endif

// Org VERTEX BOX V : 25
// Org VERTEX BOX H : 30
// Height offset : 40
// CoreLogic Test VERTEX BOX V : 40
// CoreLogic Test VERTEX BOX H : 30
// Height offset : 20
#define VERTEX_BOX_VGAP     40	//25 //jdy  height
#define VERTEX_BOX_HGAP     60	//40 //jdy  width
#define DIFFSUM_MARGIN      200
#define WARNING_GAP		10
/*******************************************************************************
 * DEFINE STRUCTURE
 ******************************************************************************/
typedef struct {
    HSINT16 rho;
    HSINT16 ang;
    HSINT16 cnt;
} HS_STRUCT_POLAR_COORD;

typedef int (* hsaFpDecideRightLane_CALLBACK )(int rho, int ang);

/*******************************************************************************
 * DEFINE FUNCTIONS
 ******************************************************************************/
// Gain the rho value from angle using decided rule.
void hsaGetRhoFromRule(HSINT16 ang, HSINT16 *rho);

// Calculate two average lane value (Left, Right) using mostLine value
void hsaAvgLane(HS_STRUCT_POLAR_COORD *mostLine, HS_STRUCT_POLAR_COORD *avgLine);

// Decide if it is the right lane or not given rho and angle.
int  hsaIsRightLane(HSINT16 rho, HSINT16 ang);

// Algorithm
void hmInitMask(void);
void hsaLengthDetect(HSUINT8 *pYBuffer, HSUINT8 *pSetData, HSUINT8 flag);
void hsaLengthFilter(HSUINT8* ldwsBuf, HSUINT16 wid, HSUINT16 hei);
void hsaHoughtransform_f(HSUINT8 *greyBuf, HS_STRUCT_POLAR_COORD *ml, hsaFpDecideRightLane_CALLBACK rightLane);
HSUINT8 hsaDecideTwoLane(HS_STRUCT_POLAR_COORD *mostLine, HS_STRUCT_POLAR_COORD *avgLine);

// Main Interface Function
HSUINT8 hsaGetLaneInfo(HSUINT8* src, int *r1, int *a1, int *r2, int *a2);
HSUINT8 hsaAutoCalib(HSUINT8* src, int *r1, int *a1, int *r2, int *a2, HSUINT16 calibCntSet);
void hsaGetVertexInfo(HSINT16 *x, HSINT16 *y);
void hsaSetVertexInfo(HSINT16 x, HSINT16 y);

// Draw line using rho and angle values.
HSINT16  hsaRhoTheta2XY(int rho, int ang, int *y);
void hsaDrawLineRhoThetaY8(HSUINT8 *ptr, int rho, int theta, HSUINT8 c);
void hsaDrawLineRhoThetaY8Scale(int w, int h, HSUINT8 *ptr, int rho, int theta, HSUINT8 c);
void hsaDrawLineRhoThetaY8Scale_(int w, int h, HSUINT8 *ptr, int rho, int theta, HSUINT8 c);

void hsaDrawLineRhoThetaY16(HSUINT16 *ptr, int rho, int theta, HSUINT16 c);
void hsaDrawLineRhoThetaY16Scale(int w, int h, HSUINT16 *ptr, int rho, int theta, HSUINT16 c);
void hsaDrawLineRhoThetaY16Scale_(int w, int h, HSUINT16 *ptr, int rho, int theta, HSUINT16 c);
HSUINT16 hmGetRescaledAngle(HSUINT16 orgAngle, HSUINT16 orgWid, HSUINT16 orgHgt, HSUINT16 destWid, HSUINT16 destHgt);

HSUINT8 hsaChangeSetting(HS_STRUCT_POLAR_COORD l1, HS_STRUCT_POLAR_COORD l2);
void hsaGetVertex(HS_STRUCT_POLAR_COORD l1, HS_STRUCT_POLAR_COORD l2, HSINT16 *x, HSINT16 *y);
void hsaChangeSettingXY(HSINT16 x, HSINT16 y);

void hmRescaleSetting(HS_STRUCT_LDWS_PARAMETER src, HS_STRUCT_LDWS_PARAMETER *dest, HSUINT16 width, HSUINT16 height);
int displaySystemMaker(void);
