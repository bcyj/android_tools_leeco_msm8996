
/* 
    Facial Parts Detection Library API
*/

#ifndef FACEPROCPTAPI_H__
#define FACEPROCPTAPI_H__

#define FACEPROC_API
#include "FaceProcDef.h"

#ifndef FACEPROC_DEF_HPOINTER
#define FACEPROC_DEF_HPOINTER
    typedef void *      HPOINTER; /* Facial Parts Detection Handle */
#endif /* FACEPROC_DEF_HPOINTER */

#ifndef FACEPROC_DEF_HPTRESULT
#define FACEPROC_DEF_HPTRESULT
    typedef void *      HPTRESULT; /* Facial Parts Detection Result Handle */
#endif /* FACEPROC_DEF_HPTRESULT */

#ifndef FACEPROC_DEF_HDTRESULT
#define FACEPROC_DEF_HDTRESULT
    typedef void *      HDTRESULT; /* Face Detection Result Handle */
#endif /* FACEPROC_DEF_HDTRESULT */

/* Feature point extraction mode */
#define PT_MODE_DEFAULT             0                   /* Normal Mode      */
#define PT_MODE_FAST                1                   /* Fast Mode        */
#define PT_MODE_SUPER_FAST          2                   /* Super Fast Mode  */

/* Confidence output mode */
#define PT_CONF_USE                 0                   /* Enable confidence calculation */
#define PT_CONF_NOUSE               1                   /* Skip Confidence calculation   */

#define PT_MODE_CONF_DEFAULT        PT_CONF_USE         /* Enable confidence calculation */
#define PT_MODE_CONF_NOUSE          PT_CONF_NOUSE       /* Skip Confidence calculation   */

/* Face detection (FD) version */
#define DTVERSION  INT32
#define DTVERSION_SOFT_V3       300   /* FD V3.x(000300)               */
#define DTVERSION_SOFT_V4       400   /* FD V4.x(000400)               */
#define DTVERSION_SOFT_V5       500   /* FD V5.x(000500)               */
#define DTVERSION_IP_V1       10100   /* FD IPV1.x                     */
#define DTVERSION_IP_V2       10200   /* FD IPV2.x                     */
#define DTVERSION_IP_V3       10300   /* FD IPV3.x                     */
#define DTVERSION_UNKNOWN         0   /* 3rd party FD engine (000000)  */

/* Detected face pose */
#define PT_POSE_LF_PROFILE   -90    /* Left side face               */
#define PT_POSE_LH_PROFILE   -45    /* Partially left sided face    */
#define PT_POSE_FRONT          0    /* Frontal face                 */
#define PT_POSE_RH_PROFILE    45    /* Partially Right sided face   */
#define PT_POSE_RF_PROFILE    90    /* Right sided face             */
#define PT_POSE_UNKNOWN        0    /* Pose Unknown                 */

/* Facial Parts Detection Point Index */
enum PT_PARTS_POINT {
    PT_POINT_LEFT_EYE = 0,  /* Center of left eye        */
    PT_POINT_RIGHT_EYE,     /* Center of right eye       */
    PT_POINT_MOUTH,         /* Mouth Center              */
    PT_POINT_LEFT_EYE_IN,   /* Inner corner of left eye  */
    PT_POINT_LEFT_EYE_OUT,  /* Outer corner of left eye  */
    PT_POINT_RIGHT_EYE_IN,  /* Inner corner of right eye */
    PT_POINT_RIGHT_EYE_OUT, /* Outer corner of right eye */
    PT_POINT_MOUTH_LEFT,    /* Left corner of mouth      */
    PT_POINT_MOUTH_RIGHT,   /* Right corner of mouth     */
    PT_POINT_NOSE_LEFT,     /* Left Nostril              */
    PT_POINT_NOSE_RIGHT,    /* Right Nostril             */
    PT_POINT_MOUTH_UP,      /* Mouth top                 */
    PT_POINT_KIND_MAX       /* Number of Parts Points    */
};


#define FEATURE_NO_POINT    -1  /* Parts Points Could Not Be Detected */

#ifdef  __cplusplus
extern "C" {
#endif

/**********************************************************/
/* Get Version                                            */
/**********************************************************/
/* Get Facial Parts Detection Library API Version */
FACEPROC_API INT32      FACEPROC_PT_GetVersion(UINT8 *pucMajor, UINT8 *pucMinor);

/**********************************************************/
/* Create/Delete Handle                                   */
/**********************************************************/
/* Create Facial Parts Detection Handle */
FACEPROC_API HPOINTER   FACEPROC_PT_CreateHandle(void);
/* Delete Facial Parts Detection Handle */
FACEPROC_API INT32      FACEPROC_PT_DeleteHandle(HPOINTER hPT);
/* Create Facial Parts Detection Result Handle */
FACEPROC_API HPTRESULT  FACEPROC_PT_CreateResultHandle(void);
/* Delete Facial Parts Detection Result Handle */
FACEPROC_API INT32      FACEPROC_PT_DeleteResultHandle(HPTRESULT hPtResult);

/**********************************************************/
/* Set Face Position                                      */
/**********************************************************/
/* Set face position from Face detection result */
FACEPROC_API INT32      FACEPROC_PT_SetPositionFromHandle(HPOINTER hPT, HDTRESULT hDtResult,INT32 nIndex);
/* Set face position */
FACEPROC_API INT32      FACEPROC_PT_SetPosition(HPOINTER hPT, POINT *pptLeftTop, POINT *pptRightTop, POINT *pptLeftBottom,
                                                                       POINT *pptRightBottom, INT32 nPose, DTVERSION DtVersion);
/* Set face position from Motion face detection IP result */
FACEPROC_API INT32      FACEPROC_PT_SetPositionIP(HPOINTER hPT, INT32 nCenterX, INT32 nCenterY, INT32 nSize, INT32 nAngle,
                                                                       INT32 nScale, INT32 nPose, DTVERSION DtVersion);

/**********************************************************/
/* Set/Get Facial Parts Detection Mode                    */
/**********************************************************/
/* Set Facial Parts Detection Mode */
FACEPROC_API INT32      FACEPROC_PT_SetMode(HPOINTER hPT, INT32 nMode);
/* Get Facial Parts Detection Mode */
FACEPROC_API INT32      FACEPROC_PT_GetMode(HPOINTER hPT, INT32 *pnMode);

/**********************************************************/
/* Set/Get Confidence calculation Mode                    */
/**********************************************************/
/* Set Confidence calculation Mode */
FACEPROC_API INT32      FACEPROC_PT_SetConfMode(HPOINTER hPT, INT32 nConfMode);
/* Get Confidence calculation Mode */
FACEPROC_API INT32      FACEPROC_PT_GetConfMode(HPOINTER hPT, INT32 *pnConfMode);

/**********************************************************/
/* Facial Parts Detection                                 */
/**********************************************************/
/* Execute Facial Parts Detection */
FACEPROC_API INT32      FACEPROC_PT_DetectPoint(HPOINTER hPT, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);

/**********************************************************/
/* Get Facial Parts Detection Result                      */
/**********************************************************/
/* Get Facial Parts Position Result */
FACEPROC_API INT32      FACEPROC_PT_GetResult(HPTRESULT hPtResult, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
/* Get the face direction angles(degree) detected by FACEPROC_Pointer() */
FACEPROC_API INT32      FACEPROC_PT_GetFaceDirection(HPTRESULT hPtResult, INT32 *pnUpDown, INT32 *pnLeftRight, INT32 *pnRoll);

#ifdef  __cplusplus
}
#endif

#endif    /* FACEPROCPTAPI_H__ */
