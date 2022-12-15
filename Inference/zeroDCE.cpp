//
// Created by trongan93 on 12/15/22.
//
#include "zeroDCE.h"
#include "qZeroDCE_Weight.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
//#include <cstring>
int sigmoidMapping(int x)
{
    if(x >= -QX && x <= QX)
        return (x*473)>>14;
    else if((x >= -2*QX && x < -QX) || (x <= 2*QX && x > QX))
        return (x*423)>>14;
    else if((x >= -3*QX && x < -2*QX) || (x <= 3*QX && x > 2*QX))
        return (x*342)>>14;
    else if((x >= -4*QX && x < -3*QX) || (x <= 4*QX && x > 3*QX))
        return (x*273)>>14;
    else
        return (x>0?QA:-QA);
}


void qNorm()
{
    uint8_t *s_src = (uint8_t*)&ISP_DBDATA->data[0][0][0];
    uint8_t *d_src = s_src + IMGHEIGHT * IMGWIDTH * IMGCHANNEL;
    short *s_dst = (short*)&AIIP_NORM->data[0][0][0];

    while(s_src<d_src)
    {
        *s_dst = (short)(((int)(*s_src) * Qtune)>>2);
        s_src++;
        s_dst++;
    }
}

void qDownSample()
{
    for(int h = 0; h < DCENET_HEIGHT; ++h)
    {
        for(int w = 0; w < DCENET_WIDTH; ++w)
        {
            AIIP_NETIO->data[h][w][0] = AIIP_NORM->data[h*DCENET_DSRATE][w*DCENET_DSRATE][0];
            AIIP_NETIO->data[h][w][1] = AIIP_NORM->data[h*DCENET_DSRATE][w*DCENET_DSRATE][1];
            AIIP_NETIO->data[h][w][2] = AIIP_NORM->data[h*DCENET_DSRATE][w*DCENET_DSRATE][2];
        }
    }
}


void qConv1st()
{
    for(int h = 0; h < DCENET_HEIGHT; ++h)
    {
        for(int w = 0; w < DCENET_WIDTH; ++w)
        {
            for(int cout = 0; cout < DCENET_CHANNEL; ++cout)
            {
                int sum = 0;
                for(int cin = 0; cin < IMGCHANNEL; ++cin)
                {
                    for(int kh = -1; kh <= 1; ++kh)
                    {
                        for(int kw = -1; kw <= 1; ++kw)
                        {
                            if(((h+kh) >=0) && ((w+kw) >=0) && ((h+kh) < DCENET_HEIGHT) && ((w+kw) < DCENET_WIDTH))
                                sum += AIIP_NETIO->data[h+kh][w+kw][cin] * AIIP_CONVW01->data[cout][cin][kh+1][kw+1];
                        }
                    }
                }
                sum += AIIP_CONVB01->data[cout];
                sum = (sum>0)?sum:0;
                AIIP_FEATURE1->data[h][w][cout] = sum >> 14;
            }
        }
    }
}

void qConv2nd()
{
    for(int h = 0; h < DCENET_HEIGHT; ++h)
    {
        for(int w = 0; w < DCENET_WIDTH; ++w)
        {
            for(int cout = 0; cout < DCENET_CHANNEL; ++cout)
            {
                int sum = 0;
                for(int cin = 0; cin < DCENET_CHANNEL; ++cin)
                {
                    for(int kh = -1; kh <= 1; ++kh)
                    {
                        for(int kw = -1; kw <= 1; ++kw)
                        {
                            if(((h+kh) >=0) && ((w+kw) >=0) && ((h+kh) < DCENET_HEIGHT) && ((w+kw) < DCENET_WIDTH))
                                sum += AIIP_FEATURE1->data[h+kh][w+kw][cin] * AIIP_CONVW02->data[cout][cin][kh+1][kw+1];
                        }
                    }
                }
                sum += AIIP_CONVB02->data[cout];
                sum = (sum>0)?sum:0;
                AIIP_FEATURE2->data[h][w][cout] = sum >> 14;
            }
        }
    }
}

void qConv3rd()
{
    for(int h = 0; h < DCENET_HEIGHT; ++h)
    {
        for(int w = 0; w < DCENET_WIDTH; ++w)
        {
            for(int cout = 0; cout < IMGCHANNEL; ++cout)
            {
                int sum = 0;
                for(int cin = 0; cin < DCENET_CHANNEL; ++cin)
                {
                    for(int kh = -1; kh <= 1; ++kh)
                    {
                        for(int kw = -1; kw <= 1; ++kw)
                        {
                            if(((h+kh) >=0) && ((w+kw) >=0) && ((h+kh) < DCENET_HEIGHT) && ((w+kw) < DCENET_WIDTH))
                                sum += (AIIP_FEATURE2->data[h+kh][w+kw][cin] + AIIP_FEATURE1->data[h+kh][w+kw][cin]) * AIIP_CONVW03->data[cout][cin][kh+1][kw+1];
                        }
                    }
                }
                sum += AIIP_CONVB03->data[cout];
                AIIP_NETIO->data[h][w][cout] = (short)sigmoidMapping(sum >> 14);
            }
        }
    }
}



void qUpSample()
{
#if(DCENET_USOPTION == 0)
    for(int h = 0; h < IMGHEIGHT; ++h)
    {
        for(int w = 0; w < IMGWIDTH; ++w)
        {
            AIIP_PARAM->data[h][w][0] = AIIP_NETIO->data[(int)(h/DCENET_DSRATE)][(int)(w/DCENET_DSRATE)][0];
            AIIP_PARAM->data[h][w][1] = AIIP_NETIO->data[(int)(h/DCENET_DSRATE)][(int)(w/DCENET_DSRATE)][1];
            AIIP_PARAM->data[h][w][2] = AIIP_NETIO->data[(int)(h/DCENET_DSRATE)][(int)(w/DCENET_DSRATE)][2];
        }
    }

#elif(DCENET_USOPTION == 1)
    for(int h = 0; h < DCENET_HEIGHT; ++h)
    {
        for(int w = 0; w < IMGWIDTH; ++w)
        {
            int sW = (Qus*w + Qusc) / DCENET_DSRATE - Qusc;
            if(sW >= 0 && sW < (Qus* (DCENET_WIDTH-1)))
            {
                int i = sW / Qus;
                int j = i + 1;
                AIIP_USBUFFER->data[h][w][0] = ((sW - i*Qus) * (AIIP_NETIO->data[h][j][0] - AIIP_NETIO->data[h][i][0]) >> 10) + AIIP_NETIO->data[h][i][0];
                AIIP_USBUFFER->data[h][w][1] = ((sW - i*Qus) * (AIIP_NETIO->data[h][j][1] - AIIP_NETIO->data[h][i][1]) >> 10) + AIIP_NETIO->data[h][i][1];
                AIIP_USBUFFER->data[h][w][2] = ((sW - i*Qus) * (AIIP_NETIO->data[h][j][2] - AIIP_NETIO->data[h][i][2]) >> 10) + AIIP_NETIO->data[h][i][2];
            }
            else
            {
            	AIIP_USBUFFER->data[h][w][0] = AIIP_NETIO->data[h][(int)(w/DCENET_DSRATE)][0];
            	AIIP_USBUFFER->data[h][w][1] = AIIP_NETIO->data[h][(int)(w/DCENET_DSRATE)][0];
            	AIIP_USBUFFER->data[h][w][2] = AIIP_NETIO->data[h][(int)(w/DCENET_DSRATE)][0];
            }
        }
    }

    for(int h = 0; h < IMGHEIGHT; ++h)
    {
        int sH = (Qus*h + Qusc) / DCENET_DSRATE - Qusc;
        if(sH >= 0 && sH < (Qus* (DCENET_HEIGHT-1)))
        {
            int i = sH / Qus;
            int j = i + 1;
            for(int w = 0; w < IMGWIDTH; ++w)
            {
            	AIIP_PARAM->data[h][w][0] = ((sH - i*Qus) * (AIIP_USBUFFER->data[j][w][0] - AIIP_USBUFFER->data[i][w][0]) >> 10) + AIIP_USBUFFER->data[i][w][0];
            	AIIP_PARAM->data[h][w][1] = ((sH - i*Qus) * (AIIP_USBUFFER->data[j][w][1] - AIIP_USBUFFER->data[i][w][1]) >> 10) + AIIP_USBUFFER->data[i][w][1];
            	AIIP_PARAM->data[h][w][2] = ((sH - i*Qus) * (AIIP_USBUFFER->data[j][w][2] - AIIP_USBUFFER->data[i][w][2]) >> 10) + AIIP_USBUFFER->data[i][w][2];
            }
        }
        else
        {
            for(int w = 0; w < IMGWIDTH; ++w)
            {
            	AIIP_PARAM->data[h][w][0] = AIIP_USBUFFER->data[(int)(h/DCENET_DSRATE)][w][0];
            	AIIP_PARAM->data[h][w][1] = AIIP_USBUFFER->data[(int)(h/DCENET_DSRATE)][w][1];
            	AIIP_PARAM->data[h][w][2] = AIIP_USBUFFER->data[(int)(h/DCENET_DSRATE)][w][2];
            }
        }
    }

#else
    int coef[12] = {42, 128, 213, 298, 384, 469, 554, 640, 725, 810, 896, 981};
	for(int h = 0; h < DCENET_HEIGHT; ++h)
	{
		int wi = 0;
		for(int d = 0; d < DCENET_DSRATE/2; ++d, ++wi)
		{
			AIIP_USBUFFER->data[h][wi][0] = AIIP_NETIO->data[h][0][0];
			AIIP_USBUFFER->data[h][wi][1] = AIIP_NETIO->data[h][0][1];
			AIIP_USBUFFER->data[h][wi][2] = AIIP_NETIO->data[h][0][2];
		}
		for(int w = 1; w < DCENET_WIDTH; ++w)
		{
			for(int d = 0; d < DCENET_DSRATE; ++d, ++wi)
			{
				AIIP_USBUFFER->data[h][wi][0] = (coef[d] * (AIIP_NETIO->data[h][w][0] - AIIP_NETIO->data[h][w-1][0]) >> 10) + AIIP_NETIO->data[h][w-1][0];
				AIIP_USBUFFER->data[h][wi][1] = (coef[d] * (AIIP_NETIO->data[h][w][1] - AIIP_NETIO->data[h][w-1][1]) >> 10) + AIIP_NETIO->data[h][w-1][1];
				AIIP_USBUFFER->data[h][wi][2] = (coef[d] * (AIIP_NETIO->data[h][w][2] - AIIP_NETIO->data[h][w-1][2]) >> 10) + AIIP_NETIO->data[h][w-1][2];
			}
		}
		for(int d = 0; d < DCENET_DSRATE/2; ++d, ++wi)
		{
			AIIP_USBUFFER->data[h][wi][0] = AIIP_NETIO->data[h][DCENET_WIDTH-1][0];
			AIIP_USBUFFER->data[h][wi][1] = AIIP_NETIO->data[h][DCENET_WIDTH-1][1];
			AIIP_USBUFFER->data[h][wi][2] = AIIP_NETIO->data[h][DCENET_WIDTH-1][2];
		}
	}

	int hi = 0;
	for(int d = 0; d < DCENET_DSRATE/2; ++d, ++hi)
	{
		for(int w = 0; w < IMGWIDTH; ++w)
		{
			AIIP_PARAM->data[hi][w][0] = AIIP_USBUFFER->data[0][w][0];
			AIIP_PARAM->data[hi][w][1] = AIIP_USBUFFER->data[0][w][1];
			AIIP_PARAM->data[hi][w][2] = AIIP_USBUFFER->data[0][w][2];
		}
	}
	for(int h = 1; h < DCENET_HEIGHT; ++h)
	{
		for(int d = 0; d < DCENET_DSRATE; ++d, ++hi)
		{
			for(int w = 0; w < IMGWIDTH; ++w)
			{
				AIIP_PARAM->data[hi][w][0] = (coef[d] * (AIIP_USBUFFER->data[h][w][0] - AIIP_USBUFFER->data[h-1][w][0]) >> 10) + AIIP_USBUFFER->data[h-1][w][0];
				AIIP_PARAM->data[hi][w][1] = (coef[d] * (AIIP_USBUFFER->data[h][w][1] - AIIP_USBUFFER->data[h-1][w][1]) >> 10)  + AIIP_USBUFFER->data[h-1][w][1];
				AIIP_PARAM->data[hi][w][2] = (coef[d] * (AIIP_USBUFFER->data[h][w][2] - AIIP_USBUFFER->data[h-1][w][2]) >> 10)  + AIIP_USBUFFER->data[h-1][w][2];
			}

		}
	}
	for(int d = 0; d < DCENET_DSRATE/2; ++d, ++hi)
	{
		for(int w = 0; w < IMGWIDTH; ++w)
		{
			AIIP_PARAM->data[hi][w][0] = AIIP_USBUFFER->data[DCENET_HEIGHT-1][w][0];
			AIIP_PARAM->data[hi][w][1] = AIIP_USBUFFER->data[DCENET_HEIGHT-1][w][1];
			AIIP_PARAM->data[hi][w][2] = AIIP_USBUFFER->data[DCENET_HEIGHT-1][w][2];
		}
	}
#endif
}

void qEnhance()
{
    short *s = (short*)&AIIP_NORM->data[0][0][0];
    short *sa = (short*)&AIIP_PARAM->data[0][0][0];
    short *d = s + IMGCHANNEL * IMGHEIGHT * IMGWIDTH;
    uint8_t *yd = (uint8_t*)&ISP_AIISPDATA->data[0][0][0];

    int output;
    while(s<d)
    {
        int x_q = (*s) >> 4;
        int a_q = *sa;
        for(int i = 0; i < 8; ++i)
        {
            int x_q2 = x_q * QI;
            int x_q3 = x_q2 * QA;
            x_q3 = x_q3 + a_q * (x_q * x_q - x_q2);
            x_q = (int)(x_q3 / (QI * QA));
        }
        output = x_q/(QI/255);
        output = output>255?255:output;
        *yd = (uint8_t)(output);
        s++;
        sa++;
        yd++;
    }
}


void qDCENet()
{
    printf("Run qNorm");
    qNorm();
    printf("Run qDownSample");
    qDownSample();
    printf("Run qConv1st");
    qConv1st();
    printf("Run qConv2nd");
    qConv2nd();
    printf("Run     qConv3rd();");
    qConv3rd();
    printf("Run qUpSample");
    qUpSample();
    printf("Run qEnhance");
    qEnhance();
}


void qLoadParam()
{
    memcpy((void *) AIIP_CONVW01, conv1_w, 2 * 864);
    memcpy((void *) AIIP_CONVB01, conv1_b, 4 * 32);
    memcpy((void *) AIIP_CONVW02, conv2_w, 2 * 9216);
    memcpy((void *) AIIP_CONVB02, conv2_b, 4 * 32);
    memcpy((void *) AIIP_CONVW03, conv3_w, 2 * 864);
    memcpy((void *) AIIP_CONVB03, conv3_b, 4 * 3);
}
