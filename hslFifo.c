#include "hslFifo.h"
#include "hslMath.h"

/***************************************************/
/*       QUE Functions                             */
/***************************************************/

int  hslInitQ(HS_STRUCT_QUE *que, int queSize)
{
    int bitCnt;

    if (queSize>QMAX || queSize<2) return 0;
    
    bitCnt = hslGetBitResolution(queSize-1);
    que->qSize = 1<<bitCnt;
    que->qMask = (que->qSize)-1;
    que->headptr = que->tailptr = 0;
#ifdef ENABLE_SUMQ    
    que->qSum = 0;
    que->qNum = 0;
#endif    

    return 1;
}

int  hslIsFullQ(HS_STRUCT_QUE *que)
{
    unsigned int diff = (unsigned int)(que->headptr - que->tailptr);

    if (diff>=que->qSize) 
        return 1;
    else 
        return 0;
}

int  hslPutQ(HS_STRUCT_QUE *que, int val)
{
    if (hslIsFullQ(que)) 
    {
        return 0;
    }
    else
    {
        que->que[(que->headptr++)&(que->qMask)] = val;
#ifdef ENABLE_SUMQ    
        que->qSum += val;
        que->qNum ++;
#endif        
        return 1;
    }
}

void hslPushQ(HS_STRUCT_QUE *que, int val)
{
    if (hslIsFullQ(que)) 
    {
#ifdef ENABLE_SUMQ    
        que->qSum -= que->que[(que->tailptr)&(que->qMask)];
#endif
        que->tailptr++;        
        que->que[(que->headptr++)&(que->qMask)] = val;
        que->qSum += val;
    }
    else
    {
        que->que[(que->headptr++)&(que->qMask)] = val;
#ifdef ENABLE_SUMQ    
        que->qSum += val;
        que->qNum ++;
#endif
    }
}

int  hslIsEmptyQ(HS_STRUCT_QUE *que)
{
    unsigned int diff = (unsigned int)(que->headptr - que->tailptr);

    if (diff)
        return 0;
    else
        return 1;
}

int  hslGetQ(HS_STRUCT_QUE *que, int *val)
{
    if (hslIsEmptyQ(que)) 
    {
        return 0;
    }
    else
    {
        *val = que->que[(que->tailptr++)&(que->qMask)];
#ifdef ENABLE_SUMQ
        que->qSum -= *val;
        que->qNum --;
#endif
        return 1;
    }
}

int  hslSeeQHead(HS_STRUCT_QUE *que)
{
    int val=que->que[(que->headptr-1)&(que->qMask)];
    return val;
}

int  hslSeeQTail(HS_STRUCT_QUE *que)
{
    int val = que->que[(que->tailptr)&(que->qMask)];
    return val;
}


#ifndef ENABLE_SUMQ
int  hslGetSumQ(HS_STRUCT_QUE *que)
{
    int i, sum=0;

    for (i=que->tailptr;i<que->headptr;i++)
    {
        sum += que->que[(i&que->qMask)];
    }

    return sum;
}

int  hslGetNumQ(HS_STRUCT_QUE *que)
{
    unsigned int diff = (unsigned int)(que->headptr - que->tailptr);

    return (int)diff;
}
#endif
