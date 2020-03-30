#include "hslType.h"
#include "hslGraphics.h"

void hslDrawPoint8(HSUINT8 *ptr, int w, int h, int x, int y, int thick, HSUINT8 c)
{
	register int m, n;

    for (m=0;m<thick;m++)
        for (n=0;n<thick;n++)
            ptr[(y+m)*w+x+n] = c;
}


void hslDrawPoint16(HSUINT16 *ptr, int w, int h, int x, int y, int thick, HSUINT16 c)
{
	register int m, n;

    for (m=0;m<thick;m++)
        for (n=0;n<thick;n++)
            ptr[(y+m)*w+x+n] = c;
}

void hslDrawLine8(HSUINT8 *ptr, int w, int h, int x1, int y1, int x2, int y2, HSUINT8 c)
{
	register int x, y, m;

	if( y1 > y2 )
	{
		m = y1;
		y1 = y2;
		y2 = m;

		m = x1;
		x1 = x2;
		x2 = m;
	}

	//-------------------------------------------------------------------------
	// 수직선인 경우
	//-------------------------------------------------------------------------
	if( x1 == x2 )
	{
		for( y = y1 ; y <= y2 ; y++ )
			ptr[y*w+x1] = c;

		return;
	}

	//-------------------------------------------------------------------------
	// (x1, y1) 에서 (x2, y2)까지 직선 그리기
	//-------------------------------------------------------------------------

	if((y2 - y1)/(x2 - x1))
	{
		for( y = y1 ; y <= y2 ; y++ )
		{
			x = (int)(y - y1)*(x2 - x1)/(y2 - y1) + x1;
			ptr[y*w+x] = c;
		}
	}
	else
	{
		if( x1 > x2 )
		{
			m = x1;
			x1 = x2;
			x2 = m;

			m = y1;
			y1 = y2;
			y2 = m;
		}

		for( x = x1 ; x <= x2 ; x++ )
		{
			y = (x - x1)*(y2 - y1)/(x2 - x1) + y1;
			ptr[y*w+x] = c;
		}
	}
}

void hslDrawLine16(HSUINT16 *ptr, int w, int h, int x1, int y1, int x2, int y2, HSUINT16 c)
{
	register int x, y, m;

	if( y1 > y2 )
	{
		m = y1;
		y1 = y2;
		y2 = m;

		m = x1;
		x1 = x2;
		x2 = m;
	}

	//-------------------------------------------------------------------------
	// 수직선인 경우
	//-------------------------------------------------------------------------
	if( x1 == x2 )
	{
		for( y = y1 ; y <= y2 ; y++ )
			ptr[y*w+x1] = c;

		return;
	}

	//-------------------------------------------------------------------------
	// (x1, y1) 에서 (x2, y2)까지 직선 그리기
	//-------------------------------------------------------------------------

	if((y2 - y1)/(x2 - x1))
	{
		for( y = y1 ; y <= y2 ; y++ )
		{
			x = (int)(y - y1)*(x2 - x1)/(y2 - y1) + x1;
			ptr[y*w+x] = c;
		}
	}
	else
	{
		if( x1 > x2 )
		{
			m = x1;
			x1 = x2;
			x2 = m;

			m = y1;
			y1 = y2;
			y2 = m;
		}

		for( x = x1 ; x <= x2 ; x++ )
		{
			y = (x - x1)*(y2 - y1)/(x2 - x1) + y1;
			ptr[y*w+x] = c;
		}
	}
}

void hslDrawBox8(HSUINT8 *ptr, int w, int h, int x, int y, int bw, int bh, HSUINT8 c)
{
    int i, j;
    int x1, x2, y1, y2;

    x1=x;       y1=y;
    x2=x+bw;    y2=y+bh;

    for (i=y1; i<y2; i++)
    {
        for (j=x1; j<x2; j++)
        {
            ptr[i*w+j] = c;
        }
    }
}

void hslDrawBox16(HSUINT16 *ptr, int w, int h, int x, int y, int bw, int bh, HSUINT16 c)
{
    int i, j;
    int x1, x2, y1, y2;

    x1=x;       y1=y;
    x2=x+bw;    y2=y+bh;

    for (i=y1; i<y2; i++)
    {
        for (j=x1; j<x2; j++)
        {
            ptr[i*w+j] = c;
        }
    }
}
