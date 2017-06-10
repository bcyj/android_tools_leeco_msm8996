
/*
    Face Detection Library API
*/

#ifndef FACEPROCDTAPI_H__
#define FACEPROCDTAPI_H__

#define FACEPROC_API
#include "CommonDef.h"
#include "FaceProcDef.h"

#ifndef FACEPROC_DEF_HDETECTION
#define FACEPROC_DEF_HDETECTION
    typedef VOID *      HDETECTION; /* Face Detection Handle */
#endif /* FACEPROC_DEF_HDETECTION */

#ifndef FACEPROC_DEF_HDTRESULT
#define FACEPROC_DEF_HDTRESULT
    typedef VOID *      HDTRESULT;  /* Face Detection Result Handle */
#endif /* FACEPROC_DEF_HDTRESULT */


/* Face Detection mode */
#define DT_MODE_DEFAULT     0   /* Default Mode                              */
#define DT_MODE_MOTION1     1   /* Motion Mode: Whole search mode            */
#define DT_MODE_MOTION2     2   /* Motion Mode: 3 Partition search mode      */
#define DT_MODE_MOTION3     3   /* Motion Mode: Gradual progress search mode */

/* Face Pose */
#define POSE_FRONT          0   /* Straight(front view) face */
#define POSE_HALF_PROFILE   1   /* Diagonal face             */
#define POSE_PROFILE        2   /* Sideways face             */
#define POSE_TYPE_COUNT     3   /* Face pose types count     */

/* Face image direction and angle range (Non-tracking related settings) */
/* Lower  12 bits are for right side of Diagonal and Sideway faces      */
/* Higher 12 bits are for left  side of Diagonal and Sideway faces      */
#define ANGLE_0     0x00001001  /* Upward    Front +/- 15 degree */
#define ANGLE_1     0x00002002  /* Upward    Left  +/- 15 degree */
#define ANGLE_2     0x00004004  /* Rightward Right +/- 15 degree */
#define ANGLE_3     0x00008008  /* Rightward Front +/- 15 degree */
#define ANGLE_4     0x00010010  /* Rightward Left  +/- 15 degree */
#define ANGLE_5     0x00020020  /* Downward  Right +/- 15 degree */
#define ANGLE_6     0x00040040  /* Downward  Front +/- 15 degree */
#define ANGLE_7     0x00080080  /* Downward  Left  +/- 15 degree */
#define ANGLE_8     0x00100100  /* Leftward  Right +/- 15 degree */
#define ANGLE_9     0x00200200  /* Leftward  Front +/- 15 degree */
#define ANGLE_10    0x00400400  /* Leftward  Left  +/- 15 degree */
#define ANGLE_11    0x00800800  /* Upward    Right +/- 15 degree */

#define ANGLE_ALL       0x00ffffff                  /* All angles are detected                            */
#define ANGLE_NONE      0x00000000                  /* None of the angles will be detected                */
#define ANGLE_U15       ANGLE_0                     /* Up +/- 15 degree only will be detected             */
#define ANGLE_U45       ANGLE_0|ANGLE_1|ANGLE_11    /* Up +/- 45 degree only will be detected             */
#define ANGLE_ULR15     ANGLE_0|ANGLE_3|ANGLE_9     /* Up-Left-Right +/- 15 degree only will be detected  */
#define ANGLE_ULR45     ANGLE_0|ANGLE_1|ANGLE_2|ANGLE_3|ANGLE_4|ANGLE_8|ANGLE_9|ANGLE_10|ANGLE_11
                                                    /* Up-Left-Right +/- 45 degree only will be detected  */

/* Expand value settings for Tracking */
/* 4bits-wise setting                 */
/* From Higher bits: Not used, Head tracking Yes/No, Not used, Left-side pose,          */
/*                   Left-side rotation, Not used, Right-side pose, Right-side rotation */
#define ANGLE_ROTATION_EXT0     ANGLE_NONE      /* No rotation angle expansion                            */
#define ANGLE_ROTATION_EXT1     0x00001001      /* Rotation angle expansion: Upto left-right 1 direction  */
#define ANGLE_ROTATION_EXT2     0x00002002      /* Rotation angle expansion: Upto left-right 2 directions */
#define ANGLE_ROTATION_EXTALL   0x0000b00b      /* Rotation angle expansion: In all directions            */

#define ANGLE_POSE_EXT0         ANGLE_NONE      /* No Face pose expansion                                 */
#define ANGLE_POSE_EXT1         0x00010010      /* Face pose expansion: Upto left-right 1 direction       */
#define ANGLE_POSE_EXTALL       0x00070070      /* Face pose expansion: In all directions                 */

#define DETECT_HEAD_NOUSE       ANGLE_NONE      /* Head tracking disabled                                 */
#define DETECT_HEAD_USE         0x01000000      /* Head tracking enabled                                  */

/* Face detection accuracy */
#define ACCURACY_NORMAL     0       /* Detects using default accuracy             */
#define ACCURACY_HIGH_TR    1       /* Detects with high accuracy during tracking */

/* Face Pose(Angle of Yaw direction) */
#define DT_POSE_LF_PROFILE   -90    /* Left Profile Face   */
#define DT_POSE_LH_PROFILE   -45    /* Left inclined Face  */
#define DT_POSE_FRONT          0    /* Front Face          */
#define DT_POSE_RH_PROFILE    45    /* Right-inclined Face */
#define DT_POSE_RF_PROFILE    90    /* Right Profile Face  */
#define DT_POSE_HEAD        -180    /* Head                */

#ifdef  __cplusplus
extern "C" {
#endif

/**********************************************************/
/* Get Version                                            */
/**********************************************************/
/* Get Face Detection Library API Version */
FACEPROC_API INT32      FACEPROC_GetDtVersion(UINT8 *pucMajor, UINT8 *pucMinor);

/**********************************************************/
/* Required Memory Calculation                            */
/**********************************************************/
/* Get the Required Memory Size for Default mode */
FACEPROC_API INT32      FACEPROC_GetDtRequiredStillMemSize(
                        INT32 nWidth, INT32 nHeight, INT32 nMinSize, INT32 nMaxFaceNumber,
                        RECT rcNonTrackingEdge, INT32 nNonTrackingStep,
                        UINT32 *pnBackupMemSize, UINT32 *pnMinWorkMemSize,
                        UINT32 *pnMaxWorkMemSize);
/* Get the Required Memory Size for Motion mode */
FACEPROC_API INT32      FACEPROC_GetDtRequiredMovieMemSize(
                        INT32 nWidth, INT32 nHeight, INT32 nMinSize, INT32 nMaxFaceNumber,
                        RECT rcNonTrackingEdge, INT32 nNonTrackingStep, RECT rcTrackingEdge, INT32 nTrackingStep,
                        UINT32 *pnBackupMemSize, UINT32 *pnMinWorkMemSize,
                        UINT32 *pnMaxWorkMemSize);

/**********************************************************/
/* Create/Delete/Clear Handle                             */
/**********************************************************/
/* Create Face Detection handle */
FACEPROC_API HDETECTION FACEPROC_CreateDetection(void);
/* Delete Face Detection handle */
FACEPROC_API INT32      FACEPROC_DeleteDetection(HDETECTION  hDT);

/* Create Face Detection result handle */
FACEPROC_API HDTRESULT  FACEPROC_CreateDtResult(INT32 nMaxFaceNumber, INT32 nMaxSwapNumber);
/* Delete Face Detection result handle */
FACEPROC_API INT32      FACEPROC_DeleteDtResult(HDTRESULT hDtResult);
/* Clear Face Detection result handle  */
FACEPROC_API INT32      FACEPROC_ClearDtResult(HDTRESULT hDtResult);

/**********************************************************/
/* Face Detection                                         */
/**********************************************************/
/* Execute Face Detection */
FACEPROC_API INT32      FACEPROC_Detection(HDETECTION hDT, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight,
                                   INT32 nAccuracy, HDTRESULT hDtResult);

/**********************************************************/
/* Get Face Detection Result                              */
/**********************************************************/
/* Get the number of faces detected by face detection */
FACEPROC_API INT32      FACEPROC_GetDtFaceCount(HDTRESULT hDtResult, INT32 *pnCount);
/* Get the Face direction, ID, Face rectangle coordinates */
/* and Confidence Degree of the specified detected face   */
FACEPROC_API INT32      FACEPROC_GetDtFaceInfo(HDTRESULT hDtResult, INT32 nIndex, FACEINFO *psFaceInfo);

/**********************************************************/
/* Set/Get Face Detection Mode                            */
/**********************************************************/
/* Set the Face Detection Mode */
FACEPROC_API INT32      FACEPROC_SetDtMode(HDETECTION hDT, INT32 nMode);
/* Get the Face Detection Mode */
FACEPROC_API INT32      FACEPROC_GetDtMode(HDETECTION hDT, INT32 *pnMode);

/**********************************************************/
/* Set/Get Minimum/Maximum Face Size                      */
/**********************************************************/
/* Set the Minimum and Maximum face sizes to be detected */
FACEPROC_API INT32      FACEPROC_SetDtFaceSizeRange(HDETECTION hDT, INT32 nMinSize, INT32 nMaxSize);
/* Get the Minimum and Maximum face sizes to be detected */
FACEPROC_API INT32      FACEPROC_GetDtFaceSizeRange(HDETECTION hDT, INT32 *pnMinSize, INT32 *pnMaxSize);

/**********************************************************/
/* Set/Get Face Detection Angle                           */
/**********************************************************/
/* Set the Angle settings for face detection */
FACEPROC_API INT32      FACEPROC_SetDtAngle(HDETECTION hDT, UINT32 anNonTrackingAngle[POSE_TYPE_COUNT],
                                    UINT32 nTrackingAngleExtension);
/* Get the Angle settings for face detection */
FACEPROC_API INT32      FACEPROC_GetDtAngle(HDETECTION hDT, UINT32 *panNonTrackingAngle, UINT32 *pnTrackingAngleExtension);

/**********************************************************/
/* Set/Get Face Detection Rectangular Mask                */
/**********************************************************/
/* Set the Face Detction Rectangular Mask */
FACEPROC_API INT32      FACEPROC_SetDtRectangleMask(HDETECTION hDT, RECT rcNonTrackingArea, RECT rcTrackingArea);
/* Get the Face Detction Rectangular Mask */
FACEPROC_API INT32      FACEPROC_GetDtRectangleMask(HDETECTION hDT, RECT *prcNonTrackingArea, RECT *prcTrackingArea);

/**********************************************************/
/* Set/Get Face Detection Threshold                       */
/**********************************************************/
/* Set the Face Detection Threshold */
FACEPROC_API INT32      FACEPROC_SetDtThreshold(HDETECTION hDT, INT32 nNonTrackingThreshold, INT32 nTrackingThreshold);
/* Get the Face Detection Threshold */
FACEPROC_API INT32      FACEPROC_GetDtThreshold(HDETECTION hDT, INT32 *pnNonTrackingThreshold, INT32 *pnTrackingThreshold);

/**********************************************************/
/* Set/Get Face Detection Search Density Coefficient      */
/**********************************************************/
/* Set the search density coefficient for face detection */
FACEPROC_API INT32      FACEPROC_SetDtStep(HDETECTION hDT, INT32 nNonTrackingStep, INT32 nTrackingStep);
/* Get the search density coefficient for face detection */
FACEPROC_API INT32      FACEPROC_GetDtStep(HDETECTION hDT, INT32 *pnNonTrackingStep, INT32 *pnTrackingStep);


/**********************************************************/
/* Lock/Unlock tracking ID                                */
/**********************************************************/
/* Lock/Unlock tracking ID */
FACEPROC_API INT32      FACEPROC_DtLockID(HDTRESULT hDtResult, INT32 nID);

/**********************************************************/
/* Set/Get Face Detection Direction Mask                  */
/**********************************************************/
/* Set the Direction Mask for Motion mode */
FACEPROC_API INT32      FACEPROC_SetDtDirectionMask(HDETECTION hDT, BOOL bMask);
/* Get the Direction Mask for Motion mode */
FACEPROC_API INT32      FACEPROC_GetDtDirectionMask(HDETECTION hDT, BOOL *pbMask);

/**********************************************************/
/* Set/Get Motion Face Detection Refresh Count            */
/**********************************************************/
/* Set Motion Face Detection Refresh Count for each Motion mode */
FACEPROC_API INT32      FACEPROC_SetDtRefreshCount(HDETECTION hDT, INT32 nMode, INT32 nRefreshCount);
/* Get Motion Face Detection Refresh Count for each Motion mode */
FACEPROC_API INT32      FACEPROC_GetDtRefreshCount(HDETECTION hDT, INT32 nMode, INT32 *pnRefreshCount);

/**********************************************************/
/* Set/Get Motion Face Detection Retry and Hold Count     */
/**********************************************************/
/* Set Motion Face Detection Retry Count, Motion Head Detection Retry Count and Hold Count at lost time */
FACEPROC_API INT32      FACEPROC_SetDtLostParam(HDETECTION hDT,
                                        INT32 nFaceRetryCount, INT32 nHeadRetryCount, INT32 nHoldCount);
/* Get Motion Face Detection Retry Count, Motion Head Detection Retry Count, and Hold Count at lost time */
FACEPROC_API INT32      FACEPROC_GetDtLostParam(HDETECTION hDT,
                                        INT32 *pnFaceRetryCount, INT32 *pnHeadRetryCount, INT32 *pnHoldCount);

/********************************************************************/
/* Set/Get Motion Mode Face Detection Position Correction Parameter */
/********************************************************************/
/* Set the motion mode face detection position correction parameter */
FACEPROC_API INT32      FACEPROC_SetDtModifyMoveRate(HDETECTION hDT, INT32 nModifyMoveRate);
/* Get the motion mode face detection position correction parameter */
FACEPROC_API INT32      FACEPROC_GetDtModifyMoveRate(HDETECTION hDT, INT32 *pnModifyMoveRate);



/**********************************************************/
/* Set/Get Maximum Amount of Working Memory Used          */
/**********************************************************/
/* Set the maximum amount of working memory used for FACEPROC_Detection() */
FACEPROC_API INT32      FACEPROC_SetDtMemorySize(HDETECTION hDT, UINT32 unSize) ;
/* Get the maximum amount of working memory used for FACEPROC_Detection() */
FACEPROC_API INT32      FACEPROC_GetDtMemorySize(HDETECTION hDT, UINT32 *punSize);

/**********************************************************/
/* Set/Get Timeout time for Face Detection                */
/**********************************************************/
/* Set the timeout time for FACEPROC_Detection() */
FACEPROC_API INT32      FACEPROC_SetDtTimeout(HDETECTION hDT, INT32 nNonTrackingTimeout, INT32 nTrackingTimeout);
/* Get the timeout time for FACEPROC_Detection() */
FACEPROC_API INT32      FACEPROC_GetDtTimeout(HDETECTION hDT, INT32 *pnNonTrackingTimeout, INT32 *pnTrackingTimeout);


#ifdef  __cplusplus
}
#endif

#endif  /* FACEPROCDTAPI_H__ */

