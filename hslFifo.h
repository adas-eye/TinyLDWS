/***************************************************/
/*       QUE Functions                             */
/***************************************************/
#define ENABLE_SUMQ
#define QMAX 8
typedef struct {
    int headptr, tailptr;
    int que[QMAX];
    int qSize;
    int qMask;
#ifdef ENABLE_SUMQ
    int qSum;
    int qNum;
#endif
} HS_STRUCT_QUE;

int  hslInitQ(HS_STRUCT_QUE *que, int queSize);

int  hslIsFullQ(HS_STRUCT_QUE *que);
int  hslPutQ(HS_STRUCT_QUE *que, int val);
void hslPushQ(HS_STRUCT_QUE *que, int val);

int  hslIsEmptyQ(HS_STRUCT_QUE *que);
int  hslGetQ(HS_STRUCT_QUE *que, int *val);

int  hslSeeQHead(HS_STRUCT_QUE *que);
int  hslSeeQTail(HS_STRUCT_QUE *que);

#ifndef ENABLE_SUMQ
int  hslGetSumQ(HS_STRUCT_QUE *que);
int  hslGetNumQ(HS_STRUCT_QUE *que);
#endif
