/**=============================================================================

@file
scveImageRemoval.hpp

@brief
SCVE API Definition for Image Removal feature.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scveImageRemoval Image Removal
///@brief Defines API for SCVE-Image-Removal feature
///@ingroup scve
//=============================================================================

#ifndef SCVE_IMAGE_REMOVAL_HPP
#define SCVE_IMAGE_REMOVAL_HPP

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

#define SCVE_IMGREM_DEFAULT_PATCH_SIZE 17

enum StatusCodes_ImageRemoval
{
   SCVE_IMAGEREMOVAL_ERROR_DIMENSION_MISMATCH = SCVE_IMAGEREMOVAL_ERROR + 1,
   SCVE_IMAGEREMOVAL_ERROR_REGION_NOT_FOUND   = SCVE_IMAGEREMOVAL_ERROR + 2,
   SCVE_IMAVEREMOVAL_ERROR_QUEUE_FULL         = SCVE_IMAGEREMOVAL_ERROR + 3,
};

typedef void (*ImageRemovalCB)( Status status, uint32_t bIsFinished, Image* pImage,
                                Rect sRect, void* pSessionUserData, void* pTaskUserData);

class SCVE_API ImageRemoval
{
public:
   // @brief
   // instantiate ImageRemoval
   // @param cbFunc
   //             callback function
   // @param pSessionUserData
   //             anything user want to preverse
   static ImageRemoval* newInstance(Context *pScveContext, ImageRemovalCB cbFunc,
                                    void *pSessionUserData = NULL);

   static Status deleteInstance(ImageRemoval *pRemoval);

   // @brief
   // Method of sync call.
   // @param image
   //            Input and also the output image, which is a 4-channel image
   // @param mask
   //            Mask of the input image that is a 4-channel image
   //            Green is acceptable for mask, e.g. for RGBA image, the value of mask pixel is 0x00FF00FF.
   virtual Status Run_Sync(Image *pImage, const Image *pMask, Rect *pRect,
                           int32_t patchSize = SCVE_IMGREM_DEFAULT_PATCH_SIZE) = 0;
   // @brief
   // Method of async call.
   // @param image
   //            Input and also the output image, which is a 4-channel image
   // @param mask
   //            Mask of the input image that is a 4-channel image
   //            Green is acceptable for mask, e.g. for RGBA image, the value of mask pixel is 0x00FF00FF.
   virtual Status Run_Async(Image *pImage, const Image *pMask,
                            int32_t patchSize = SCVE_IMGREM_DEFAULT_PATCH_SIZE,
                            void *pTaskUserData = NULL) = 0;

   // @brief
   // Method to stop the runAsync
   //TODO nkk - What's the semantic of this function? What if there are more than one tasks queued?
   virtual Status Stop_Async() = 0;

protected:
   // @brief
   // destructor
   virtual ~ImageRemoval();
};

} //namespace SCVE

#endif //SCVE_IMAGE_REMOVAL_HPP

