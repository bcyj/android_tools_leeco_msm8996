/**=============================================================================

@file
scveObjectSegmentation.hpp

@brief
Header file for Object Extraction.

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/
#ifndef _SCVE_OBJECT_SEGMENTATION_H_
#define _SCVE_OBJECT_SEGMENTATION_H_

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

typedef enum
{
    SCVE_OBJECT_SEGMENTATION_UNKNOWN = 0, // initial value of the stroke
    SCVE_OBJECT_SEGMENTATION_BACKGROUND = 1, // background label
    SCVE_OBJECT_SEGMENTATION_FOREGROUND = 2 // foreground label
} ObjectSegmentationType;

typedef void (*ObjectSegmentationCB)( Status status,
                                    void* pSessionUserData,
                                    void* pTaskUserData);

class SCVE_API ObjectSegmentation
{
public:
   // @brief
   // instantiate ObjectSegmentation instance
   // @param cbFunc
   //     callback function
   // @param pSessionUserData
   //     anything user want to preverse
   static ObjectSegmentation* newInstance(Context *pScveContext,
                                        ObjectSegmentationCB cbFunc = NULL,
                                        void *pSessionUserData = NULL);

   // @brief
   // delete ObjectSegmentation instance
   static Status deleteInstance(ObjectSegmentation *pInstance);

   // @brief
   // Method of sync call.
   // @param pSrc
   //     Input image. 4-channel rgba image
   virtual Status init(const Image *pSrc) = 0;

   // @brief
   // Method of sync call.
   // @param pStroke
   //     Input stroke image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pMask
   //     Output mask image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   virtual Status Run_Sync( const Image *pStroke,
                                  Image *pMask) = 0;

   // @brief
   // Method of async call.
   // @param pStroke
   //     Input stroke image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pMask
   //     Output mask image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   virtual Status Run_Async( const Image *pStroke,
                                   Image *pMask,
                             void* pTaskUserData = NULL) = 0;

protected:
   // @brief
   // destructor
   virtual ~ObjectSegmentation();
};

}

#endif //_SCVE_IMAGE_OBJECT_SEGMENTATION_H_

