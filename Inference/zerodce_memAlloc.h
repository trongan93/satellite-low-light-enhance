//
// Created by trongan93 on 12/15/22.
//

#ifndef INFERENCE_ZERODCE_MEMALLOC_H
#define INFERENCE_ZERODCE_MEMALLOC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define DCENET_USOPTION    2

#define CMOS_IMGH  1200
#define CMOS_IMGW  1920
#define CMOS_IMGC  3
#define CMOS_DATABUFFER_AMOUNT  10

#define DCENET_DSRATE    12
#define IMGHEIGHT        (CMOS_IMGH)
#define IMGWIDTH         (CMOS_IMGW)
#define IMGCHANNEL       (CMOS_IMGC)
#define DCENET_HEIGHT    (IMGHEIGHT/DCENET_DSRATE)
#define DCENET_WIDTH     (IMGWIDTH/DCENET_DSRATE)
#define DCENET_CHANNEL   32


#define QX     16384
#define QW     16384
#define QB     (QX*QW)
#define QI     1024
#define QA     1024
#define Qtune  257
#define Qus    1024
#define Qusc   (Qus/2)

typedef short QuanType;
typedef int DQuanType;



typedef volatile struct cmosRawData
{
    uint8_t data[CMOS_DATABUFFER_AMOUNT][CMOS_IMGH][CMOS_IMGW*10/8];
} cmosRawData_t;

typedef volatile struct cmosTMData      /* TM: Tone Mapping */
{
    uint8_t data[CMOS_IMGH][CMOS_IMGW];
} cmosTMData_t;

typedef volatile struct cmosRGBData
{
    uint8_t data[CMOS_IMGH][CMOS_IMGW][CMOS_IMGC];
} cmosRGBData_t;


typedef volatile struct qNormImg        /* Normalized */
{
    short data[IMGHEIGHT][IMGWIDTH][IMGCHANNEL];
}qNormImg_t;

typedef volatile struct qNetIO          /* Neural Network Input/Output */
{
    short data[DCENET_HEIGHT][DCENET_WIDTH][IMGCHANNEL];
}qNetIO_t;

typedef volatile struct qNetFeature     /* Neural Network Feature Map */
{
    short data[DCENET_HEIGHT][DCENET_WIDTH][DCENET_CHANNEL];
}qNetFeature_t;

typedef volatile struct qEnhanceParam   /* Estimated Enhancing Parameters */
{
    short data[IMGHEIGHT][IMGWIDTH][IMGCHANNEL];
}qEnhanceParam_t;

typedef volatile struct usBuffer        /* Up Sampling Buffer*/
{
    short data[DCENET_HEIGHT][IMGWIDTH][IMGCHANNEL];
}usBuffer_t;



typedef volatile struct qWConv1st
{
    short data[DCENET_CHANNEL][IMGCHANNEL][3][3];
}qWConv1st_t;
typedef volatile struct qBConv1st
{
    int data[DCENET_CHANNEL];
}qBConv1st_t;


typedef volatile struct qWConv2nd
{
    short data[DCENET_CHANNEL][DCENET_CHANNEL][3][3];
}qWConv2nd_t;
typedef volatile struct qBConv2nd
{
    int data[DCENET_CHANNEL];
}qBConv2nd_t;


typedef volatile struct qWConv3rd
{
    short data[IMGCHANNEL][DCENET_CHANNEL][3][3];
}qWConv3rd_t;
typedef volatile struct qBConv3rd
{
    int data[IMGCHANNEL];
}qBConv3rd_t;


#define ISP_RAWDATA       ((cmosRawData_t*)(0xA0000000))
#define ISP_TMDATA        ((cmosTMData_t*)(0xA1B77400))
#define ISP_DBDATA        ((cmosRGBData_t*)(0xA1DA9C00))	//same as PYTHON_DATA_RGB
#define ISP_WBDATA        ((cmosRGBData_t*)(0xA2441400))
#define ISP_AIISPDATA     ((cmosRGBData_t*)(0xA2AD8C00))
#define AIIP_NORM         ((qNormImg_t*)(0xA3170400))
#define AIIP_NETIO        ((qNetIO_t*)(0xA3E9F400))
#define AIIP_FEATURE1     ((qNetFeature_t*)(0xA3EB6B00))
#define AIIP_FEATURE2     ((qNetFeature_t*)(0xA3FB0B00))
#define AIIP_PARAM        ((qEnhanceParam_t*)(0xA40AAB00))
#define AIIP_USBUFFER     ((qEnhanceParam_t*)(0xA40AAB00))
#define AIIP_CONVW01      ((qWConv1st_t*)(0xA4EF2F00))
#define AIIP_CONVB01      ((qBConv1st_t*)(0xA4EF35C0))
#define AIIP_CONVW02      ((qWConv2nd_t*)(0xA4EF3640))
#define AIIP_CONVB02      ((qBConv2nd_t*)(0xA4EF7E40))
#define AIIP_CONVW03      ((qWConv3rd_t*)(0xA4EF7EC0))
#define AIIP_CONVB03      ((qBConv3rd_t*)(0xA4EF8580))
#endif //INFERENCE_ZERODCE_MEMALLOC_H
