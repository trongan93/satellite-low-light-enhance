//
// Created by trongan93 on 12/15/22.
//

#ifndef INFERENCE_ZERODCE_H
#define INFERENCE_ZERODCE_H
#include "zerodce_memAlloc.h"

void qNorm();
void qDownSample();
void qConv1st();
void qConv2nd();
void qConv3rd();
void qUpSample();
void qEnhance();


void qLoadParam();	//init
void qDCENet();


#endif //INFERENCE_ZERODCE_H
