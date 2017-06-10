
/* 
    Smile Degree Estimation Library API
*/

#ifndef FACEPROCSMAPI_H__
#define FACEPROCSMAPI_H__

#define FACEPROC_API
#include    "FaceProcDef.h"

#ifndef FACEPROC_DEF_HSMILE
#define FACEPROC_DEF_HSMILE
typedef void *  HSMILE;         /* Smile Degree Estimation handle */
#endif /* FACEPROC_DEF_HSMILE */

#ifndef FACEPROC_DEF_HSMRESULT
#define FACEPROC_DEF_HSMRESULT
typedef void *  HSMRESULT;      /* Smile Degree Estimation result handle */
#endif /* FACEPROC_DEF_HSMRESULT */

#ifndef FACEPROC_DEF_HPTRESULT
#define FACEPROC_DEF_HPTRESULT
typedef void *  HPTRESULT;      /* Facial Parts Detection result handle */
#endif /* FACEPROC_DEF_HPTRESULT */


/*----- Index of Facial parts -----*/
enum SM_PTPOINT {
    SM_PTPOINT_LEFT_EYE_IN = 0,     /* Left eye inside        */
    SM_PTPOINT_LEFT_EYE_OUT,        /* Left eye outside       */
    SM_PTPOINT_RIGHT_EYE_IN,        /* Right eye inside       */
    SM_PTPOINT_RIGHT_EYE_OUT,       /* Right eye outside      */
    SM_PTPOINT_MOUTH_LEFT,          /* Mouth left             */
    SM_PTPOINT_MOUTH_RIGHT,         /* Mouth right            */
    SM_PTPOINT_MAX                  /* Number of Facial parts */
};


#ifdef  __cplusplus
extern "C" {
#endif

/**********************************************************/
/* Get Version                                            */
/**********************************************************/
/* Get Smile Degree Estimation Library API Version */
FACEPROC_API INT32      FACEPROC_SM_GetVersion(UINT8 *pbyMajor, UINT8 *pbyMinor);

/**********************************************************/
/* Create/Delete Handle                                   */
/**********************************************************/
/* Create Smile Estimation handle */
FACEPROC_API HSMILE     FACEPROC_SM_CreateHandle(void);

/* Delete Smile Degree Estimation handle */
FACEPROC_API INT32      FACEPROC_SM_DeleteHandle(HSMILE hSM);

/* Create Smile Degree Estimation result handle */
FACEPROC_API HSMRESULT  FACEPROC_SM_CreateResultHandle(void);

/* Delete Smile Degree Estimation result handle */
FACEPROC_API INT32      FACEPROC_SM_DeleteResultHandle(HSMRESULT hSmResult);

/**********************************************************/
/* Facial Feature Point Setting                           */
/**********************************************************/
/* Set the feature points for Smile Degree Estimation */
FACEPROC_API INT32      FACEPROC_SM_SetPoint(HSMILE hSM, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[], INT32 nUpDown, INT32 nLeftRight);

/* Set the feature points for Smile Degree Estimation from PT result handle */
FACEPROC_API INT32      FACEPROC_SM_SetPointFromHandle(HSMILE hSM, HPTRESULT hPtResult);

/**********************************************************/
/* Smile Degree Estimation                                */
/**********************************************************/
/* Estimate the smile degree */
FACEPROC_API INT32      FACEPROC_SM_Estimate(HSMILE hSM, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HSMRESULT hSmResult);

/**********************************************************/
/* Smile Degree Estimation Result                         */
/**********************************************************/
/* Get the estimated smile degree and its confidence level */
FACEPROC_API INT32      FACEPROC_SM_GetResult(HSMRESULT hSmResult, INT32 *pnSmile, INT32 *pnConfidence);

#ifdef  __cplusplus
}
#endif


#endif  /* FACEPROCSMAPI_H__ */
