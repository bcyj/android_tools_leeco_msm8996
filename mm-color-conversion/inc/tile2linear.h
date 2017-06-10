#ifndef TILE2LINEAR_H
#define TILE2LINEAR_H

/**=============================================================================

@file
   tile2linear.h

@brief
   Tile to NV12/NV21 color conversion public API

Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//------------------------------------------------------------------------------
/// @brief
///   Defines error type for tile to NV12/NV21 color conversion
//------------------------------------------------------------------------------
typedef enum { 
    tileToNv12_ERROR_NONE,
    tileToNv12_ERROR_INSUFFICIENT_RESOURCES    
} tileToNv12_ErrorType;


/****************************************************************************
*
** FUNCTION: tileToNv12Init()
*
*  Function to initalize tileToNv12 convert data structures. 
* 
*  [in] ppTileToNv12Ctx - pointer to the variable holding the
*                         pointer to the context
*  [in] width           - width of the frame
*  [in] height          - height of the frame
* 
* 
****************************************************************************/
tileToNv12_ErrorType tileToNv12Init(void ** ppTile2Nv12Ctx,
                                    unsigned int width,
                                    unsigned int height);

/****************************************************************************
*
** FUNCTION: tileToNv12Deinit()
*
*  Function to de-initalize tileToNv12 convert data structures. 
* 
*  [in] ppTileToNv12Ctx - pointer to the variable holding the
*                         pointer to the context
* 
****************************************************************************/
void tileToNv12Deinit(void ** ppTileToNv12Ctx);

/****************************************************************************
*
** FUNCTION: convertTileToNv12()
*
*  Function to convert from tiled YUV format to linear YUV
*  format (NV12). 
* [in] pSrcLuma       - Src luma buffer pointer
* [in] pSrcCbCr       - Src CbCr buffer pointer
* [in] pDstLuma       - Dest luma buffer pointer
* [in] pDstCrCb       - Dest CrCb buffer pointer
* [in] iWidth         - width of the yuv frame
* [in] iHeight        - Height of the yuv buffer
* [in] dstLumaStride  - stride value of dest luma buffer
* [in] dstCbCrStride  - stride value of the dest Chroma buffer
* [in] ppTileToNv12Ctx- pointer to the variable holding the
*                       pointer to the context
* 
****************************************************************************/
tileToNv12_ErrorType convertTileToNv12(unsigned char* pSrcLuma,
                                       unsigned char* pSrcCbCr,
                                       unsigned char* pDstLuma,
                                       unsigned char* pDstCbCr,
                                       unsigned int   iWidth,
                                       unsigned int   iHeight,
                                       int            dstLumaStride,
                                       int            dstCbCrStride,
                                       void **        ppTileToNv12Ctx);

/****************************************************************************
*
** FUNCTION: convertTileToNv21()
*
*  Function to convert from tiled YUV format to linear YVU
*  format (NV21). 
* [in] pSrcLuma       - Src luma buffer pointer
* [in] pSrcCbCr       - Src CbCr buffer pointer
* [in] pDstLuma       - Dest luma buffer pointer
* [in] pDstCrCb       - Dest CrCb buffer pointer
* [in] iWidth         - width of the yuv frame
* [in] iHeight        - Height of the yuv buffer
* [in] dstLumaStride  - stride value of dest luma buffer
* [in] dstCbCrStride  - stride value of the dest Chroma buffer
* [in] ppTileToNv12Ctx- pointer to the variable holding the
*                       pointer to the context
* 
****************************************************************************/
tileToNv12_ErrorType convertTileToNv21(unsigned char* pSrcLuma,
                                       unsigned char* pSrcCbCr,
                                       unsigned char* pDstLuma,
                                       unsigned char* pDstCbCr,
                                       unsigned int   iWidth,
                                       unsigned int   iHeight,
                                       int            dstLumaStride,
                                       int            dstCbCrStride,
                                       void **        ppTileToNv12Ctx);

#endif 
