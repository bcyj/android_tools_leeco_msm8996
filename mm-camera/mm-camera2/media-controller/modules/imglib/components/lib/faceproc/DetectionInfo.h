
#ifndef DETECTIONINFO_H__
#define DETECTIONINFO_H__

#include "FaceProcDef.h"


/* Detector Type */
#define DET_SOFT    (0x00000000U)        /* Software */
#define DET_IP      (0x00010000U)        /* IP       */

#define DET_DT      (0x00000100U)        /* Face Detector       */
#define DET_BD      (0x00000200U)        /* Human Body Detector */
#define DET_PD      (0x00000300U)        /* Pet Detector        */
#define DET_OT      (0x00000400U)        /* Object Tracking     */

#define DET_V1      (0x00000010U)
#define DET_V2      (0x00000020U)
#define DET_V3      (0x00000030U)
#define DET_V4      (0x00000040U)
#define DET_V5      (0x00000050U)
#define DET_V6      (0x00000060U)
#define DET_V7      (0x00000070U)
#define DET_V8      (0x00000080U)
#define DET_V9      (0x00000090U)


#define DETECTOR_TYPE_SOFT_BD_V1   (DET_SOFT|DET_BD|DET_V1)  /* Human Body Detection V1    */
#define DETECTOR_TYPE_SOFT_PD_V1   (DET_SOFT|DET_PD|DET_V1)  /* Pet Detection V1           */
#define DETECTOR_TYPE_OTHER        (0x00000000)              /* Third-party */


/* Object Types */
#define OBJ_TYPE_UNKNOWN  (0x00000000U)
#define OBJ_TYPE_FACE     (0x00010000U)
#define OBJ_TYPE_HUMAN    (0x00020000U)
#define OBJ_TYPE_OBJECT   (0x00040000U)
#define OBJ_TYPE_DOG      (0x00080000U)
#define OBJ_TYPE_CAT      (0x00100000U)


/* Pose(Angle of Yaw direction) */
#define POSE_YAW_LF_PROFILE   (-90)     /* Left Profile        */
#define POSE_YAW_LH_PROFILE   (-45)     /* Left Half Profile   */
#define POSE_YAW_FRONT          (0)     /* Front               */
#define POSE_YAW_RH_PROFILE    (45)     /* Right Half Profile  */
#define POSE_YAW_RF_PROFILE    (90)     /* Right Profile       */
#define POSE_YAW_HEAD        (-180)     /* Head                */
#define POSE_YAW_UNKOWN         (0)     /* Unkown              */


/* Detection Method */
#define DET_METHOD_DETECTED        (0x0000U)
#define DET_METHOD_TRACKED         (0x0001U)

#define DET_METHOD_ACCURACY_HIGH   (0x0100U)
#define DET_METHOD_ACCURACY_MIDDLE (0x0000U)

#define DET_METHOD_DETECTED_HIGH   (DET_METHOD_DETECTED|DET_METHOD_ACCURACY_HIGH)    /* Detected with high accuracy */
#define DET_METHOD_TRACKED_HIGH    (DET_METHOD_TRACKED |DET_METHOD_ACCURACY_HIGH)    /* Tracked with high accuracy  */
#define DET_METHOD_TRACKED_MIDDLE  (DET_METHOD_TRACKED |DET_METHOD_ACCURACY_MIDDLE)  /* Tracked with middle accuracy */


/* Confidence */
#define NO_CONFIDENCE    (-1)  /* No confidence */

typedef struct {
    INT32             nDetectorType;    /* Detector Type    */
    INT32             nObjectType;      /* Object Type      */

  /* Detection Result */
    INT32             nID;              /* ID Number        */
    INT32             nConfidence;      /* Confidence       */
    POINT             ptCenter;         /* Center Position  */
    INT32             nWidth;           /* Width            */
    INT32             nHeight;          /* Height           */
    INT32             nAngle;           /* Angle(Roll)      */
    INT32             nPose;            /* Pose (Yaw)       */
    INT32             nReserved;
    INT32             nDetectionMethod; /* Detection method */
    INT32             nHoldCount;       /* Hold Count       */
    INT32             nReserved2;
} DETECTION_INFO;

#endif /* DETECTIONINFO_H__ */
