/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include "camera.h"
#include "camera_dbg.h"

// Inplace Routine to convert YUV420sp-YCrCb image to YV12 image
int yuv_convert_ycrcb420sp_to_yv12(yuv_image_type* yuvStructPtr )
{
    unsigned int   stride = yuvStructPtr->dx; // Assume 16 aligned
    unsigned int   y_size = stride *yuvStructPtr->dy;
    unsigned int   c_size = (stride/2) * (yuvStructPtr->dy/2);
    unsigned char* chroma = (unsigned char *) (yuvStructPtr->imgPtr +  y_size);
    unsigned int   tempBufSize = c_size * 2;
    unsigned char* tempBuf = (unsigned char*) malloc (tempBufSize);
    unsigned int   i = 0;

    if(tempBuf == NULL) {
        CDBG_ERROR("Failed to allocate temporary buffer");
        return -ENOMEM;
    }

    /* copy into temp buffer */
    unsigned char * t1 = chroma;
    unsigned char * t2 = tempBuf;

#ifdef __ARM_HAVE_NEON
    for(i=0; i < (tempBufSize>>5); i++) {
        __asm__ __volatile__ (
                             "vldmia %0!, {d0 - d3} \n"
                             "vstmia %1!, {d0 - d3} \n"
                             :"+r"(t1), "+r"(t2)
                             :
                             :"memory","d0","d1","d2","d3"
                             );
    }
#else
    memcpy(tempBuf, chroma, tempBufSize);
#endif

    /* interleave */
    t1 = chroma;
    t2 = t1 + tempBufSize/2;
    unsigned char * t3 = tempBuf;
#ifdef __ARM_HAVE_NEON
    for(i=0; i < (tempBufSize/2)>>3; i++) {
    __asm__ __volatile__ (
                         "vld2.u8 {d0, d1}, [%2]! \n"
                         "vst1.u8 d0, [%0]! \n"
                         "vst1.u8 d1, [%1]! \n"
                         :"+r"(t1), "+r"(t2), "+r"(t3)
                         :
                         :"memory","d0","d1"
                         );
    }
#else
    for(i=0; i<tempBufSize/2; i++)
    {
        chroma[i] = tempBuf[2*i];
        chroma[i+tempBufSize/2] = tempBuf[2*i+1];
    }
#endif
    if(tempBuf)
        free(tempBuf);

    return 0;
}


// YUV420sp - CrCb to YV12 conversion routine , which take two arguments
int yuv_convert_ycrcb420sp_to_yv12_ver2(yuv_image_type* yuvStructPtr ,
                                                          yuv_image_type* yuvOutStructPtr)
{
    int k,l;
    if(yuvStructPtr ==NULL || yuvOutStructPtr==NULL )
        return -EINVAL;

    yuvOutStructPtr->dx = yuvStructPtr->dx;
    yuvOutStructPtr->dy = yuvStructPtr->dy;

    if(yuvOutStructPtr->imgPtr == NULL)
        return -EINVAL;

    unsigned char* pInputcbcrOff = yuvStructPtr->imgPtr+(yuvStructPtr->dx*yuvStructPtr->dy);
    unsigned char* pOutputcrOff = yuvOutStructPtr->imgPtr +
                                                (yuvOutStructPtr->dx * yuvOutStructPtr->dy);
    unsigned char* pOutputcbOff = yuvOutStructPtr->imgPtr +
                                                 (yuvOutStructPtr->dx* yuvOutStructPtr->dy)+
                               ((CEILING16(yuvOutStructPtr->dx/2) * yuvOutStructPtr->dy/2));
#ifdef __ARM_HAVE_NEON
    unsigned char* temp1 = yuvOutStructPtr->imgPtr;
    unsigned char* temp2 = yuvStructPtr->imgPtr;
    for (k=0; k < (yuvStructPtr->dx*yuvStructPtr->dy)>>5; k++)
    {
        __asm__ __volatile__ (
                             "vld1.u8 {d0-d3}, [%0]! \n"
                             "vst1.u8 {d0-d3}, [%1]! \n"
                             : "+r"(temp2), "+r"(temp1)
                             :
                             :"memory","d0","d1","d2","d3"
                             );

    }
    for(k=0;k < (yuvStructPtr->dx*yuvStructPtr->dy)/4 >>4; k++)
    {
        __asm__ __volatile__ (
                             "vld4.u8 {d0-d3}, [%0]! \n"
                             "vst2.u8 {d0,d2}, [%1]! \n"
                             "vst2.u8 {d1,d3}, [%2]! \n"
                             :"+r"(pInputcbcrOff), "+r"(pOutputcrOff), "+r"(pOutputcbOff)
                             :
                             :"memory","d0","d1","d2","d3"
                             );
    }
#else
    memcpy(yuvOutStructPtr->imgPtr,yuvStructPtr->imgPtr,(yuvStructPtr->dx*yuvStructPtr->dy));
    for(k=0,l=0;l < (yuvStructPtr->dx*yuvStructPtr->dy)/4; )
    {
        pOutputcrOff[l]=pInputcbcrOff[k]; //v
        pOutputcbOff[l]=pInputcbcrOff[k+1];//u
        k=k+2;
        l++;
    }

#endif
    return 0;
}
