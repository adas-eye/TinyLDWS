#include "hslType.h"
#include <malloc.h>

#define hscDebugPrintf printf

// Memory Function
#define hscMemset memset
#define hscMemcpy memcpy
#define hscMemAlloc malloc
#define hscMemFree	free

// Settings for LDWS
typedef struct {
	HSUINT16 width;
	HSUINT16 widthOffset;
	HSUINT16 widthValid;
	HSUINT16 widthGap;

	HSUINT16 height;
	HSUINT16 heightOffset;
	HSUINT16 heightValid;
	HSUINT16 heightGap;

	HSUINT16 initAngle[2];
	HSUINT16 result_Ang[2];
	HSUINT16 result_Rho[2];    
	
	HSUINT16 warningAngle[2];
	HSUINT16 input_warningAngle[2];
	HSUINT16 warningX[2];

	HSUINT16 centerPoint;
	HSUINT16 minRho;
	HSUINT16 maxRho;

	HSUINT16 laneYGap;
	HSUINT16 laneMin;
	HSUINT16 laneMax;
	
	HSUINT16 pointLimit;
	HSUINT16 marginRho;

	HSINT16 errorXMargin;
	HSUINT8 stsLdws;

	HSUINT16 cal_x; //add jdy after calibration x
	HSUINT16 cal_y; //add jdy after calibration y

	HSUINT8 calib_vertexBox_Wgap;
	HSUINT8 calib_vertexBox_Hgap;
} HS_STRUCT_LDWS_PARAMETER;

typedef struct {
	HSUINT32 x1;
	HSUINT32 y1;
	HSUINT32 x2;
	HSUINT32 y2;
} HS_STRUCT_LANE_POINT_XY;

typedef struct {
	HSUINT8 warning;                                        // warning status
	HSUINT8 status;                                         // system Status

	// 차선 등을 그려줄 영상의 size 입력
	HSINT16 lcd_width;                                     // output Image Width
	HSINT16 lcd_height;                                    // output Image Height	

    // 차선 x, y 값 (lcd_width/height 가 입력 되어야 함)
	HS_STRUCT_LANE_POINT_XY left_lane_point;                // left lane x, y
	HS_STRUCT_LANE_POINT_XY right_lane_point;               // right lane x, y	

	// warning 민감도, angle 값을 0으로 주면 내부에 설정되어 있는 값이 적용 됨(내부 설정은 left 65도, right 115도)
	HSUINT8 left_warning_angle;                             // Left Warning Line Angle 입력
	HSUINT8 right_warning_angle;                            // Right Warning Line Angle 입력

	// autoCalibration 영역 정보 (lcd_width/height 가 입력 되어야 함)
	HS_STRUCT_LANE_POINT_XY calib_area_top;                 // calibration area top x, y
	HS_STRUCT_LANE_POINT_XY calib_area_bottom;              // calibration area bottom x, y

	// 경고 기준선 (lcd_width/height 가 입력 되어야 함)
	HS_STRUCT_LANE_POINT_XY left_warning_point;				// left warning x, y
	HS_STRUCT_LANE_POINT_XY right_warning_point;			// right warning x, y

	// 소실점 위치 (lcd_width/height 가 입력 되어야 함)
	HSINT16 vanishing_point_x;								// 소실점 위치 x 출력
	HSINT16 vanishing_point_y;								// 소실점 위치 y 출력
	
	HSUINT8 Ldws_reset;								    	// Ldws reset

	HSUINT8 calibArea_w_gap;                                // Calibration Area Width 조절
	HSUINT8 calibArea_h_gap;                                // Calibration Area Height 조절

	HSUINT8 Warning_type;									// 0: 단일 경고 1: 경고 시간 선택
	HSUINT8 Warning_time;									// Warning_type 이 1일 때 0~30으로 조절

} HS_STRUCT_LDWS_CONFIG;



// Function Prototype
int hmLdwsFrameWork(HSUINT8* src, void* dest, HSUINT16 x, HSUINT16 y, HS_STRUCT_LDWS_CONFIG *configLdws);

/************************************************************************
 * External Variables
 ************************************************************************/
extern HSUINT8*  gHmLdwsBuf;					 // Lane Detect Image
extern HSUINT8*  gHmMaskBuf;					 // Mask Image
extern HSUINT16* gHmHoughBuf;					 // HoughTransform Buffer

extern HS_STRUCT_LDWS_PARAMETER gHmLdwsSet;
