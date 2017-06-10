
/*========================================================================

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

====================================================================== */
#include "DetectorComDef.h"
#include "FaceProcAPI.h"
#include "FaceProcCtAPI.h"
#include "FaceProcDtAPI.h"
#include "FaceProcFrAPI.h"
#include "FaceProcGbAPI.h"
#include "FaceProcPcAPI.h"
#include "FaceProcPtAPI.h"
#include "FaceProcSmAPI.h"
#include "FaceProcTime.h"
#include <inttypes.h>
#include "camera.h"

#define FACE_CONTOUR_DETECT 0
#define FACE_SMILE_DETECT 1
#define FACE_RECOGNITION 1
#define FACE_RECOGNIZE_TEST_REGISTER 0

#define KB_SIZE 1024
#define MAX_REGISTER_PPL 2
#define MAX_REGISTER_IMG_PER_PERSON 2
#define DT_THRESHOLD 500

#define MAX_ALBUM_SIZE (KB_SIZE + (164+148*(MAX_REGISTER_PPL*MAX_REGISTER_IMG_PER_PERSON)))

/* Private definition of the faceproc engine type */
typedef struct {
  void *FaceProc_ptr;
  /*FaceProc APIs*/
  INT32      (*FACEPROC_FreeBMemoryArea)(void);
  INT32      (*FACEPROC_FreeWMemoryArea)(void);
  INT32      (*FACEPROC_SetBMemoryArea)(void *pMemoryAddr, UINT32 unSize);
  INT32      (*FACEPROC_SetWMemoryArea)(void *pMemoryAddr, UINT32 unSize);
  /*FaceProcDt APIs*/

  INT32      (*FACEPROC_DeleteDtResult)(HDTRESULT hDtResult);
  INT32      (*FACEPROC_DeleteDetection)(HDETECTION  hDT);
  INT32      (*FACEPROC_GetDtFaceCount)(HDTRESULT hDtResult,
             INT32 *pnCount);
  INT32      (*FACEPROC_GetDtFaceInfo)(HDTRESULT hDtResult,
             INT32 nIndex, FACEINFO *psFaceInfo);
  INT32      (*FACEPROC_SetDtMemorySize)(HDETECTION hDT, UINT32 unSize);
  INT32      (*FACEPROC_GetDtRequiredMovieMemSize)(
             INT32 nWidth, INT32 nHeight, INT32 nMinSize,
             INT32 nMaxFaceNumber, RECT rcNonTrackingEdge,
             INT32 nNonTrackingStep, RECT rcTrackingEdge,
             INT32 nTrackingStep, UINT32 *pnBackupMemSize,
             UINT32 *pnMinWorkMemSize, UINT32 *pnMaxWorkMemSize);
  HDETECTION (*FACEPROC_CreateDetection)(VOID);
  INT32      (*FACEPROC_SetDtMode)(HDETECTION hDT, INT32 nMode);
  INT32      (*FACEPROC_SetDtStep)(HDETECTION hDT,
             INT32 nNonTrackingStep, INT32 nTrackingStep);
  INT32      (*FACEPROC_SetDtAngle)(HDETECTION hDT,
             UINT32 anNonTrackingAngle[POSE_TYPE_COUNT],
             UINT32 nTrackingAngleExtension);
  INT32      (*FACEPROC_SetDtDirectionMask)(HDETECTION hDT, BOOL bMask);
  INT32      (*FACEPROC_SetDtFaceSizeRange)(HDETECTION hDT, INT32 nMinSize,
             INT32 nMaxSize);
  INT32      (*FACEPROC_SetDtThreshold)(HDETECTION hDT,INT32 nNonTrackingThreshold,
             INT32 nTrackingThreshold);
  HDTRESULT  (*FACEPROC_CreateDtResult)(INT32 nMaxFaceNumber, INT32 nMaxSwapNumber);
             INT32      (*FACEPROC_Detection)(HDETECTION hDT, RAWIMAGE *pImage,
             INT32 nWidth, INT32 nHeight,
             INT32 nAccuracy, HDTRESULT hDtResult);
  /*FaceProcPT API*/
  HPOINTER   (*FACEPROC_PT_CreateHandle)(void);
  INT32      (*FACEPROC_PT_DeleteHandle)(HPOINTER hPT);
  HPTRESULT  (*FACEPROC_PT_CreateResultHandle)(void);
  INT32      (*FACEPROC_PT_DeleteResultHandle)(HPTRESULT hPtResult);
  INT32      (*FACEPROC_PT_SetPositionFromHandle)(HPOINTER hPT,
             HDTRESULT hDtResult, INT32 nIndex);
  INT32      (*FACEPROC_PT_SetMode)(HPOINTER hPT, INT32 nMode);
  INT32      (*FACEPROC_PT_SetConfMode)(HPOINTER hPT, INT32 nConfMode);
  INT32      (*FACEPROC_PT_DetectPoint)(HPOINTER hPT, RAWIMAGE *pImage,
             INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);
  INT32      (*FACEPROC_PT_GetResult)(HPTRESULT hPtResult,
             INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
  INT32      (*FACEPROC_PT_GetFaceDirection)(HPTRESULT hPtResult,
             INT32 *pnUpDown, INT32 *pnLeftRight, INT32 *pnRoll);
  /*FaceProcCt API */
  HCONTOUR   (*FACEPROC_CT_CreateHandle)(void);
  INT32      (*FACEPROC_CT_DeleteHandle)(HCONTOUR hCT);
  HCTRESULT  (*FACEPROC_CT_CreateResultHandle)(void);
  INT32      (*FACEPROC_CT_DeleteResultHandle)(HCTRESULT hCtResult);
  INT32      (*FACEPROC_CT_SetPointFromHandle)(HCONTOUR hCT, HPTRESULT hPtResult);
  INT32      (*FACEPROC_CT_DetectContour)(HCONTOUR hCT, RAWIMAGE *pImage,
             INT32 nWidth, INT32 nHeight, HCTRESULT hCtResult);
  INT32      (*FACEPROC_CT_GetResult)(HCTRESULT hCtResult, INT32 nPointNum,
             POINT aptCtPoint[]);
  INT32      (*FACEPROC_CT_SetDetectionMode)(HCONTOUR hCT, INT32 nMode);
  /* FaceProcSm APIT */
  HSMILE     (*FACEPROC_SM_CreateHandle)(void);
  INT32      (*FACEPROC_SM_DeleteHandle)(HSMILE hSM);
  HSMRESULT  (*FACEPROC_SM_CreateResultHandle)(void);
  INT32      (*FACEPROC_SM_DeleteResultHandle)(HSMRESULT hSmResult);
  INT32      (*FACEPROC_SM_SetPointFromHandle)(HSMILE hSM, HPTRESULT hPtResult);
  INT32      (*FACEPROC_SM_GetResult)(HSMRESULT hSmResult, INT32 *pnSmile,
             INT32 *pnConfidence);
  INT32      (*FACEPROC_SM_Estimate)(HSMILE hSM, RAWIMAGE *pImage, INT32 nWidth,
             INT32 nHeight, HSMRESULT hSmResult);

  /* FaceProcFR API */

  HFEATURE   (*FACEPROC_FR_CreateFeatureHandle)( void );
  INT32      (*FACEPROC_FR_DeleteFeatureHandle)(HFEATURE hFeature);
  HALBUM     (*FACEPROC_FR_CreateAlbumHandle)(INT32 nMaxUserNum,
             INT32 nMaxDataNumPerUser);
  INT32      (*FACEPROC_FR_DeleteAlbumHandle)(HALBUM hAlbum);
  INT32      (*FACEPROC_FR_ExtractFeatureFromPtHdl)(HFEATURE hFeature,
             RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);
  INT32      (*FACEPROC_FR_RegisterData)(HALBUM hAlbum, HFEATURE hFeature,
             INT32 nUserID,  INT32 nDataID);
  INT32      (*FACEPROC_FR_GetRegisteredUserNum)(HALBUM hAlbum, INT32 *pnUserNum);
  INT32      (*FACEPROC_FR_GetRegisteredUsrDataNum)(HALBUM hAlbum, INT32 nUserID,
             INT32 *pnUserDataNum);
  INT32      (*FACEPROC_FR_ClearAlbum)(HALBUM hAlbum);
  INT32      (*FACEPROC_FR_GetSerializedAlbumSize)(HALBUM hAlbum,
             UINT32 *punSerializedAlbumSize);
  INT32      (*FACEPROC_FR_SerializeAlbum)(HALBUM hAlbum, UINT8 *pbyBuffer,
             UINT32 unBufSize);
  HALBUM     (*FACEPROC_FR_RestoreAlbum)(UINT8 *pbyBuffer, UINT32 unBufSize,
             FR_ERROR *pError);
  INT32      (*FACEPROC_FR_Identify)(HFEATURE hFeature, HALBUM hAlbum,
             INT32 nMaxResultNum, INT32 anUserID[], INT32 anScore[], INT32 *pnResultNum);
  INT32      (*FACEPROC_FR_Verify)(HFEATURE hFeature, HALBUM hAlbum,
             INT32 nUserID, INT32 *pnScore );
  INT32      (*FACEPROC_FR_IsRegistered)(HALBUM hAlbum, INT32 nUserID,
             INT32 nDataID, BOOL *pIsRegistered);

  /* Gaze/Blink Detect */
  HGAZEBLINK   (*FACEPROC_GB_CreateHandle)(void);
  INT32        (*FACEPROC_GB_DeleteHandle)(HGAZEBLINK hGB);
  HGBRESULT    (*FACEPROC_GB_CreateResultHandle)(void);
  INT32        (*FACEPROC_GB_DeleteResultHandle)(HGBRESULT hGbResult);
  INT32        (*FACEPROC_GB_SetPointFromHandle)(HGAZEBLINK hGB, HPTRESULT hPtResult);
  INT32        (*FACEPROC_GB_Estimate)(HGAZEBLINK hGB, RAWIMAGE *pImage, INT32 nWidth,
               INT32 nHeight, HGBRESULT hGbResult);
  INT32        (*FACEPROC_GB_GetEyeCloseRatio)(HGBRESULT hGbResult,
               INT32 *pnCloseRatioLeftEye, INT32 *pnCloseRatioRightEye);
  INT32        (*FACEPROC_GB_GetGazeDirection)(HGBRESULT hGbResult,
               INT32 *pnGazeLeftRight, INT32 *pnGazeUpDown);
} faceproc_engine_lib_t;

/* Private definition of the faceproc engine type */
typedef struct {
  /* Frame dimension */
  uint32_t                 frame_width;
  uint32_t                 frame_height;
  /* FaceprocEngine-specific fields */
  HDETECTION               hdt;
  HDTRESULT                hresult;
  HPOINTER                 hpt;
  HPTRESULT                hptresult[MAX_ROI];
  HCONTOUR                 hct;
  HCTRESULT                hctresult[MAX_ROI];
  HSMILE                   hsm;
  HSMRESULT                hsmresult[MAX_ROI];
  HFEATURE                 hfeature;
  HALBUM                   halbum;
  HGAZEBLINK               hgb;
  HGBRESULT                hgbresult[MAX_ROI];
  /* Scratch buffers */
  void                    *bmem;  /* backup memory */
  void                    *wmem;  /* work memory */
  /* Flags */
  uint8_t                  fdBmemorySet;
  uint8_t                  fdWmemorySet;
  int                      recognized[MAX_ROI];
  int                      last_img_registered_idx;
  INT32                    UserID[MAX_REGISTER_PPL];
  int                      confidence[MAX_REGISTER_PPL];
  faceproc_engine_lib_t    lib;
} faceproc_engine_t;

/* Prototypes of the faceproc engine functions */
int faceproc_engine_create    (void * Ctrl);
int faceproc_engine_config    (void *Ctrl, void *config);
int faceproc_engine_run       (void * Ctrl);
int faceproc_engine_get_output(void *Ctrl, void* output);
int faceproc_engine_destroy   ( void *Ctrl);

/* Helper functions */
static int faceproc_engine_reset  (faceproc_engine_t *);
