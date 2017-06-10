/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __FD_COMP_H__
#define __FD_COMP_H__

#include "img_comp_priv.h"
#include "faceproc.h"

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
#include "fd_chromatix.h"


/**
 * CONSTANTS and MACROS
 **/

#define FD_FACEPT_ENABLE(p) (p->config.fd_feature_mask & FACE_PROP_PARTS_ON)
#define FD_ANGLE_ENABLE(p) (p->config.fd_feature_mask & FACE_PROP_ANGLE_ON)
#define FD_DS_ENABLE(p) (p->config.fd_feature_mask & FACE_PROP_DOWNSCALE_ON)

#define KB_SIZE 1024
#define MAX_REGISTER_PPL 2
#define MAX_REGISTER_IMG_PER_PERSON 2
#define DT_THRESHOLD 500
#define FR_THRESHOLD 500

#define MAX_ALBUM_SIZE \
 (KB_SIZE + (164 + 148*(MAX_REGISTER_PPL * MAX_REGISTER_IMG_PER_PERSON)))

/* utility functions to get frame info */
/** FD_ADDR
 *   @p: pointer to the frame
 *
 *   Returns the Y address from the frame
 **/
#define FD_ADDR(p) (p->frame[0].plane[0].addr)

/** FD_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the Y plane width
 **/
#define FD_WIDTH(p) (p->frame[0].plane[0].width)

/** FD_HEIGHT
 *   @p: pointer to the frame
 *
 *   Returns the Y plane height
 **/
#define FD_HEIGHT(p) (p->frame[0].plane[0].height)

/** faceproc_comp_eng_config
 *   @ptr: pointer to the faceproc library handle
 *
 *   The function pointers maps to the faceproc engine library
 **/
typedef struct {
  void *ptr;
  /*FaceProc APIs*/
  INT32 (*FACEPROC_FreeBMemoryArea)(void);
  INT32 (*FACEPROC_FreeWMemoryArea)(void);
  INT32 (*FACEPROC_SetBMemoryArea)(void *pMemoryAddr, UINT32 unSize);
  INT32 (*FACEPROC_SetWMemoryArea)(void *pMemoryAddr, UINT32 unSize);

  /*FaceProcDt APIs*/
  INT32 (*FACEPROC_SetDtRefreshCount)(HDETECTION hDT, INT32 nMode,
    INT32 nRefreshCount);
  INT32 (*FACEPROC_DeleteDtResult)(HDTRESULT hDtResult);
  INT32 (*FACEPROC_DeleteDetection)(HDETECTION  hDT);
  INT32 (*FACEPROC_GetDtFaceCount)(HDTRESULT hDtResult,
    INT32 *pnCount);
  INT32 (*FACEPROC_GetDtFaceInfo)(HDTRESULT hDtResult,
    INT32 nIndex, FACEINFO *psFaceInfo);
  INT32 (*FACEPROC_SetDtMemorySize)(HDETECTION hDT, UINT32 unSize);
  INT32 (*FACEPROC_GetDtRequiredMovieMemSize)(
    INT32 nWidth, INT32 nHeight, INT32 nMinSize,
    INT32 nMaxFaceNumber, RECT rcNonTrackingEdge,
    INT32 nNonTrackingStep, RECT rcTrackingEdge,
    INT32 nTrackingStep, UINT32 *pnBackupMemSize,
    UINT32 *pnMinWorkMemSize, UINT32 *pnMaxWorkMemSize);
  HDETECTION (*FACEPROC_CreateDetection)(VOID);
  INT32 (*FACEPROC_SetDtMode)(HDETECTION hDT, INT32 nMode);
  INT32 (*FACEPROC_SetDtStep)(HDETECTION hDT,
    INT32 nNonTrackingStep, INT32 nTrackingStep);
  INT32 (*FACEPROC_SetDtAngle)(HDETECTION hDT,
    UINT32 anNonTrackingAngle[POSE_TYPE_COUNT],
    UINT32 nTrackingAngleExtension);
  INT32 (*FACEPROC_SetDtDirectionMask)(HDETECTION hDT, BOOL bMask);
  INT32 (*FACEPROC_SetDtFaceSizeRange)(HDETECTION hDT, INT32 nMinSize,
    INT32 nMaxSize);
  INT32 (*FACEPROC_SetDtThreshold)(HDETECTION hDT,
    INT32 nNonTrackingThreshold,
    INT32 nTrackingThreshold);
  HDTRESULT (*FACEPROC_CreateDtResult)(INT32 nMaxFaceNumber,
    INT32 nMaxSwapNumber);
    INT32      (*FACEPROC_Detection)(HDETECTION hDT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight,
    INT32 nAccuracy, HDTRESULT hDtResult);
  /*FaceProcPT API*/
  HPOINTER (*FACEPROC_PT_CreateHandle)(void);
  INT32 (*FACEPROC_PT_DeleteHandle)(HPOINTER hPT);
  HPTRESULT (*FACEPROC_PT_CreateResultHandle)(void);
  INT32 (*FACEPROC_PT_DeleteResultHandle)(HPTRESULT hPtResult);
  INT32 (*FACEPROC_PT_SetPositionFromHandle)(HPOINTER hPT,
    HDTRESULT hDtResult, INT32 nIndex);
  INT32 (*FACEPROC_PT_SetMode)(HPOINTER hPT, INT32 nMode);
  INT32 (*FACEPROC_PT_SetConfMode)(HPOINTER hPT, INT32 nConfMode);
  INT32 (*FACEPROC_PT_DetectPoint)(HPOINTER hPT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);
  INT32 (*FACEPROC_PT_GetResult)(HPTRESULT hPtResult,
    INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
  INT32 (*FACEPROC_PT_GetFaceDirection)(HPTRESULT hPtResult,
    INT32 *pnUpDown, INT32 *pnLeftRight, INT32 *pnRoll);
  /*FaceProcCt API */
  HCONTOUR (*FACEPROC_CT_CreateHandle)(void);
  INT32 (*FACEPROC_CT_DeleteHandle)(HCONTOUR hCT);
  HCTRESULT (*FACEPROC_CT_CreateResultHandle)(void);
  INT32 (*FACEPROC_CT_DeleteResultHandle)(HCTRESULT hCtResult);
  INT32 (*FACEPROC_CT_SetPointFromHandle)(HCONTOUR hCT,
    HPTRESULT hPtResult);
  INT32 (*FACEPROC_CT_DetectContour)(HCONTOUR hCT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight, HCTRESULT hCtResult);
  INT32 (*FACEPROC_CT_GetResult)(HCTRESULT hCtResult, INT32 nPointNum,
    POINT aptCtPoint[]);
  INT32 (*FACEPROC_CT_SetDetectionMode)(HCONTOUR hCT, INT32 nMode);
  /* FaceProcSm APIT */
  HSMILE (*FACEPROC_SM_CreateHandle)(void);
  INT32 (*FACEPROC_SM_DeleteHandle)(HSMILE hSM);
  HSMRESULT (*FACEPROC_SM_CreateResultHandle)(void);
  INT32 (*FACEPROC_SM_DeleteResultHandle)(HSMRESULT hSmResult);
  INT32 (*FACEPROC_SM_SetPointFromHandle)(HSMILE hSM, HPTRESULT hPtResult);
  INT32 (*FACEPROC_SM_GetResult)(HSMRESULT hSmResult, INT32 *pnSmile,
    INT32 *pnConfidence);
  INT32 (*FACEPROC_SM_Estimate)(HSMILE hSM, RAWIMAGE *pImage, INT32 nWidth,
    INT32 nHeight, HSMRESULT hSmResult);

  /* FaceProcFR API */

  HFEATURE (*FACEPROC_FR_CreateFeatureHandle)( void );
  INT32 (*FACEPROC_FR_DeleteFeatureHandle)(HFEATURE hFeature);
  HALBUM (*FACEPROC_FR_CreateAlbumHandle)(INT32 nMaxUserNum,
    INT32 nMaxDataNumPerUser);
  INT32 (*FACEPROC_FR_DeleteAlbumHandle)(HALBUM hAlbum);
  INT32 (*FACEPROC_FR_ExtractFeatureFromPtHdl)(HFEATURE hFeature,
    RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight,
    HPTRESULT hPtResult);
  INT32 (*FACEPROC_FR_RegisterData)(HALBUM hAlbum, HFEATURE hFeature,
    INT32 nUserID,  INT32 nDataID);
  INT32 (*FACEPROC_FR_GetRegisteredUserNum)(HALBUM hAlbum,
    INT32 *pnUserNum);
  INT32 (*FACEPROC_FR_GetRegisteredUsrDataNum)(HALBUM hAlbum,
    INT32 nUserID,
    INT32 *pnUserDataNum);
  INT32 (*FACEPROC_FR_ClearAlbum)(HALBUM hAlbum);
  INT32 (*FACEPROC_FR_GetSerializedAlbumSize)(HALBUM hAlbum,
    UINT32 *punSerializedAlbumSize);
  INT32 (*FACEPROC_FR_SerializeAlbum)(HALBUM hAlbum, UINT8 *pbyBuffer,
    UINT32 unBufSize);
  HALBUM (*FACEPROC_FR_RestoreAlbum)(UINT8 *pbyBuffer, UINT32 unBufSize,
    FR_ERROR *pError);
  INT32 (*FACEPROC_FR_Identify)(HFEATURE hFeature, HALBUM hAlbum,
    INT32 nMaxResultNum, INT32 anUserID[], INT32 anScore[],
    INT32 *pnResultNum);
  INT32 (*FACEPROC_FR_Verify)(HFEATURE hFeature, HALBUM hAlbum,
    INT32 nUserID, INT32 *pnScore );
  INT32 (*FACEPROC_FR_IsRegistered)(HALBUM hAlbum, INT32 nUserID,
    INT32 nDataID, BOOL *pIsRegistered);

  /* Gaze/Blink Detect */
  HGAZEBLINK (*FACEPROC_GB_CreateHandle)(void);
  INT32 (*FACEPROC_GB_DeleteHandle)(HGAZEBLINK hGB);
  HGBRESULT (*FACEPROC_GB_CreateResultHandle)(void);
  INT32 (*FACEPROC_GB_DeleteResultHandle)(HGBRESULT hGbResult);
  INT32 (*FACEPROC_GB_SetPointFromHandle)(HGAZEBLINK hGB,
    HPTRESULT hPtResult);
  INT32 (*FACEPROC_GB_Estimate)(HGAZEBLINK hGB, RAWIMAGE *pImage,
    INT32 nWidth,
    INT32 nHeight, HGBRESULT hGbResult);
  INT32 (*FACEPROC_GB_GetEyeCloseRatio)(HGBRESULT hGbResult,
    INT32 *pnCloseRatioLeftEye, INT32 *pnCloseRatioRightEye);
  INT32 (*FACEPROC_GB_GetGazeDirection)(HGBRESULT hGbResult,
    INT32 *pnGazeLeftRight, INT32 *pnGazeUpDown);
  INT32 (*FACEPROC_SetDtLostParam)(HDETECTION hDT, INT32 nFaceRetryCount,
    INT32 nHeadRetryCount, INT32 nHoldCount);
  INT32 (*FACEPROC_SetDtModifyMoveRate)(HDETECTION hDT, INT32 nModifyMoveRate);
  INT32 (*FACEPROC_DtLockID)(HDTRESULT hDtResult, INT32 nID);
} faceproc_lib_t;

/** faceproc_comp_t
 *   @b: base image component
 *   @abort_flag: Flag to indicate whether the abort is issued
 *   @width : Variable to hold the width of the last frame
 *   @height : Variable to hold the height of the last frame
 *   @hdt: face detection handle
 *   @hresult: face detection result handle
 *   @hpt: Face parts detection handle
 *   @hptresult: face parts result handle
 *   @hct: countour handle
 *   @hctresult: contour result handle
 *   @hsm: smile detection handle
 *   @hsmresult: smile detection result handle
 *   @hfeature: face feature data handle
 *   @halbum: album data handle
 *   @hgb: gaze/blink estimation handle
 *   @hgbresult: gaze/blink result handle
 *   @bmem: backup memory
 *   @wmem: work memory
 *   @fdBmemorySet: flag to indicate if backup memory is set
 *   @fdWmemorySet: flag to indicate if work memory is set
 *   @recognized: flag to indicate if face is recognized
 *   @last_img_registered_idx: user id of the last registered
 *                           person
 *   @user_id: array of user ids
 *   @confidence: array of confidence degree of the users
 *   @p_lib: pointer to the faceproc library
 *   @config: faceproc configuration
 *   @config_set: flag to indicate if the config is set
 *   @mode: faceproc mode of execution
 *   @facedrop_cnt: face detection drop count
 *   @trans_info: translation info for the face cordinates
 *   @fd_chromatix: chromatix values for FD configuration
 *   @frame_id: Frame id of the processed buffer
 *   Faceproc component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  int abort_flag;

  /* Frame dimension */
  uint32_t width;
  uint32_t height;

  /* FaceprocEngine-specific fields */
  HDETECTION hdt;
  HDTRESULT hresult;
  HPOINTER hpt;
  HPTRESULT hptresult[MAX_FACE_ROI];
  HCONTOUR hct;
  HCTRESULT hctresult[MAX_FACE_ROI];
  HSMILE hsm;
  HSMRESULT hsmresult[MAX_FACE_ROI];
  HFEATURE hfeature;
  HALBUM halbum;
  HGAZEBLINK hgb;
  HGBRESULT hgbresult[MAX_FACE_ROI];
  /* Scratch buffers */
  void *bmem;  /* backup memory */
  void *wmem;  /* work memory */
  /* Flags */
  uint8_t fdBmemorySet;
  uint8_t fdWmemorySet;
  int recognized[MAX_FACE_ROI];
  int last_img_registered_idx;
  INT32 user_id[MAX_REGISTER_PPL];
  int confidence[MAX_REGISTER_PPL];

  /* pointer to the library */
  faceproc_lib_t *p_lib;

  faceproc_config_t config;
  int8_t config_set;
  faceproc_mode_t mode;
  int facedrop_cnt;
  img_trans_info_t trans_info;
  fd_chromatix_t fd_chromatix;
  int frame_id;
} faceproc_comp_t;


/* Prototypes of the faceproc engine functions */

/** faceproc_comp_eng_config
 *   @p_comp: pointer to the faceproc component
 *
 *   Configures the faceproc engine
 **/
int faceproc_comp_eng_config(faceproc_comp_t *p_comp);

/** faceproc_comp_eng_exec
 *   @p_comp: pointer to the faceproc component
 *   @p_frame: pointer to the frame
 *
 *   Main face proc execution function. This function needs to
 *   be called in the context of the component thread
 **/
int faceproc_comp_eng_exec(faceproc_comp_t *p_comp,
  img_frame_t *p_frame);

/** faceproc_comp_eng_get_output
 *   @p_comp: pointer to the faceproc component
 *   @p_res: pointer to the faceproc result
 *
 *   This function returns the results of faceproc for the last
 *   frame.
 *
 *   Note that this function needs to be called after the
 *   execution is completed
 **/
int faceproc_comp_eng_get_output(faceproc_comp_t *p_comp,
  faceproc_result_t *p_res);

/** faceproc_comp_eng_exec
 *   @p_comp: pointer to the faceproc component
 *
 *   Destroys the faceproc engine
 **/
int faceproc_comp_eng_destroy(faceproc_comp_t *p_comp);

/** faceproc_comp_eng_reset
 *   @p_comp: pointer to the faceproc component
 *
 *   Resets the faceproc engine
 **/
int faceproc_comp_eng_reset(faceproc_comp_t *p_comp);

/** faceproc_comp_eng_unload
 *   @p_lib: pointer to the faceproc library
 *
 *   Unloads the faceproclibrary
 **/
void faceproc_comp_eng_unload(faceproc_lib_t *p_lib);

/** faceproc_comp_eng_load
 *   @p_lib: pointer to the faceproc library
 *
 *   Loads the faceproclibrary
 **/
int faceproc_comp_eng_load(faceproc_lib_t *p_lib);

#endif //__FD_COMP_H__
