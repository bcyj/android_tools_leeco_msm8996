/**=============================================================================

@file
scveImageCloning.hpp

@brief
SCVE API Definition for Image Cloning features.

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scveImageCloning Image Cloning
///@brief Defines API for SCVE-Image-Cloning feature
///@ingroup scve
//=============================================================================

#ifndef SCVE_IMAGE_CLONING_HPP
#define SCVE_IMAGE_CLONING_HPP

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

//------------------------------------------------------------------------------
/// @brief
///    Error codes specific to Image-Cloning
///
/// @ingroup scveImageCloning
//------------------------------------------------------------------------------
enum StatusCodes_ImageCloning
{
   /// Returned by Init function if the dimesnsions of Object and Mask dont match.
   SCVE_IMAGECLONING_ERROR_DIMENSION_MISMATCH        = SCVE_IMAGECLONING_ERROR + 1,

   /// Returned by Run functions if the session's 'Init' is not called
   SCVE_IMAGECLONING_ERROR_SESSION_NOT_INITED        = SCVE_IMAGECLONING_ERROR + 2,

   /// Returned by Run_Async function if the there are too many Run_Async calls that are still pending.
   SCVE_IMAVECLONING_ERROR_QUEUE_FULL                = SCVE_IMAGECLONING_ERROR + 3,
};

//------------------------------------------------------------------------------
/// @brief
///    Definition of the Image-Cloning's Callback function corresponding to the
///    ImageCloning::Run_Async function.
///
/// @ingroup scveImageCloning
//------------------------------------------------------------------------------
typedef Status (*ImageCloningCB)(Status status, Image* pTarget, void* pSessionUserData, void* pTaskUserData);

//------------------------------------------------------------------------------
/// @brief
///    SCVE::ImageCloning class that implements SCVE's Image Cloning feature.
///
/// @details
///    -
///
/// @ingroup scveImageCloning
//------------------------------------------------------------------------------
class SCVE_API ImageCloning
{
public:
   // @brief
   // instantiate ImageCloning
   // @param fun
   //             callback function
   // @param userData
   //             anything user want to preverse
   static ImageCloning* newInstance(Context* pScveContext, ImageCloningCB cbFunc = NULL, void* pSessionUserData = NULL);

   static Status deleteInstance(ImageCloning *pCloning);

   // @brief
   // Method to initialize object image and mask.
   // @param object
   //            Object to be cloned, which is a 4-channel image
   // @param mask
   //            Mask of the object image that is a 4-channel image
   //            Green is acceptable for mask, e.g. for RGBA image, the value of mask pixel is 0x00FF00FF.
   virtual Status Init( const Image* pObject, const Image* pMask, uint32_t bIsMaskFilled = 0 ) = 0;

   // @brief
   // Method of sync call.
   // @param target
   //            target image that is a 4-channel image. The size of target image should be as same as the object and mask image.
   virtual Status Run_Sync(Image* pTarget) = 0;

   // @brief
   // Method of async call.
   // @param target
   //            target image that is a 4-channel image. The size of target image should be as same as the object and mask image.
   virtual Status Run_Async(Image* pTarget, void* pTaskUserData = NULL) = 0;

protected:
   // @brief
   // destructor
   virtual ~ImageCloning( );
};

} //namespace SCVE

#endif //SCVE_IMAGE_CLONING_HPP

