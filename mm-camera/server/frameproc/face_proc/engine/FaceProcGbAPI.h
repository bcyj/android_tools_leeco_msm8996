
/**
 *  Gaze Blink Estimation Library API
 */
#ifndef FACEPROCGBAPI_H__
#define FACEPROCGBAPI_H__

#define FACEPROC_API

#include    "FaceProcDef.h"


#ifndef FACEPROC_DEF_HGAZEBLINK
#define FACEPROC_DEF_HGAZEBLINK
typedef VOID * HGAZEBLINK;    /* Gaze Blink Estimation handle       */
#endif /* FACEPROC_DEF_HGAZEBLINK */

#ifndef FACEPROC_DEF_HGBRESULT
#define FACEPROC_DEF_HGBRESULT
typedef VOID * HGBRESULT;     /* Gaze Blink Estimation Result handle */
#endif /* FACEPROC_DEF_HGBRESULT */

#ifndef FACEPROC_DEF_HPTRESULT
#define FACEPROC_DEF_HPTRESULT
typedef VOID * HPTRESULT;
#endif /* FACEPROC_DEF_HPTRESULT */

/*----- Index of Facial parts -----*/
enum GB_PTPOINT {
    GB_PTPOINT_LEFT_EYE_IN = 0,     /* Left eye inside        */
    GB_PTPOINT_LEFT_EYE_OUT,        /* Left eye outside       */
    GB_PTPOINT_RIGHT_EYE_IN,        /* Right eye inside       */
    GB_PTPOINT_RIGHT_EYE_OUT,       /* Right eye outside      */
    GB_PTPOINT_MAX                  /* Number of Facial parts */
};

#ifdef  __cplusplus
extern "C" {
#endif


/**********************************************************/
/* Get Version                                            */
/**********************************************************/
/* Get Version */
FACEPROC_API INT32        FACEPROC_GB_GetVersion(UINT8 *pucMajor, UINT8 *pucMinor);


/**********************************************************/
/* Create/Delete Handle                                   */
/**********************************************************/
/* Create/Delete Gaze Blink Estimation handle */
FACEPROC_API HGAZEBLINK   FACEPROC_GB_CreateHandle(void);
FACEPROC_API INT32        FACEPROC_GB_DeleteHandle(HGAZEBLINK hGB);

/* Create/Delete Gaze Blink Estimation result handle */
FACEPROC_API HGBRESULT    FACEPROC_GB_CreateResultHandle(void);
FACEPROC_API INT32        FACEPROC_GB_DeleteResultHandle(HGBRESULT hGbResult);

/**********************************************************/
/* Set Facial Parts Position                              */
/**********************************************************/
/* Set facial parts postion */
FACEPROC_API INT32        FACEPROC_GB_SetPoint(HGAZEBLINK hGB, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[],
                                       INT32 nUpDown, INT32 nLeftRight);

/* Set facial parts position from PT result handle */
FACEPROC_API INT32        FACEPROC_GB_SetPointFromHandle(HGAZEBLINK hGB, HPTRESULT hPtResult);

/**********************************************************/
/* Gaze Blink Estimation                                  */
/******************************************************** */
/* Execute Gaze Blink Estimation */
FACEPROC_API INT32        FACEPROC_GB_Estimate(HGAZEBLINK hGB, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HGBRESULT hGbResult);

/**********************************************************/
/* Get Gaze Blink Estimation Result                       */
/**********************************************************/
/* Get the estimation result for blink */
FACEPROC_API INT32        FACEPROC_GB_GetEyeCloseRatio(HGBRESULT hGbResult, INT32 *pnCloseRatioLeftEye, INT32 *pnCloseRatioRightEye);
/* Get the estimation result for gaze */
FACEPROC_API INT32        FACEPROC_GB_GetGazeDirection(HGBRESULT hGbResult, INT32 *pnGazeLeftRight, INT32 *pnGazeUpDown);


#ifdef  __cplusplus
}
#endif

#endif /* FACEPROCGBAPI_H__ */
