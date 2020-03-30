// Select Video Image Source

/////////////////////////////////////////////////////
/* Configuration on LDWS and Camera of Lucy SDK HD */
/////////////////////////////////////////////////////

// Regarding hsaLengthDetect algorithm
#define LANE_Y_GAP          2 //10 //jdy   //kys 3

// Regarding DecideTwoLane Function
#define INIT_LEFT_ANGLE     36      /* The initial angle of left lane when reset */
#define INIT_RIGHT_ANGLE    144     /* The initial angle of right lane when reset */

#define LANE_MIN			6	//6 jdy
#define LANE_MAX            10		//10 jdy for high pass  

#define WARNING_ANGLE_LEFT  65      //65//68    // jdy     /* The left angle for determining the LANE DEPARTURE */
#define WARNING_ANGLE_RIGHT 115   //115//112   //jdy      /* The right angle for determining the LANE DEPARTURE */

#define ERROR_STEP_XLOC       15		//org 15 //kys 20  //jdy
#define MARGIN_RHO			10      /* Margin out of the formula which makes a decision if it's a right one or not */

