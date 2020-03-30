#include <string.h>
#include <stdio.h>

#include "hslType.h"
#include "hscAttachLdws.h"
#include "hscSetting.h"
#include "hsaLdws.h"
//#include "hslGraphics.h"
//#include "hslImage.h"

// Debug Message Function
//#define hscDebugPrintf printf


static HSUINT32 gHmTotalTime, gHmTotalCnt;     // Kevin

// Settings for LDWS
HS_STRUCT_LDWS_PARAMETER gHmLdwsSet;
HSUINT8* gHmLdwsBuf;                    // Lane Detect Image
HSUINT8* gHmMaskBuf;
HSUINT16 *gHmHoughBuf;                  // HoughTransform Buffer

#define HM_DATE "2013/01/21"

int cl_checkic();

void hmInitLdws(void)
{

}

void hmInitMemoryLdws(void)
{
    gHmTotalTime = gHmTotalCnt = 0;  // Kevin
    gHmLdwsBuf = (HSUINT8*)hscMemAlloc(gHmLdwsSet.width*gHmLdwsSet.height); // Kevin
    gHmMaskBuf = (HSUINT8*)hscMemAlloc(gHmLdwsSet.width*gHmLdwsSet.height); // Kevin
    hscMemset((void*)gHmMaskBuf, 0xFF, gHmLdwsSet.width*gHmLdwsSet.height);
    gHmHoughBuf = (HSUINT16*)hscMemAlloc(gHmLdwsSet.width*180*2*2);
}

void hmExitMemoryLdws(void)
{
    hscMemFree(gHmLdwsBuf);
    hscMemFree(gHmMaskBuf);
    hscMemFree(gHmHoughBuf);
}

static void hmInitSetting(void)
{
    HSINT16 y, rho;
    
    gHmLdwsSet.width = 160;
    gHmLdwsSet.widthOffset = 30;
    gHmLdwsSet.widthGap = 30;
    gHmLdwsSet.widthValid = 100;
    gHmLdwsSet.centerPoint = 80;

    gHmLdwsSet.height = 120;
    gHmLdwsSet.heightOffset = 40;
    gHmLdwsSet.heightValid = 60;
    gHmLdwsSet.heightGap = 5;      // 10 jdy

    gHmLdwsSet.initAngle[0] = INIT_LEFT_ANGLE;
    gHmLdwsSet.initAngle[1] = INIT_RIGHT_ANGLE;
    gHmLdwsSet.result_Ang[0] = 0;
    gHmLdwsSet.result_Ang[1] = 0;    
    gHmLdwsSet.result_Rho[0] = 0;    
    gHmLdwsSet.result_Rho[1] = 0;        

    if(gHmLdwsSet.input_warningAngle[0])
	{
		gHmLdwsSet.warningAngle[0] = gHmLdwsSet.input_warningAngle[0];
	}
    else
	{
		gHmLdwsSet.warningAngle[0] = WARNING_ANGLE_LEFT;
	}
    if(gHmLdwsSet.input_warningAngle[1])
	{
		gHmLdwsSet.warningAngle[1] = gHmLdwsSet.input_warningAngle[1];
	}
	else
	{
		gHmLdwsSet.warningAngle[1] = WARNING_ANGLE_RIGHT;
	}

    y = gHmLdwsSet.heightValid;
    hsaGetRhoFromRule(gHmLdwsSet.warningAngle[0], &rho);
    gHmLdwsSet.warningX[0] = hsaRhoTheta2XY(rho, gHmLdwsSet.warningAngle[0], &y);
    hsaGetRhoFromRule(gHmLdwsSet.warningAngle[1], &rho);    
    gHmLdwsSet.warningX[1] = hsaRhoTheta2XY(rho, gHmLdwsSet.warningAngle[1], &y);
    
    gHmLdwsSet.laneYGap = LANE_Y_GAP;
    gHmLdwsSet.laneMin = LANE_MIN;
    gHmLdwsSet.laneMax = LANE_MAX;
    gHmLdwsSet.pointLimit = 8; //jdy org 8 //(gHmLdwsSet.heightValid-gHmLdwsSet.heightGap+1)/2;
    gHmLdwsSet.marginRho = 30;

    gHmLdwsSet.minRho = gHmLdwsSet.centerPoint/2;
    gHmLdwsSet.maxRho = gHmLdwsSet.centerPoint;

    gHmLdwsSet.errorXMargin = ERROR_STEP_XLOC;

    gHmLdwsSet.cal_x = 0;
    gHmLdwsSet.cal_y = 0;

    gHmLdwsSet.calib_vertexBox_Wgap = 0;    // modify_20130528_kys
    gHmLdwsSet.calib_vertexBox_Hgap = 0;

        
}

HSUINT8 hmRhoTheta2XY(int rho, int ang, HSUINT16 w, HSUINT16 h, HSINT16 *sx1, HSINT16 *sy1, HSINT16 *sx2, HSINT16 *sy2)
{
	HSINT16 x1, y1, x2, y2;
	int scaleX, scaleY;

	if(rho && ang)
	{
		scaleX = (w*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
		scaleY = (h*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;

		y1 = 0;
		x1 = hsaRhoTheta2XY(rho, ang, &y1);
		y2 = gHmLdwsSet.heightValid-1;
		x2 = hsaRhoTheta2XY(rho, ang, &y2);

		*sx1 = x1*scaleX/1024;	
		*sx2 = x2*scaleX/1024;
		*sy1 = (y1+gHmLdwsSet.heightOffset)*scaleY/1024;
		*sy2 = (y2+gHmLdwsSet.heightOffset)*scaleY/1024;
		return 1;
	}
	else
	    return 0;
}

void hmInitMask(void)
{
    HSUINT16 i,j,w, s;

#if 0    
    hscMemset((void*)gHmMaskBuf, 0x00, gHmLdwsSet.width*gHmLdwsSet.heightValid);
    for (i=0;i<gHmLdwsSet.heightValid/2;i++)
    {
        w = gHmLdwsSet.widthValid*i/gHmLdwsSet.heightValid*2;
        s = gHmLdwsSet.widthValid/2 - w/2 + gHmLdwsSet.widthOffset;
        for (j=0;j<w;j++)
        {
            gHmMaskBuf[i*gHmLdwsSet.width+j+s] = 0xFF;
        }
    }
    for (i=gHmLdwsSet.heightValid/2;i<gHmLdwsSet.heightValid;i++)
    {
        hscMemset((void*)(gHmMaskBuf+i*gHmLdwsSet.width + gHmLdwsSet.widthOffset), 0xFF, gHmLdwsSet.widthValid);
    }
#else
    hscMemset((void*)gHmMaskBuf, 0xFF, gHmLdwsSet.width*gHmLdwsSet.heightValid);
/*    for (i=0;i<gHmLdwsSet.heightValid;i++)
    {
        for (j=0;j<gHmLdwsSet.heightValid-i-1;j++)
        {
            gHmMaskBuf[i*gHmLdwsSet.width+j+gHmLdwsSet.widthOffset] = 0x00;
            gHmMaskBuf[(i+1)*gHmLdwsSet.width-j-1-gHmLdwsSet.widthGap] = 0x00;
        }
    }*/
#endif
}

static void hmDisplayIcon(HSUINT16 *img, HSUINT16 x, HSUINT16 y, HSUINT16 *icon, HSUINT16 iconWid, HSUINT16 iconHgt)
{

}




void hmShowGuideLine16(HSUINT16 *img)
{

}


void hmShowAutoCalibGuide(HS_STRUCT_LDWS_CONFIG *config)
{
    HSUINT32 hratio = config->lcd_width*1024/gHmLdwsSet.width, vratio = config->lcd_height*1024/gHmLdwsSet.height;
    HSUINT32 boxgapV = gHmLdwsSet.calib_vertexBox_Hgap*vratio/1024;
    HSUINT32 boxgapH = gHmLdwsSet.calib_vertexBox_Wgap*hratio/1024;

    config->calib_area_top.x1 = config->lcd_width/2-boxgapH;
    config->calib_area_top.y1 = config->lcd_height/2-boxgapV;
    config->calib_area_top.x2 = config->lcd_width/2+boxgapH;
    config->calib_area_top.y2 = config->lcd_height/2-boxgapV;

    config->calib_area_bottom.x1 = config->lcd_width/2-boxgapH;
    config->calib_area_bottom.y1 = config->lcd_height/2+boxgapV;
    config->calib_area_bottom.x2 = config->lcd_width/2+boxgapH;
    config->calib_area_bottom.y2 = config->lcd_height/2+boxgapV;
}

void hmsGetVanishingPoint(HS_STRUCT_LDWS_CONFIG *config)
{
    int scaleX, scaleY;

    scaleX = (config->lcd_width*1024+gHmLdwsSet.width/2)/gHmLdwsSet.width;
    scaleY = (config->lcd_height*1024+gHmLdwsSet.height/2)/gHmLdwsSet.height;
    config->vanishing_point_x= gHmLdwsSet.centerPoint*scaleX/1024;
    config->vanishing_point_y= (gHmLdwsSet.heightOffset)*scaleY/1024;
}



#define LDWS_STATUS_START       0
#define LDWS_STATUS_RUNNING     1
#define LDWS_STATUS_CALIBRATION 2
#define LDWS_STATUS_RESTART     3

//#define ENABLE_FCWS
#ifdef ENABLE_FCWS
extern int gLdwsMenu;   // Kevin
//#include "hscAttachFcws.h"
#endif

int hmLdwsFrameWork(HSUINT8* src, void* dest, HSUINT16 x, HSUINT16 y, HS_STRUCT_LDWS_CONFIG *configLdws)
{
    HSUINT8 *pDest = (HSUINT8*)dest;
    int rho[2]={0,0}, ang[2]={0,0};
    int rho_temp;
    static int rho_x[2], ang_x[2];
    int scaleX, scaleY;
    HSUINT8 warning = 0;
    static HSUINT8 stsLdws=LDWS_STATUS_START;
    static HSUINT32 frameErrCnt, framecnt = 0;
    static HSUINT8 no_warning_cnt = 0;

    static HSUINT8 autoCalFlg=0, leftMaxIdx=0, rightMaxIdx=0;
    static HSUINT16 leftCalCnt, rightCalCnt;
    HSINT16 leftLaneCheck, rightLaneCheck;
    HSINT16 sx1=0, sy1=0, sx2=0,  sy2=0;
    static HSUINT8 license=1, outVer=1;
    static HSINT16 preLeftXY[4]={0,}, preRightXY[4]={0,}, testFlg=1;
    HSUINT16 i, j;
    HSUINT8 val;

    if (outVer)
    {
        
        #ifdef LIB_CORELOGIC_LOCK
        hscDebugPrintf("HM_VERSION_CORELOGIC_LOCK:%d_%d_%d \r\n", HM_YEAR, HM_MONTH, HM_DAY);
        fflush(stdout);
        #else
        hscDebugPrintf("HM_VERSION_CORELOGIC_UNLOCK:%d_%d_%d \r\n", HM_YEAR, HM_MONTH, HM_DAY);
        fflush(stdout);
        #endif
        
        outVer=0;
    }

#if 1
    if( cl_checkic() == 0 )
    {
        license = 0;
    }

    if (license == 0)   return 99;
#endif

/**************/
/* Initialize */
/**************/
	if(configLdws->Ldws_reset == 1)
	{
	    hmExitMemoryLdws();
		configLdws->Ldws_reset = 0;
		stsLdws = LDWS_STATUS_START;
		x = y = 0;
	}

    if (stsLdws == LDWS_STATUS_START)
    {
        if (configLdws->left_warning_angle > 0)
            gHmLdwsSet.input_warningAngle[0] = configLdws->left_warning_angle;
        else
            gHmLdwsSet.input_warningAngle[0] = 0;
            
        if (configLdws->right_warning_angle > 0)
            gHmLdwsSet.input_warningAngle[1] = configLdws->right_warning_angle;
        else
            gHmLdwsSet.input_warningAngle[1] = 0;
		
        hmInitSetting();
        hmInitMemoryLdws();
        hmInitMask();

        // modify_20130528_kys
        if (configLdws->calibArea_w_gap > 0 && configLdws->calibArea_w_gap < gHmLdwsSet.width/2)
        {
            gHmLdwsSet.calib_vertexBox_Wgap = configLdws->calibArea_w_gap;
        }
        else
        {
            gHmLdwsSet.calib_vertexBox_Wgap = VERTEX_BOX_HGAP;  // width
        }
        
        if (configLdws->calibArea_h_gap > 0 && configLdws->calibArea_h_gap < gHmLdwsSet.height/2)
        {
            gHmLdwsSet.calib_vertexBox_Hgap = configLdws->calibArea_h_gap;
        }
        else
        {
            gHmLdwsSet.calib_vertexBox_Hgap = VERTEX_BOX_VGAP;  // height
        }

        if (x && y)
        {
            scaleX = configLdws->lcd_width / gHmLdwsSet.width;
            scaleY = configLdws->lcd_height / gHmLdwsSet.height;

			//printf("input %d %d  output %d %d  scale  %d %d \r\n", x, y, x/scaleX, y/scaleY, scaleX, scaleY );
			
			x = x/scaleX;
			y = y/scaleY;
			
            hsaSetVertexInfo(x, y);
            stsLdws = LDWS_STATUS_RUNNING;
            frameErrCnt=0;
        }
        else 
        {
            stsLdws = LDWS_STATUS_CALIBRATION;
        }

    }
    hscMemset((void*)gHmHoughBuf, 0x0, gHmLdwsSet.width*180*4);

/*******************************/
/* Calibration && Running LDWS */
/*******************************/
    switch (stsLdws)
    {
        case LDWS_STATUS_RUNNING:
            frameErrCnt++;
            warning = hsaGetLaneInfo(src + gHmLdwsSet.width*gHmLdwsSet.heightOffset, &rho[0], &ang[0], &rho[1], &ang[1]); // Core Function

            if(no_warning_cnt)
            {
                no_warning_cnt--;
		        warning  = RET_WARNING_NONE;
            }
	     
            if (ang[0] || ang[1])
                frameErrCnt = frameErrCnt>>1;
            if (frameErrCnt>0x100) stsLdws = LDWS_STATUS_RESTART;

            if (testFlg)
            {
                hscDebugPrintf("End AutoCalibation!!___Start Running!! \r\n");
                fflush(stdout);
                testFlg=0;
            }
           
            break;

        case LDWS_STATUS_CALIBRATION:
            warning = hsaAutoCalib(src + gHmLdwsSet.width*gHmLdwsSet.heightOffset, &rho[0], &ang[0], &rho[1], &ang[1], 20);
            if (warning==RET_WARNING_NONE)
            {
                //Menu_Setup_Beep();
                stsLdws = LDWS_STATUS_RUNNING;
                frameErrCnt=0;
                gHmLdwsSet.marginRho = MARGIN_RHO;
            }
            else if (warning == RET_CALIB_TIMEOUT)
            {
                /* TBD */
                warning==RET_CALIBRATION;
               // stsLdws = LDWS_STATUS_CALIBRATION; //add jdy 20130723 
            }
            break;
        case LDWS_STATUS_RESTART:
            hmInitSetting();
            //20130723 modify jdy // modify_20130528_kys 
        if (configLdws->calibArea_w_gap > 0 && configLdws->calibArea_w_gap < gHmLdwsSet.width/2)
        {
            gHmLdwsSet.calib_vertexBox_Wgap = configLdws->calibArea_w_gap;
        }
        else
        {
            gHmLdwsSet.calib_vertexBox_Wgap = VERTEX_BOX_HGAP;  // width
        }
        
        if (configLdws->calibArea_h_gap > 0 && configLdws->calibArea_h_gap < gHmLdwsSet.height/2)
        {
            gHmLdwsSet.calib_vertexBox_Hgap = configLdws->calibArea_h_gap;
        }
        else
        {
            gHmLdwsSet.calib_vertexBox_Hgap = VERTEX_BOX_VGAP;  // height
        }
            stsLdws = LDWS_STATUS_CALIBRATION;
            break;
    }
    gHmLdwsSet.stsLdws = stsLdws;
    configLdws->status = stsLdws;
    
/*********************/
/* Show Informations */
/*********************/
#if 1
    if (stsLdws == LDWS_STATUS_CALIBRATION)
    {
        //hmShowAutoCalibGuide((HSUINT16*)pDest);
        hmShowAutoCalibGuide(configLdws);
    }
#endif

#if 1
/*************************/
/* Show Warning Messages */
/*************************/

    // 차선 좌표값 구함
	if (configLdws->lcd_width > 0 && configLdws->lcd_height > 0)
	{
	   	if (ang[0])
	   	{
            hmRhoTheta2XY(rho[0], ang[0], configLdws->lcd_width, configLdws->lcd_height, &sx1, &sy1, &sx2,  &sy2);
            configLdws->left_lane_point.x1 = (HSUINT32)sx1;
            configLdws->left_lane_point.y1 = (HSUINT32)sy1;
            configLdws->left_lane_point.x2 = (HSUINT32)sx2;
            configLdws->left_lane_point.y2 = (HSUINT32)sy2;
	    }
	    else
	    {
	        // 20130520_kys test
            configLdws->left_lane_point.x1 = 1;
            configLdws->left_lane_point.y1 = 1;
            configLdws->left_lane_point.x2 = 1;
            configLdws->left_lane_point.y2 = 1;
	    }

	    if (ang[1])
	    {
			hmRhoTheta2XY(rho[1], ang[1], configLdws->lcd_width, configLdws->lcd_height, &sx1, &sy1, &sx2,  &sy2);
			configLdws->right_lane_point.x1 = (HSUINT32)sx1;
			configLdws->right_lane_point.y1 = (HSUINT32)sy1;
			configLdws->right_lane_point.x2 = (HSUINT32)sx2;
			configLdws->right_lane_point.y2 = (HSUINT32)sy2;
	    }
	    else
	    {
	        // 20130520_kys test
			configLdws->right_lane_point.x1 = 1;
			configLdws->right_lane_point.y1 = 1;
			configLdws->right_lane_point.x2 = 1;
			configLdws->right_lane_point.y2 = 1;
	    }
	}

    // warning line 값 구함
	if (configLdws->lcd_width > 0 && configLdws->lcd_height > 0)
	{
        hmsGetVanishingPoint(configLdws);   // 소실점 x, y 값 구함
	
        hsaGetRhoFromRule(gHmLdwsSet.warningAngle[0], &rho_temp);
    	hmRhoTheta2XY(rho_temp, gHmLdwsSet.warningAngle[0], configLdws->lcd_width, configLdws->lcd_height, &sx1, &sy1, &sx2,  &sy2);
    	configLdws->left_warning_point.x1 = (HSUINT32)sx1;
    	configLdws->left_warning_point.y1 = (HSUINT32)sy1;
    	configLdws->left_warning_point.x2 = (HSUINT32)sx2;
    	configLdws->left_warning_point.y2 = (HSUINT32)sy2;
    	
        hsaGetRhoFromRule(gHmLdwsSet.warningAngle[1], &rho_temp);
    	hmRhoTheta2XY(rho_temp, gHmLdwsSet.warningAngle[1], configLdws->lcd_width, configLdws->lcd_height, &sx1, &sy1, &sx2,  &sy2);
    	configLdws->right_warning_point.x1 = (HSUINT32)sx1;
    	configLdws->right_warning_point.y1 = (HSUINT32)sy1;
    	configLdws->right_warning_point.x2 = (HSUINT32)sx2;
    	configLdws->right_warning_point.y2 = (HSUINT32)sy2;

    }

    //hscDebugPrintf("-------------------------------->ldws_right_lane (%d,      %d) \r\n", rho[0], rho[1]);
    #if 0
    if (ang[0] && ang[1])
    {
        hscDebugPrintf("hmLdws warning : %d, Point : %d , %d , width %d, height %d\r\n", warning, configLdws->vanishing_point_x, configLdws->vanishing_point_y, configLdws->lcd_width, configLdws->lcd_height);

        hscDebugPrintf("--------------->ldws_point_lane (%d, %d) \r\n", configLdws->vanishing_point_x, configLdws->vanishing_point_y);
        hscDebugPrintf("--------------->ldws_left _lane (%d, %d - %d, %d) \r\n", configLdws->left_lane_point.x1, configLdws->left_lane_point.y1, configLdws->left_lane_point.x2, configLdws->left_lane_point.y2);
        hscDebugPrintf("--------------->ldws_right_lane (%d, %d - %d, %d) \r\n", configLdws->right_lane_point.x1, configLdws->right_lane_point.y1, configLdws->right_lane_point.x2, configLdws->right_lane_point.y2);
        

        fflush(stdout);
    }
    #endif

    switch (warning&0x3)
    {
        case RET_WARNING_NONE:
            break;
        case RET_WARNING_LEFT:
	        no_warning_cnt = 30; // add jdy
            break;
        case RET_WARNING_RIGHT:
	        no_warning_cnt = 30; // add jdy
            break;
    }

	configLdws->warning = warning;
	if(configLdws->Warning_type)	
	{
		if(no_warning_cnt)
		{
			configLdws->warning = 48; // 20130528 RET_FREEZE_LEFT | RET_FREEZE_RIGHT
		}
		if(warning == RET_WARNING_LEFT || warning == RET_WARNING_RIGHT)
		{
			no_warning_cnt = configLdws->Warning_time;
		}
	}
	else
	{
		if(no_warning_cnt)
		{
			configLdws->warning = 48; // 20130528 RET_FREEZE_LEFT | RET_FREEZE_RIGHT
		}
		if(warning == RET_WARNING_LEFT || warning == RET_WARNING_RIGHT)
		{
			no_warning_cnt = 30;
		}
	}
#if 0
    if (warning == 1)
    {
        hscDebugPrintf("-------------------------------------------------------\r\n");
        hscDebugPrintf("ldws_left _warning (0x%2x) ang (%d) \r\n", warning, ang[0]);
        //hscDebugPrintf("ldws_left _lane (%d, %d - %d, %d) \r\n", configLdws->left_lane_point.x1, configLdws->left_lane_point.y1, configLdws->left_lane_point.x2, configLdws->left_lane_point.y2);
        //hscDebugPrintf("ldws_right_lane (%d, %d - %d, %d) \r\n", configLdws->right_lane_point.x1, configLdws->right_lane_point.y1, configLdws->right_lane_point.x2, configLdws->right_lane_point.y2);
        hscDebugPrintf("-               \r\n");
        hscDebugPrintf("-               \r\n");
        hscDebugPrintf("-               \r\n");
        hscDebugPrintf("-               \r\n");
        hscDebugPrintf("-------------------------------------------------------\r\n");        
        fflush(stdout);
    }
    if (warning == 2)
    {
        hscDebugPrintf("-------------------------------------------------------\r\n");    
        hscDebugPrintf("ldws_right_warning (0x%2x) ang (%d) \r\n", warning, ang[1]);
        //hscDebugPrintf("ldws_left _lane (%d, %d - %d, %d) \r\n", configLdws->left_lane_point.x1, configLdws->left_lane_point.y1, configLdws->left_lane_point.x2, configLdws->left_lane_point.y2);
        //hscDebugPrintf("ldws_right_lane (%d, %d - %d, %d) \r\n", configLdws->right_lane_point.x1, configLdws->right_lane_point.y1, configLdws->right_lane_point.x2, configLdws->right_lane_point.y2);
        hscDebugPrintf("--              \r\n");
        hscDebugPrintf("--              \r\n");
        hscDebugPrintf("--              \r\n");
        hscDebugPrintf("--              \r\n");
        hscDebugPrintf("-------------------------------------------------------\r\n");
        fflush(stdout);
    }
#endif


#if 0
	// display LdwsBuf (160 x 120)

	if (dest != NULL)
	{
        hsaGetRhoFromRule(gHmLdwsSet.warningAngle[0], &rho_temp);    
        hsaDrawLineRhoThetaY8Scale_(160, 120, (HSUINT8*)src, rho_temp, gHmLdwsSet.warningAngle[0], 0x00);
        hsaGetRhoFromRule(gHmLdwsSet.warningAngle[1], &rho_temp);
        hsaDrawLineRhoThetaY8Scale_(160, 120, (HSUINT8*)src, rho_temp, gHmLdwsSet.warningAngle[1], 0x00);
	
        for (i=0;i<gHmLdwsSet.heightValid;i++)
        {
            for (j=0;j<gHmLdwsSet.width;j++)
            {
                if (gHmLdwsBuf[(i+gHmLdwsSet.heightGap)*gHmLdwsSet.width+j])
                {
                    pDest[i*320+j] = 0xFF;
                }
                else
                {
                    pDest[i*320+j] = 0x00;
                }

                val = src[(i+gHmLdwsSet.heightOffset+gHmLdwsSet.heightGap)*gHmLdwsSet.width+j];
                pDest[i*320+(160+j)] = val;
            }
        }
        
        //for (i=0; i<gHmLdwsSet.heightValid; i++)
        //{
        //    pDest[i*320+(gHmLdwsSet.warningX[0]+160)] = 0x00;
        //    pDest[i*320+(gHmLdwsSet.warningX[1]+160)] = 0x00;
        //}
    }
#endif

    return warning;
}
#endif
