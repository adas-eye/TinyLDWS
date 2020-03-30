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

	// ���� ���� �׷��� ������ size �Է�
	HSINT16 lcd_width;                                     // output Image Width
	HSINT16 lcd_height;                                    // output Image Height	

    // ���� x, y �� (lcd_width/height �� �Է� �Ǿ�� ��)
	HS_STRUCT_LANE_POINT_XY left_lane_point;                // left lane x, y
	HS_STRUCT_LANE_POINT_XY right_lane_point;               // right lane x, y	

	// warning �ΰ���, angle ���� 0���� �ָ� ���ο� �����Ǿ� �ִ� ���� ���� ��(���� ������ left 65��, right 115��)
	HSUINT8 left_warning_angle;                             // Left Warning Line Angle �Է�
	HSUINT8 right_warning_angle;                            // Right Warning Line Angle �Է�

	// autoCalibration ���� ���� (lcd_width/height �� �Է� �Ǿ�� ��)
	HS_STRUCT_LANE_POINT_XY calib_area_top;                 // calibration area top x, y
	HS_STRUCT_LANE_POINT_XY calib_area_bottom;              // calibration area bottom x, y

	// ��� ���ؼ� (lcd_width/height �� �Է� �Ǿ�� ��)
	HS_STRUCT_LANE_POINT_XY left_warning_point;				// left warning x, y
	HS_STRUCT_LANE_POINT_XY right_warning_point;			// right warning x, y

	// �ҽ��� ��ġ (lcd_width/height �� �Է� �Ǿ�� ��)
	HSINT16 vanishing_point_x;								// �ҽ��� ��ġ x ���
	HSINT16 vanishing_point_y;								// �ҽ��� ��ġ y ���
	
	HSUINT8 Ldws_reset;								    	// Ldws reset

	HSUINT8 calibArea_w_gap;                                // Calibration Area Width ����
	HSUINT8 calibArea_h_gap;                                // Calibration Area Height ����

	HSUINT8 Warning_type;									// 0: ���� ��� 1: ��� �ð� ����
	HSUINT8 Warning_time;									// Warning_type �� 1�� �� 0~30���� ����

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
