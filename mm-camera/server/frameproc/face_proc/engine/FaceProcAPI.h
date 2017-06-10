
/* 
    FACEPROC_SDK Library API
*/
#ifndef FACEPROCCOMAPI_H__
#define FACEPROCCOMAPI_H__

#define FACEPROC_API
#include "FaceProcDef.h"
#include "DetectionInfo.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*----- Get Version -----*/
FACEPROC_API INT32      FACEPROC_GetVersion(UINT8 *pbyMajor, UINT8 *pbyMinor);
/*----- Get Version Information -----*/
FACEPROC_API INT32 FACEPROC_CO_GetVersionInfo(UINT8 aucVerInfo[256]);

/*----- Memory Management -----*/
FACEPROC_API INT32      FACEPROC_SetWMemoryArea(VOID *pMemoryAddr, UINT32 unSize);                      /* Work Memory Area Setting */
FACEPROC_API INT32      FACEPROC_SetBMemoryArea(VOID *pMemoryAddr, UINT32 unSize);                      /* Backup Memory Area Setting */
FACEPROC_API INT32      FACEPROC_FreeWMemoryArea(void);             /* Release Work Memory Area */
FACEPROC_API INT32      FACEPROC_FreeBMemoryArea(void);             /* Release Backup Memory Area */

/*----- BGR Data --> Gray Data -----*/
FACEPROC_API INT32 FACEPROC_ImgConvertBGRtoGRAY(
    RAWIMAGE    *pSrcImage,         /* (I)  BGR Image Data      */
    INT32       nWidth,             /* (I)  Image Width         */
    INT32       nHeight,            /* (I)  Image Height        */
    RAWIMAGE    *pGrayImage);       /* (O)  Gray Scale Data     */

/*----- YUV422 Data --> GRAY Data -----*/
FACEPROC_API INT32 FACEPROC_ImgConvertYUV422toGRAY(
    RAWIMAGE    *pSrcImage,         /* (I)  YUV422 Image Data   */
    INT32       nWidth,             /* (I)  Image Width         */
    INT32       nHeight,            /* (I)  Image Height        */
    RAWIMAGE    *pGrayImage);       /* (O)  Gray Scale Data     */

/*----- BGR Data --> YUV422 Data -----*/
FACEPROC_API INT32 FACEPROC_ImgConvertBGRtoYUV422(
    RAWIMAGE    *pSrcImage,         /* (I) BGR Image Data       */
    INT32       nWidth,             /* (I) Image Width          */
    INT32       nHeight,            /* (I) Image Height         */
    RAWIMAGE    *pDstImage);        /* (O) YUV422 Image Data    */

/*----- YUV422 Data --> BGR Data -----*/
FACEPROC_API INT32 FACEPROC_ImgConvertYUV422toBGR(
    RAWIMAGE    *pSrcImage,         /* (I) YUV422 Image Data    */
    INT32       nWidth,             /* (I) Image Width          */
    INT32       nHeight,            /* (I) Image Height         */
    RAWIMAGE    *pDstImage);        /* (O) BGR Image Data       */

/*----- RGBAlpha Data --> GRAY Data -----*/
FACEPROC_API INT32 FACEPROC_ImgConvertRGBALPHAtoGRAY(
    RAWIMAGE    *pSrcImage,         /* (I)  RGBAlpha Image Data */
    INT32       nWidth,             /* (I)  Image Width         */
    INT32       nHeight,            /* (I)  Image Height        */
    RAWIMAGE    *pGrayImage);       /* (O)  Gray Scale Data     */

/*----- Conversion of Detection Information -----*/
FACEPROC_API INT32 FACEPROC_GetRectangleFromInfo(
    DETECTION_INFO *psDetectionInfo,/* (I)  Detection Information */
    POINT *ptLeftTop,               /* (O)  Top-left Corner */
    POINT *ptRightTop,              /* (O)  Top-right Corner */
    POINT *ptLeftBottom,            /* (O)  Bottom-left Corner */
    POINT *ptRightBottom);          /* (O)  Bottom-right Corner */

#ifdef  __cplusplus
}
#endif

#endif  /* FACEPROCCOMAPI_H__ */

