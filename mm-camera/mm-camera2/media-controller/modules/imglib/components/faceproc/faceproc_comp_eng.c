/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/
#include <dlfcn.h>
#include <math.h>

#include "faceproc_comp.h"
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>

#ifdef FD_PROFILE
#define FACEPROC_START_MEASURE \
  struct timeval start_time, mid_time, end_time;\
  gettimeofday(&start_time, NULL); \
  mid_time = start_time \

#define FACEPROC_MIDDLE_TIME \
do { \
  gettimeofday(&end_time, NULL); \
  IDBG_ERROR("%s]%d Middle mtime  %lu ms",  __func__, __LINE__, \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((mid_time.tv_sec * 1000) + (mid_time.tv_usec / 1000))); \
  mid_time = end_time; \
} while (0)\

#define FACEPROC_END_MEASURE \
do { \
  gettimeofday(&end_time, NULL); \
  IDBG_ERROR("End of measure Total %lu ms", \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((start_time.tv_sec * 1000) + (start_time.tv_usec / 1000))); \
} while (0) \

#else
#define FACEPROC_START_MEASURE \
  do{}while(0) \

#define FACEPROC_MIDDLE_TIME \
  do{}while(0) \

#define FACEPROC_END_MEASURE \
  do{}while(0) \

#endif

const char FACE_ALBUM[] =  "/data/fdAlbum";

#define FD_MAX_DUMP_CNT 10
static unsigned frame_count = 0;
static const unsigned skip_count = 10;
static const unsigned initial_skip_count = 5;
static unsigned dump_count = 0;

/**
 * Function: faceproc_comp_eng_unload
 *
 * Description: Unload the faceproc library
 *
 * Input parameters:
 *   p_lib - The pointer to the faceproc lib object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
void faceproc_comp_eng_unload(faceproc_lib_t *p_lib)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (p_lib->ptr) {
    dlclose(p_lib->ptr);
    p_lib->ptr = NULL;
  }
  memset(p_lib, 0, sizeof(faceproc_lib_t));
}

/**
 * Function: faceproc_comp_eng_load
 *
 * Description: Loads the faceproc library
 *
 * Input parameters:
 *   p_lib - The pointer to the faceproc lib object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_load(faceproc_lib_t *p_lib)
{
  int rc = 0;
  p_lib->ptr =
    dlopen("libmmcamera_faceproc.so", RTLD_NOW);
  if (!p_lib->ptr) {
    IDBG_ERROR("%s:%d] Error opening libmmcamera_faceproc.so lib",
      __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DeleteDtResult) =
      dlsym(p_lib->ptr, "FACEPROC_DeleteDtResult");
    if (p_lib->FACEPROC_DeleteDtResult == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_DeleteDtResult ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DeleteDetection) =
      dlsym(p_lib->ptr, "FACEPROC_DeleteDetection");
    if (p_lib->FACEPROC_DeleteDetection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_DeleteDetection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtFaceCount) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtFaceCount");
    if (p_lib->FACEPROC_GetDtFaceCount == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtFaceCount ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtFaceInfo) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtFaceInfo");
    if (p_lib->FACEPROC_GetDtFaceInfo == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtFaceInfo ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtMemorySize) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtMemorySize");
    if (p_lib->FACEPROC_SetDtMemorySize == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtMemorySize ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtRequiredMovieMemSize) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtRequiredMovieMemSize");
    if (p_lib->FACEPROC_GetDtRequiredMovieMemSize == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtRequiredMovieMemSize ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CreateDetection) =
      dlsym(p_lib->ptr, "FACEPROC_CreateDetection");
    if (p_lib->FACEPROC_CreateDetection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_CreateDetection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtMode) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtMode");
    if (p_lib->FACEPROC_SetDtMode == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtMode ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtStep) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtStep");
    if (p_lib->FACEPROC_SetDtStep == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtStep ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtAngle) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtAngle");
    if (p_lib->FACEPROC_SetDtAngle == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtAngle ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtDirectionMask) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtDirectionMask");
    if (p_lib->FACEPROC_SetDtDirectionMask == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtDirectionMask ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtFaceSizeRange) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtFaceSizeRange");
    if (p_lib->FACEPROC_SetDtFaceSizeRange == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtFaceSizeRange ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
    if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtThreshold) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtThreshold");
    if (p_lib->FACEPROC_SetDtThreshold == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtThreshold ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CreateDtResult) =
      dlsym(p_lib->ptr, "FACEPROC_CreateDtResult");
    if (p_lib->FACEPROC_CreateDtResult == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_CreateDtResult ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_Detection) =
      dlsym(p_lib->ptr, "FACEPROC_Detection");
    if (p_lib->FACEPROC_Detection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_Detection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FreeBMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_FreeBMemoryArea");
    if (p_lib->FACEPROC_FreeBMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FreeBMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FreeWMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_FreeWMemoryArea");
    if (p_lib->FACEPROC_FreeWMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FreeWMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetBMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_SetBMemoryArea");
    if (p_lib->FACEPROC_SetBMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetBMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetWMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_SetWMemoryArea");
    if (p_lib->FACEPROC_SetWMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetWMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtRefreshCount) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtRefreshCount");
    if (p_lib->FACEPROC_SetDtRefreshCount == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetDtRefreshCount error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  /*Pt detection */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_CreateHandle");
    if (p_lib->FACEPROC_PT_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DeleteHandle");
    if (p_lib->FACEPROC_PT_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_CreateResultHandle");
    if (p_lib->FACEPROC_PT_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DeleteResultHandle");
    if (p_lib->FACEPROC_PT_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetPositionFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_SetPositionFromHandle");
    if (p_lib->FACEPROC_PT_SetPositionFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetPositionFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetMode) =
      dlsym(p_lib->ptr,
      "FACEPROC_PT_SetMode");
    if (p_lib->FACEPROC_PT_SetMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
   if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetConfMode) =
      dlsym(p_lib->ptr, "FACEPROC_PT_SetConfMode");
    if (p_lib->FACEPROC_PT_SetConfMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetConfMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DetectPoint) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DetectPoint");
    if (p_lib->FACEPROC_PT_DetectPoint == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DetectPoint error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_PT_GetResult");
    if (p_lib->FACEPROC_PT_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_GetFaceDirection) =
      dlsym(p_lib->ptr, "FACEPROC_PT_GetFaceDirection");
    if (p_lib->FACEPROC_PT_GetFaceDirection == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_GetFaceDirection error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_CreateHandle");
    if (p_lib->FACEPROC_CT_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DeleteHandle");
    if (p_lib->FACEPROC_CT_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_CreateResultHandle");
    if (p_lib->FACEPROC_CT_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DeleteResultHandle");
    if (p_lib->FACEPROC_CT_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetPointFromHandle");
    if (p_lib->FACEPROC_CT_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DetectContour) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DetectContour");
    if (p_lib->FACEPROC_CT_DetectContour == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DetectContour error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_CT_GetResult");
    if (p_lib->FACEPROC_CT_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetDetectionMode) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetDetectionMode");
    if (p_lib->FACEPROC_CT_SetDetectionMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Smile API */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_CreateHandle");
    if (p_lib->FACEPROC_SM_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_DeleteHandle");
    if (p_lib->FACEPROC_SM_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_CreateResultHandle");
    if (p_lib->FACEPROC_SM_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_DeleteResultHandle");
    if (p_lib->FACEPROC_SM_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_SetPointFromHandle");
    if (p_lib->FACEPROC_SM_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_Estimate) =
      dlsym(p_lib->ptr, "FACEPROC_SM_Estimate");
    if (p_lib->FACEPROC_SM_Estimate == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_Estimate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_SM_GetResult");
    if (p_lib->FACEPROC_SM_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetDetectionMode) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetDetectionMode");
    if (p_lib->FACEPROC_CT_SetDetectionMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Face Recognition */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_CreateFeatureHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_CreateFeatureHandle");
    if (p_lib->FACEPROC_FR_CreateFeatureHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_CreateFeatureHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_DeleteFeatureHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_DeleteFeatureHandle");
    if (p_lib->FACEPROC_FR_DeleteFeatureHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_DeleteFeatureHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_CreateAlbumHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_CreateAlbumHandle");
    if (p_lib->FACEPROC_FR_CreateAlbumHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_CreateAlbumHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_DeleteAlbumHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_DeleteAlbumHandle");
    if (p_lib->FACEPROC_FR_DeleteAlbumHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_DeleteAlbumHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl) =
      dlsym(p_lib->ptr, "FACEPROC_FR_ExtractFeatureFromPtHdl");
    if (p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_ExtractFeatureFromPtHdl error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_RegisterData) =
      dlsym(p_lib->ptr, "FACEPROC_FR_RegisterData");
    if (p_lib->FACEPROC_FR_RegisterData == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_RegisterData error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetRegisteredUserNum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetRegisteredUserNum");
    if (p_lib->FACEPROC_FR_GetRegisteredUserNum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetRegisteredUserNum error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetRegisteredUsrDataNum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetRegisteredUsrDataNum");
    if (p_lib->FACEPROC_FR_GetRegisteredUsrDataNum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetRegisteredUsrDataNum error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_ClearAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_ClearAlbum");
    if (p_lib->FACEPROC_FR_ClearAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_ClearAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetSerializedAlbumSize) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetSerializedAlbumSize");
    if (p_lib->FACEPROC_FR_GetSerializedAlbumSize == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetSerializedAlbumSize error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_SerializeAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_SerializeAlbum");
    if (p_lib->FACEPROC_FR_SerializeAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_SerializeAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_RestoreAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_RestoreAlbum");
    if (p_lib->FACEPROC_FR_RestoreAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_RestoreAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_Identify) =
      dlsym(p_lib->ptr, "FACEPROC_FR_Identify");
    if (p_lib->FACEPROC_FR_Identify == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_Identify error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_Verify) =
      dlsym(p_lib->ptr, "FACEPROC_FR_Verify");
    if (p_lib->FACEPROC_FR_Verify == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_Verify error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_IsRegistered) =
      dlsym(p_lib->ptr, "FACEPROC_FR_IsRegistered");
    if (p_lib->FACEPROC_FR_IsRegistered == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_IsRegistered error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Blink Detect */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_CreateHandle");
    if (p_lib->FACEPROC_GB_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_DeleteHandle");
    if (p_lib->FACEPROC_GB_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_CreateResultHandle");
    if (p_lib->FACEPROC_GB_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_DeleteResultHandle");
    if (p_lib->FACEPROC_GB_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_SetPointFromHandle");
    if (p_lib->FACEPROC_GB_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_Estimate) =
      dlsym(p_lib->ptr, "FACEPROC_GB_Estimate");
    if (p_lib->FACEPROC_GB_Estimate == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_Estimate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_GetEyeCloseRatio) =
      dlsym(p_lib->ptr, "FACEPROC_GB_GetEyeCloseRatio");
    if (p_lib->FACEPROC_GB_GetEyeCloseRatio == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_GetEyeCloseRatio error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_GetGazeDirection) =
      dlsym(p_lib->ptr, "FACEPROC_GB_GetGazeDirection");
    if (p_lib->FACEPROC_GB_GetGazeDirection == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_GetGazeDirection error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtLostParam) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtLostParam");
    if (p_lib->FACEPROC_SetDtLostParam == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_SetDtLostParam error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtModifyMoveRate) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtModifyMoveRate");
    if (p_lib->FACEPROC_SetDtModifyMoveRate == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_SetDtModifyMoveRate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DtLockID) =
      dlsym(p_lib->ptr, "FACEPROC_DtLockID");
    if (p_lib->FACEPROC_DtLockID == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_DtLockID error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (rc < 0) {
    faceproc_comp_eng_unload(p_lib);
  }
  return rc;
}

/**
 * Function: faceproc_register_frame
 *
 * Description: Register the frame to faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc object
 *   p_input_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_register_frame(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  int num_faces)
{
  INT32 i;
  INT32 pn_user_data_num;
  INT32 user_idx, data_idx;
  int rc = 0;

  if (num_faces <= 0)
    return IMG_SUCCESS;
  if (p_comp->last_img_registered_idx >= MAX_REGISTER_PPL)
    return IMG_ERR_GENERAL;

  for (i = 0; i < num_faces; i++) {
    if (p_comp->recognized[i] == 0) {
      IDBG_MED("Face not registered. Adding new Entry");
      user_idx = (p_comp->last_img_registered_idx % MAX_REGISTER_PPL);
      p_comp->user_id[i] = user_idx;
      p_comp->last_img_registered_idx++;
      data_idx = 0;
    } else {
      IDBG_MED("Face already registered. Updating Entry");
      rc = p_comp->p_lib->
        FACEPROC_FR_GetRegisteredUsrDataNum(p_comp->halbum,
        p_comp->user_id[i],
        &pn_user_data_num);
      user_idx = p_comp->user_id[i];
      data_idx = pn_user_data_num % MAX_REGISTER_IMG_PER_PERSON;
      IDBG_MED("value of pn_user_data_num %d", pn_user_data_num);
    }

    IDBG_MED("REGISTERING DATA AT user_id %d dataid %d ",
      user_idx, data_idx);
    rc = p_comp->p_lib->FACEPROC_FR_RegisterData(p_comp->halbum,
      p_comp->hfeature, user_idx, data_idx);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_save_album
 *
 * Description: Save Registered faces album
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_save_album(faceproc_comp_t *p_comp)
{
  FILE *fp;
  uint8_t *buffer = NULL;
  UINT32 punSerializedAlbumSize = 0;
  int rc;

  if (!(p_comp && p_comp->halbum))  {
    IDBG_ERROR("%s:%d] Error No album ", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  rc = p_comp->p_lib->FACEPROC_FR_GetSerializedAlbumSize(p_comp->halbum,
    (UINT32 *)&punSerializedAlbumSize);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }

  if (punSerializedAlbumSize > 0 && punSerializedAlbumSize > MAX_ALBUM_SIZE) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d %d",
      __func__, __LINE__, rc, punSerializedAlbumSize);
    return IMG_ERR_GENERAL;
  }
  buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
  if (NULL == buffer) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }
  memset(buffer, 0, MAX_ALBUM_SIZE);
  rc = p_comp->p_lib->FACEPROC_FR_SerializeAlbum(p_comp->halbum, buffer,
    MAX_ALBUM_SIZE);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_SerializeAlbum failed %d",
      __func__, __LINE__, rc);
    free(buffer);
    return IMG_ERR_GENERAL;
  }

  fp = fopen(FACE_ALBUM, "wb");
  if (fp != NULL) {
    fwrite(buffer, 1, MAX_ALBUM_SIZE, fp);
    IDBG_MED("%s:%d] Writing to /data", __func__, __LINE__);
    fclose(fp);
  }

  free(buffer);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fd_output
 *
 * Description: Gets the frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_input_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data,
  INT32 *num_faces)
{
  POINT apt_point[PT_POINT_KIND_MAX];
  INT32 an_confidence[PT_POINT_KIND_MAX];
  int rc, leye_close_ratio, reye_close_ratio;
  int pn_gaze_left_right, pn_gaze_up_down;
  uint32_t i, j;
  faceproc_info_t *p_output = NULL;

  /* FD START */
  /* Get the number of faces */
  rc = p_comp->p_lib->FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("%s:%d] num_faces %d", __func__, __LINE__, *num_faces);

  /* Parse and store the faces */
  fd_data->num_faces_detected = (uint32_t)*num_faces;
  fd_data->frame_id = p_comp->frame_id;

  if (fd_data->num_faces_detected > MAX_FACE_ROI)
    fd_data->num_faces_detected = MAX_FACE_ROI;

  if (!fd_data->num_faces_detected) {
    for (i = 0; i < MAX_FACE_ROI; i++) {
      p_output = &fd_data->roi[i];
      p_output->blink_detected = 0;
      p_output->left_blink = 0;
      p_output->right_blink = 0;
      p_output->left_right_gaze = 0;
      p_output->top_bottom_gaze = 0;
    }
  }
  for (i = 0; i < fd_data->num_faces_detected; i++) {
    FACEINFO face_info;
    int left, top, right, bottom;
    rc = p_comp->p_lib->FACEPROC_GetDtFaceInfo(p_comp->hresult, i,
      &face_info);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GetDtFaceInfo returned error: %d", (uint32_t)rc);
      fd_data->num_faces_detected--;
      return IMG_ERR_GENERAL;
    }

    if ( p_comp->fd_chromatix.lock_faces && face_info.nID > 0) {
      rc = p_comp->p_lib->FACEPROC_DtLockID(p_comp->hresult, face_info.nID);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("FACEPROC_DtLockID returned error: %d", (uint32_t)rc);
        return IMG_ERR_GENERAL;
      }
    }

    /* Translate the data */
    /* Clip each detected face coordinates to be within the frame boundary */
    CLIP(face_info.ptLeftTop.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptRightTop.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptLeftBottom.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptRightBottom.x, 0,
      (int32_t)p_comp->width);
    CLIP(face_info.ptLeftTop.y, 0, (int32_t)p_comp->height);
    CLIP(face_info.ptRightTop.y, 0, (int32_t)p_comp->height);
    CLIP(face_info.ptLeftBottom.y, 0,
      (int32_t)p_comp->height);
    CLIP(face_info.ptRightBottom.y, 0,
      (int32_t)p_comp->height);

    /* Find the bounding box */
    left = MIN4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    top = MIN4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);
    right = MAX4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    bottom = MAX4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);
    p_output = &fd_data->roi[i];
    p_output->gaze_angle = face_info.nPose;
    p_output->face_boundary.x = left;
    p_output->face_boundary.y = top;
    p_output->face_boundary.dx = right - left;
    p_output->face_boundary.dy = bottom - top;
    p_output->unique_id = abs(face_info.nID);
    p_output->fd_confidence = face_info.nConfidence;


  if (FD_FACEPT_ENABLE(p_comp)) {
    /* Pt result */
#if(FACE_PART_DETECT)
    rc = p_comp->p_lib->FACEPROC_PT_GetResult(p_comp->hptresult[i],
      PT_POINT_KIND_MAX, (POINT *)p_output->fp.face_pt,
      p_output->fp.confidence);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: PT_GetResult failed", __func__);
      return IMG_ERR_GENERAL;
    }
    rc = p_comp->p_lib->FACEPROC_PT_GetFaceDirection(p_comp->hptresult[i],
      &(p_output->fp.direction_up_down),
      &(p_output->fp.direction_left_right),
      &(p_output->fp.direction_roll));
    IDBG_MED("%s: PT cord[%d] LEFT_EYE %d, %d  up %d, left %d, roll %d",
      __func__, i, p_output->fp.face_pt[PT_POINT_LEFT_EYE].x,
      p_output->fp.face_pt[PT_POINT_LEFT_EYE].y,
      p_output->fp.direction_up_down,
      p_output->fp.direction_left_right,
      p_output->fp.direction_roll);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: eng_FACEPROC_PT_GetFaceDirection failed", __func__);
      return IMG_ERR_GENERAL;
    }
#endif
#if(FACE_CONTOUR_DETECT)
    rc = p_comp->p_lib->FACEPROC_CT_GetResult(p_comp->hctresult[i],
      CT_POINT_KIND_MAX, (POINT *)&p_output->ct);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: CT_GetResult failed", __func__);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("%s: CT Coordinates[%d] for CT_POINT_EYE_L_UP %d, %d", __func__, i,
      p_output->ct.contour_pt[CT_POINT_EYE_L_UP].x,
      p_output->ct.contour_pt[CT_POINT_EYE_L_UP].y);
    IDBG_MED("%s: CT Coordinates[%d] for CT_POINT_EYE_L_DOWN %d, %d", __func__, i,
      p_output->ct.contour_pt[CT_POINT_EYE_L_DOWN].x,
      p_output->ct.contour_pt[CT_POINT_EYE_L_DOWN].y);

#endif
#if(FACE_SMILE_DETECT)
    rc = p_comp->p_lib->FACEPROC_SM_GetResult(p_comp->hsmresult[i],
      &(p_output->sm.smile_degree), &(p_output->sm.confidence));
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: CT_GetResult failed", __func__);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("%s: SM Coordinates[%d] for %d, %d", __func__, i,
      p_output->sm.smile_degree,
      p_output->sm.confidence);
#endif
    p_output->blink_detected = 0;
#if(FACE_BGS_DETECT)
    rc = p_comp->p_lib->FACEPROC_GB_GetEyeCloseRatio(p_comp->hgbresult[i],
      &(leye_close_ratio), &(reye_close_ratio));
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: eng_FACEPROC_GB_GetEyeCloseRatio failed", __func__);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("%s: BLINK Ratio  l %d, r %d", __func__, leye_close_ratio,
      reye_close_ratio);
    p_output->left_blink = leye_close_ratio;
    p_output->right_blink = reye_close_ratio;
    if (leye_close_ratio > 600 &&
      reye_close_ratio > 600) {
      IDBG_MED("EYES CLOSED");
      p_output->blink_detected = 1;
    }
    rc = p_comp->p_lib->FACEPROC_GB_GetGazeDirection(p_comp->hgbresult[i],
      &(pn_gaze_left_right),&(pn_gaze_up_down));
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s: eng_FACEPROC_GB_GetGazeDirection failed", __func__);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("%s: Gaze Ratio  left_right %d, top_bottom %d", __func__,
      pn_gaze_left_right,
      pn_gaze_up_down);
    p_output->left_right_gaze = pn_gaze_left_right;
    p_output->top_bottom_gaze = pn_gaze_up_down;
#endif
    }
  }  /* end of forloop */
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fr_output
 *
 * Description: Gets the frameproc recognition output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_input_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fr_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data, INT32 num_faces)
{
  int i;
  faceproc_info_t *p_output;
  for (i=0; i < num_faces;i++) {
    p_output = &fd_data->roi[i];
    p_output->is_face_recognised = p_comp->recognized[i];
    p_output->unique_id = p_comp->user_id[i];
    p_output->fd_confidence = p_comp->confidence[i];
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_register_output
 *
 * Description: Register frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - pointer to frame out data
 *   num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_register_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data, INT32 num_faces)
{
  int i;
  for (i = 0; i < num_faces; i++) {
    fd_data->roi[i].unique_id = p_comp->user_id[i];
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_frame_dump
 *
 * Description: Dump the faceproc frame
 *
 * Input parameters:
 *   p_frame - pointer to input frame
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void faceproc_frame_dump(img_frame_t *p_frame)
{
  char filename[50];
  FILE *fp;

  if ((dump_count > 10) || (frame_count <= initial_skip_count) ||
    (frame_count % skip_count != 0))
    return;

  snprintf(filename, sizeof(filename), "/data/img/fd_in_%dx%d_%d.yuv",
    FD_WIDTH(p_frame), FD_HEIGHT(p_frame), frame_count);

  IDBG_ERROR("FACEPROC dump frame %s \n", filename);

  fp = fopen(filename, "w+");
  int rc = 0;
  if (fp) {
    rc = fwrite(FD_ADDR(p_frame), 1, FD_WIDTH(p_frame) *
      FD_HEIGHT(p_frame), fp);
    fclose(fp);
    dump_count++;
  }
}

/**
 * Function: faceproc_fd_execute
 *
 * Description: Executes the face detecttion algorithm
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_input_frame - pointer to input frame
 *   num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_execute(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  INT32 * num_faces)
{
  ATRACE_BEGIN("Camera:Faceproc");
  INT32 i;
  IDBG_MED("%s:%d] E", __func__, __LINE__);

  FACEPROC_START_MEASURE;

  int rc = p_comp->p_lib->FACEPROC_Detection(p_comp->hdt,
    (RAWIMAGE *)FD_ADDR(p_frame),
    FD_WIDTH(p_frame),
    FD_HEIGHT(p_frame),
    ACCURACY_HIGH_TR,
    p_comp->hresult);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_Detection returned error: %d", (uint32_t)rc);
    return IMG_ERR_GENERAL;
  }
  /* Set frame id of the processed frame */
  p_comp->frame_id = p_frame->frame_id;

  if (frame_count < 10)
    FACEPROC_END_MEASURE;

  ATRACE_END();
  if (p_comp->config.fd_feature_mask & FACE_PROP_DUMP_FRAMES)
    faceproc_frame_dump(p_frame);


  frame_count++;

  /*Set Position for PT */
  rc = p_comp->p_lib->FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    return IMG_ERR_GENERAL;
  }
  IDBG_LOW("%s:%d] num faces %d", __func__, __LINE__, *num_faces);
  if (*num_faces <= 0) {
    IDBG_MED("%s:%d] no faces detected X", __func__, __LINE__);
    return IMG_SUCCESS;
  }
  for (i = 0; ((i < *num_faces) && (i < MAX_FACE_ROI)); i++) {

  if (FD_FACEPT_ENABLE(p_comp)) {
#if(FACE_PART_DETECT)
    rc = p_comp->p_lib->FACEPROC_PT_SetPositionFromHandle(p_comp->hpt,
      p_comp->hresult, i);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_PT_SetPositionFromHandle returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    rc = p_comp->p_lib->FACEPROC_PT_DetectPoint(p_comp->hpt,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_PT_DetectPoint returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
#endif
#if(FACE_CONTOUR_DETECT)
    rc = p_comp->p_lib->FACEPROC_CT_SetPointFromHandle(p_comp->hct,
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_CT_SetPositionFromHandle returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    rc = p_comp->p_lib->FACEPROC_CT_DetectContour(p_comp->hct,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      p_comp->hctresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_PT_DetectPoint returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
#endif
#if(FACE_BGS_DETECT)
    rc = p_comp->p_lib->FACEPROC_SM_SetPointFromHandle(p_comp->hsm,
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_SM_SetPointFromHandle returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    rc = p_comp->p_lib->FACEPROC_SM_Estimate(p_comp->hsm,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      p_comp->hsmresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_SM_Estimate returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    /* Blink/Gaze Detection */
    rc = p_comp->p_lib->FACEPROC_GB_SetPointFromHandle(p_comp->hgb,
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_GB_SetPointFromHandle returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    rc = p_comp->p_lib->FACEPROC_GB_Estimate(p_comp->hgb,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      p_comp->hgbresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_GB_Estimate returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
#endif

    }
  }  /* end of forloop */
  IDBG_MED("%s:%d] X", __func__, __LINE__);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fr_execute
 *
 * Description: Executes the face recognition algorithm
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_input_frame - pointer to input frame
 *   num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fr_execute(faceproc_comp_t *p_comp,
  img_frame_t *p_frame,
  int num_faces)
{
  int i, rc;
  INT32 an_user_id[MAX_REGISTER_PPL], pn_user_num, pn_user_data_num;
  INT32 an_score[MAX_REGISTER_PPL], pn_result_num;
  BOOL is_registered;

  if (num_faces <= 0)
    return IMG_SUCCESS;

  /* FR START */
  for (i = 0; i < num_faces; i++) {
    p_comp->user_id[i] = -1;
    p_comp->confidence[i] = 0;
#if(FACE_RECOGNITION)
    rc = p_comp->p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl(p_comp->hfeature,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_FR_ExtractFeatureFromPtHdl returned error: %d",
        (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
#if(FACE_RECOGNIZE_TEST_REGISTER)
    /*TODO move recognie to different fn */
    rc = p_comp->p_lib->FACEPROC_FR_RegisterData(p_comp->halbum,
      p_comp->hfeature, i, 0);
    return IMG_SUCCESS;
#endif

    /* Find if album is empty */
    rc = p_comp->p_lib->FACEPROC_FR_GetRegisteredUserNum(p_comp->halbum,
      &pn_user_num);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_FR_GetRegisteredUsrNum returned error: %d",
      (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    p_comp->last_img_registered_idx = pn_user_num;
    if (!pn_user_num) {
      IDBG_ERROR("Face Recognition database/Album is empty");
      return IMG_ERR_NOT_FOUND;
    } else
      IDBG_MED("Number of User entries in album is %d", pn_user_num);

    if (!p_comp->hfeature ||!p_comp->halbum)
      IDBG_ERROR("hfeature and halbum is null, failed");

    IDBG_MED("Executing Face Recognition");
    rc = p_comp->p_lib->FACEPROC_FR_Identify(p_comp->hfeature,
      p_comp->halbum, MAX_REGISTER_PPL,
      an_user_id, an_score, &pn_result_num);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_FR_Identify returned error: %d", (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    IDBG_ERROR("%s USER ID %d  score %d", __func__, an_user_id[0], an_score[0]);
    if (pn_result_num <= MAX_REGISTER_PPL && (an_score[0] >= FR_THRESHOLD)) {
      p_comp->recognized[i] = 1;
      p_comp->user_id[i] = an_user_id[0];
      p_comp->confidence[i] = an_score[0];
    } else {
      p_comp->recognized[i] = 0;
      p_comp->user_id[i] = -1;
      p_comp->confidence[i] = 0;
    }
#endif
    /*FR END */
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_get_angle
 *
 * Description: Get the faceproc angle
 *
 * Input parameters:
 *   angle: face detection angle macro
 *   angle_bitmask - face detection angle bitmask
 *
 * Return values:
 *     bitmask for engine angle
 *
 * Notes: none
 **/
uint32_t faceproc_comp_eng_get_angle(fd_chromatix_angle_t angle,
  uint16_t angle_bitmask)
{
  switch (angle) {
  case FD_ANGLE_ALL:
    return ANGLE_ALL;
  case FD_ANGLE_15_ALL:
    return (ANGLE_0 | ANGLE_3 | ANGLE_6 | ANGLE_9);
  case FD_ANGLE_45_ALL:
    return (ANGLE_0 | ANGLE_1 | ANGLE_2 |
      ANGLE_3 | ANGLE_4 | ANGLE_8 |
      ANGLE_9 |ANGLE_10 | ANGLE_11);
  case FD_ANGLE_MANUAL:
    /* todo */
    return ANGLE_NONE;
  default:
  case FD_ANGLE_NONE:
    return ANGLE_NONE;
  }
  return ANGLE_NONE;
}

/**
 * Function: faceproc_comp_eng_face_size
 *
 * Description: Get the face size
 *
 * Input parameters:
 *   face_adj_type: type of face dimension calculation
 *   face_size: face size for fixed adjustment
 *   ratio: facesize ratio for floating adjustment
 *   min_size: minimum face size supported by library
 *   dimension: min(height/width) of the image
 *
 * Return values:
 *     face size based on input parameters
 *
 * Notes: none
 **/
uint32_t faceproc_comp_eng_face_size(fd_face_adj_type_t face_adj_type,
  uint32_t face_size,
  float ratio,
  uint32_t min_size,
  uint32_t dimension)
{
  switch (face_adj_type) {
  case FD_FACE_ADJ_FLOATING: {
    uint32_t size = (float)dimension * ratio;
    size = (size/10) * 10;
    return (size < min_size) ? min_size : size;
  }
  default:
  case FD_FACE_ADJ_FIXED:
    return face_size;
  }
  return face_size;
}

/**
 * Function: faceproc_comp_eng_config
 *
 * Description: Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_cfg - pointer to faceproc_config_t
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_config(faceproc_comp_t *p_comp)
{
  UINT32 an_still_angle[POSE_TYPE_COUNT];
  UINT8 major, minor, *buffer;
  FILE *fp;
  FR_ERROR pError;
  int i;
  UINT32 bmem_size_required = 0, wmem_size_required_min = 0,
    wmem_size_required_max;
  RECT nil_edge = {-1, -1, -1, -1};
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t rotation_range = p_cfg->face_cfg.rotation_range;
  uint32_t max_num_face_to_detect;
  uint32_t min_face_size;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] NULL component", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  min_face_size = faceproc_comp_eng_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    50, /* keeping min facesize as 50 */
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));
  IDBG_HIGH("%s:%d] ###min_face_size %d", __func__, __LINE__,
    min_face_size);

  max_num_face_to_detect = p_comp->fd_chromatix.max_num_face_to_detect;

  if (FD_ANGLE_ENABLE(p_comp)) {
    an_still_angle[POSE_FRONT] = faceproc_comp_eng_get_angle(
      p_comp->fd_chromatix.angle_front,
      p_comp->fd_chromatix.angle_front_bitmask);
    an_still_angle[POSE_HALF_PROFILE] = faceproc_comp_eng_get_angle(
      p_comp->fd_chromatix.angle_half_profile,
      p_comp->fd_chromatix.angle_half_profile_bitmask);
    an_still_angle[POSE_PROFILE] = faceproc_comp_eng_get_angle(
      p_comp->fd_chromatix.angle_full_profile,
      p_comp->fd_chromatix.angle_full_profile_bitmask);
  } else {
    IDBG_HIGH("%s:%d] ###Disable Angle", __func__, __LINE__);
    an_still_angle[POSE_FRONT] = ANGLE_NONE;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_NONE;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;
  }

  /* Determine the size of memory needed */
  rc = p_comp->p_lib->FACEPROC_GetDtRequiredMovieMemSize(
    p_cfg->frame_cfg.max_width,
    p_cfg->frame_cfg.max_height,
    min_face_size,
    max_num_face_to_detect +
    max_num_face_to_detect / 2,
    nil_edge, p_comp->fd_chromatix.search_density_nontracking,
    nil_edge, p_comp->fd_chromatix.search_density_tracking,
    &bmem_size_required,
    &wmem_size_required_min,
    &wmem_size_required_max);

#ifdef FD_USE_INTERNAL_MEM
  /* Allocate scratch memory for Faceproc FD */
  IDBG_MED("GetDtRequiredMovieMemSiz bmem %d wmem %d rc=%d",
    bmem_size_required, wmem_size_required_max, rc);
  /* Pt API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (110 * KB_SIZE);
  /* Face Contour API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (250 * KB_SIZE);
  /* Smile Detect API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (45 * KB_SIZE);
  /* Face Recognition API */
  bmem_size_required += MAX_ALBUM_SIZE;
  wmem_size_required_max += (150 * KB_SIZE);
  IDBG_MED("After GetDtRequiredMovieMemSiz bmem %d wmem %d rc = %d",
    bmem_size_required, wmem_size_required_max, rc);

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_comp->bmem = malloc(bmem_size_required);
  if (NULL == p_comp->bmem) {
    IDBG_ERROR("%s:%d] bmem failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->wmem = malloc(wmem_size_required_max);
  if (NULL == p_comp->bmem) {
    IDBG_ERROR("%s:%d] wmem failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  IDBG_MED("GetDtRequiredMovieMemSiz1 value rc=%d", rc);

  /* Pass the scratch memory to Faceproc's FD */

  rc = (int) p_comp->p_lib->FACEPROC_SetWMemoryArea(p_comp->wmem,
    wmem_size_required_max);
  IDBG_MED("SetWMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] SetWMemoryArea failed %d", __func__, __LINE__, rc);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->fdWmemorySet = TRUE;
  rc = (int) p_comp->p_lib->FACEPROC_SetBMemoryArea(p_comp->bmem,
    bmem_size_required);

  IDBG_MED("SetBMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] SetBMemoryArea failed %d", __func__, __LINE__, rc);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->fdBmemorySet = TRUE;
#endif

  /* Create Faceproc FD handle */
  p_comp->hdt = p_comp->p_lib->FACEPROC_CreateDetection();
  if (!p_comp->hdt) {
    IDBG_ERROR("FACEPROC_CreateDetection failed");
    return IMG_ERR_GENERAL;
  }

  /* Set best Faceproc detection mode for video */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtMode(
    p_comp->hdt, p_comp->fd_chromatix.detection_mode);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtMode failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Set search density */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtStep(
    p_comp->hdt,
    p_comp->fd_chromatix.search_density_nontracking,
    p_comp->fd_chromatix.search_density_tracking);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtStep failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Set Detection Angles */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtAngle(
    p_comp->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtAngle failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Set refresh count */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtRefreshCount(
    p_comp->hdt, p_comp->fd_chromatix.detection_mode,
    p_comp->fd_chromatix.refresh_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtRefreshCount failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  rc = (int) p_comp->p_lib->FACEPROC_SetDtDirectionMask(
    p_comp->hdt, p_comp->fd_chromatix.direction);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtDirectionMask failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height/2)) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, min_face_size,
    p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return IMG_ERR_GENERAL;
  }
  /* Set Detection Threshold */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtThreshold(
    p_comp->hdt, DT_THRESHOLD, DT_THRESHOLD);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Set DT Modify move rate */
  rc = p_comp->p_lib->FACEPROC_SetDtModifyMoveRate(p_comp->hdt,
    p_comp->fd_chromatix.move_rate_threshold);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtModifyMoveRate failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Set DT lost params */
  rc = p_comp->p_lib->FACEPROC_SetDtLostParam(p_comp->hdt,
    p_comp->fd_chromatix.face_retry_count,
    p_comp->fd_chromatix.head_retry_count,
    p_comp->fd_chromatix.hold_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtLostParam failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  /* Create Faceproc result handle */
  p_comp->hresult = p_comp->p_lib->FACEPROC_CreateDtResult(
    max_num_face_to_detect ,
    (max_num_face_to_detect/2));
  if (!(p_comp->hresult)) {
    IDBG_ERROR("FACEPROC_CreateDtResult failed");
    return IMG_ERR_GENERAL;
  }
  /* Create PT handle */
  if (FD_FACEPT_ENABLE(p_comp)) {
#if(FACE_PART_DETECT)
  p_comp->hpt = p_comp->p_lib->FACEPROC_PT_CreateHandle();
  if (!(p_comp->hpt)) {
    IDBG_ERROR("eng_FACEPROC_PT_CreateHandle failed");
    return IMG_ERR_GENERAL;
  }
  for (i = 0; i< MAX_FACE_ROI; i++) {
    p_comp->hptresult[i] = p_comp->p_lib->FACEPROC_PT_CreateResultHandle();
    if (!(p_comp->hptresult[i])) {
      IDBG_ERROR("eng_FACEPROC_PT_CreateResultHandle failed");
      return IMG_ERR_GENERAL;
    }
  }
  rc = (int) p_comp->p_lib->FACEPROC_PT_SetMode(p_comp->hpt, PT_MODE_DEFAULT);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("eng_FACEPROC_PT_SetMode failed");
    return IMG_ERR_GENERAL;
  }
  rc = (int)p_comp->p_lib->FACEPROC_PT_SetConfMode(p_comp->hpt, PT_CONF_NOUSE);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("eng_FACEPROC_PT_SetConfMode failed");
    return IMG_ERR_GENERAL;
  }
#endif
  /* Config CT API */
#if(FACE_CONTOUR_DETECT)
  p_comp->hct = p_comp->p_lib->FACEPROC_CT_CreateHandle();
  if (!(p_comp->hct)) {
    IDBG_ERROR("eng_FACEPROC_CT_CreateHandle failed");
    return IMG_ERR_GENERAL;
  }
  for (i = 0; i< MAX_FACE_ROI; i++) {
    p_comp->hctresult[i] = p_comp->p_lib->FACEPROC_CT_CreateResultHandle();
    if (!(p_comp->hctresult[i])) {
      IDBG_ERROR("eng_FACEPROC_CT_CreateResultHandle failed");
      return IMG_ERR_GENERAL;
    }
  }
  rc = (int) p_comp->p_lib->FACEPROC_CT_SetDetectionMode(p_comp->hct,
    p_comp->fd_chromatix.ct_detection_mode);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("eng_FACEPROC_CT_SetDetectionMode failed");
    return IMG_ERR_GENERAL;
  }
#endif
#if(FACE_BGS_DETECT)
  p_comp->hsm = p_comp->p_lib->FACEPROC_SM_CreateHandle();
  if (!(p_comp->hsm)) {
    IDBG_ERROR("eng_FACEPROC_SM_CreateHandle failed");
    return IMG_ERR_GENERAL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    p_comp->hsmresult[i] = p_comp->p_lib->FACEPROC_SM_CreateResultHandle();
    if (!(p_comp->hsmresult[i])) {
      IDBG_ERROR("eng_FACEPROC_SM_CreateResultHandle failed");
      return IMG_ERR_GENERAL;
    }
  }
#endif
  /* Face Recognition */
#if(FACE_RECOGNITION)
  p_comp->hfeature = p_comp->p_lib->FACEPROC_FR_CreateFeatureHandle();
  if (!(p_comp->hfeature)) {
    IDBG_ERROR("eng_FACEPROC_FR_CreateFeatureHandle failed");
    return IMG_ERR_GENERAL;
  }
  fp = fopen(FACE_ALBUM, "r");
  if (fp == NULL) {
    IDBG_ERROR("Face Album FILE DOESNT EXIST");
    p_comp->halbum = p_comp->p_lib->FACEPROC_FR_CreateAlbumHandle(
      MAX_REGISTER_PPL,
      MAX_REGISTER_IMG_PER_PERSON);
  } else {
    IDBG_ERROR("Face Album FILE EXISTS");
    buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
    if (buffer != NULL) {
      fread(buffer, 1, MAX_ALBUM_SIZE, fp);
    } else
      IDBG_ERROR("%s: buffer is null", __func__);
    p_comp->halbum = p_comp->p_lib->FACEPROC_FR_RestoreAlbum((UINT8 *)buffer,
      MAX_ALBUM_SIZE, &pError);
    if (p_comp->halbum == NULL) {
      IDBG_ERROR("Fd album is Corrupted %d, hence recreating new one", pError);
      p_comp->halbum = p_comp->p_lib->FACEPROC_FR_CreateAlbumHandle(
        MAX_REGISTER_PPL,
        MAX_REGISTER_IMG_PER_PERSON);
    }
    if (buffer) {
        free(buffer);
    }
    fclose(fp);
  }
  if (!(p_comp->halbum)) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_CreateAlbumHandle failed",
      __func__, __LINE__);
  }
  p_comp->last_img_registered_idx = 0;
#endif

  /* Blink Detection */
#if(FACE_BGS_DETECT)
  p_comp->hgb = p_comp->p_lib->FACEPROC_GB_CreateHandle();
  if (!(p_comp->hgb)) {
    IDBG_ERROR("eng_FACEPROC_GB_CreateHandle failed");
    return IMG_ERR_GENERAL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    p_comp->hgbresult[i] = p_comp->p_lib->FACEPROC_GB_CreateResultHandle();
    if (!(p_comp->hgbresult[i])) {
      IDBG_ERROR("eng_FACEPROC_GB_CreateResultHandle failed");
      return IMG_ERR_GENERAL;
    }
  }
#endif
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_exec
 *
 * Description: main algorithm execution function for face processing
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_input_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_exec(faceproc_comp_t *p_comp, img_frame_t *p_frame)
{
  INT32 i;
  INT32 num_faces;
  int rc;
  switch (p_comp->mode) {
  case FACE_DETECT:
    rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
    if (rc != IMG_SUCCESS)
      return IMG_ERR_GENERAL;
    break;
  case FACE_RECOGNIZE:
    rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
    if (rc != IMG_SUCCESS)
      return IMG_ERR_GENERAL;
    rc = faceproc_fr_execute(p_comp, p_frame, num_faces);
    if (rc != IMG_SUCCESS)
      return IMG_ERR_GENERAL;
    break;
  case FACE_REGISTER:
    rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
    if (rc != IMG_SUCCESS)
      return IMG_ERR_GENERAL;
    rc = faceproc_fr_execute(p_comp, p_frame, num_faces);
    if (rc == IMG_ERR_GENERAL)
      return IMG_ERR_GENERAL;
    rc = faceproc_register_frame(p_comp, p_frame, num_faces);
    if (rc != IMG_SUCCESS)
      return IMG_ERR_GENERAL;
    break;
  default :
    IDBG_ERROR("%s MODE not selected/recognized", __func__);
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_get_output
 *
 * Description: Get the output from the frameproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_get_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data)
{
  INT32 num_faces;
  int status = IMG_SUCCESS;
  switch (p_comp->mode) {
  case FACE_DETECT:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    break;
  case FACE_RECOGNIZE:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    if (IMG_SUCCEEDED(status))
      status = faceproc_fr_output(p_comp, fd_data, num_faces);
    break;
  case FACE_REGISTER:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    if (IMG_SUCCEEDED(status))
      status = faceproc_register_output(p_comp, fd_data, num_faces);
    break;
  default:
    IDBG_ERROR("%s: Unsupported mode selected", __func__);
    return IMG_ERR_INVALID_INPUT;
  }
  return status;
}

/**
 * Function: faceproc_comp_eng_destroy
 *
 * Description: Destroy the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_destroy(faceproc_comp_t *p_comp)
{
  int rc;

  if (!p_comp)
    return IMG_ERR_GENERAL;

#if(FACE_RECOGNITION)
  /* Save album */
  rc = faceproc_save_album(p_comp);
  if (rc != IMG_SUCCESS)
    IDBG_ERROR("%s: Album save failed %d", __func__, rc);
#endif

  IDBG_MED("%s:%d] faceproc engine clean", __func__, __LINE__);
  rc = faceproc_comp_eng_reset(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s: faceproc_comp_eng_reset failed %d", __func__, rc);
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_reset
 *
 * Description: Reset the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_reset(faceproc_comp_t *p_comp)
{
  IDBG_MED("%s:%d]", __func__, __LINE__);
  INT32 ret, i;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  /* Delete Result handle */
  if (p_comp->hresult) {
    ret = p_comp->p_lib->FACEPROC_DeleteDtResult(p_comp->hresult);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hresult = NULL;
  }
  /* Delete Handle */
  if (p_comp->hdt) {
    ret = p_comp->p_lib->FACEPROC_DeleteDetection(p_comp->hdt);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hdt = NULL;
  }

  if (FD_FACEPT_ENABLE(p_comp)) {
#if(FACE_PART_DETECT)
  if (p_comp->hpt) {
    ret = p_comp->p_lib->FACEPROC_PT_DeleteHandle(p_comp->hpt);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hpt = NULL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    if (p_comp->hptresult[i]) {
      ret = p_comp->p_lib->FACEPROC_PT_DeleteResultHandle(
        p_comp->hptresult[i]);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hptresult[i] = NULL;
    }
  }
#endif

#if(FACE_CONTOUR_DETECT)
  if (p_comp->hct) {
    ret = p_comp->p_lib->FACEPROC_CT_DeleteHandle(p_comp->hct);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hct = NULL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    if (p_comp->hctresult[i]) {
      ret = p_comp->p_lib->FACEPROC_CT_DeleteResultHandle(
        p_comp->hctresult[i]);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hctresult[i] = NULL;
    }
  }
#endif
#if(FACE_BGS_DETECT)
  if (p_comp->hsm) {
    ret = p_comp->p_lib->FACEPROC_SM_DeleteHandle(p_comp->hsm);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hsm = NULL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    if (p_comp->hsmresult[i]) {
      ret = p_comp->p_lib->FACEPROC_SM_DeleteResultHandle(
        p_comp->hsmresult[i]);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hsmresult[i] = NULL;
    }
  }
#endif
  /* Face Recognition */
#if(FACE_RECOGNITION)
  if (p_comp->hfeature) {
    ret = p_comp->p_lib->FACEPROC_FR_DeleteFeatureHandle(
      p_comp->hfeature);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hfeature = NULL;
  }
  if (p_comp->halbum) {
    ret = p_comp->p_lib->FACEPROC_FR_DeleteAlbumHandle(
      p_comp->halbum);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->halbum = NULL;
  }
#endif
  /* Blink/Gaze Detection */
#if(FACE_BGS_DETECT)
  if (p_comp->hgb) {
    ret = p_comp->p_lib->FACEPROC_GB_DeleteHandle(p_comp->hgb);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hgb = NULL;
  }
  for (i = 0; i < MAX_FACE_ROI; i++) {
    if (p_comp->hgbresult[i]) {
      ret = p_comp->p_lib->FACEPROC_GB_DeleteResultHandle(
        p_comp->hgbresult[i]);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hgbresult[i] = NULL;
    }
  }
#endif
  }

  /* API to detach allocated memory from Faceproc engine */
  if (p_comp->fdBmemorySet) {
    ret = p_comp->p_lib->FACEPROC_FreeBMemoryArea();
    p_comp->fdBmemorySet = FALSE;
  }
  /* API to detach allocated memory from Faceproc */
  if (p_comp->fdWmemorySet) {
    ret = p_comp->p_lib->FACEPROC_FreeWMemoryArea();
    IDBG_ERROR("Free Wmemory %d", ret);
    p_comp->fdWmemorySet = FALSE;
  }
  /* Scratch buffers */
  if (p_comp->bmem) {
    free(p_comp->bmem);
    p_comp->bmem = NULL;
  }

  if (p_comp->wmem) {
    free(p_comp->wmem);
    p_comp->wmem = NULL;
  }
  frame_count = 0;
  dump_count = 0;
  return IMG_SUCCESS;
}
