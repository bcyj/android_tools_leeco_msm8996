/*========================================================================

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

====================================================================== */
#include <dlfcn.h>
#include <math.h>
#include "faceproc_engine.h"
#include "../face_proc_util.h"
#define SEARCH_DENSITY 33

/*===========================================================================
* FUNCTION    -  faceproc_unload_lib -
*
* DESCRIPTION: Unregister faceproc engine
* .
*==========================================================================*/
static void faceproc_unload_lib(faceproc_engine_t *p_eng)
{
  CDBG_FD("%s E", __func__);
  if (p_eng->lib.FaceProc_ptr) {
    dlclose(p_eng->lib.FaceProc_ptr);
    p_eng->lib.FaceProc_ptr = NULL;
  }
  memset(&(p_eng->lib), 0, sizeof(faceproc_engine_lib_t));
}

/*===========================================================================
* FUNCTION    -  faceproc_load_lib -
*
* DESCRIPTION: Register faceproc engine
* .
*==========================================================================*/
static int faceproc_load_lib(faceproc_engine_t *p_eng)
{
  int rc = 0;
  p_eng->lib.FaceProc_ptr =
    dlopen("libmmcamera_faceproc.so", RTLD_NOW);
  if (!p_eng->lib.FaceProc_ptr) {
    CDBG_FD("%s Error opening libmmcamera_faceproc.so lib", __func__);
    rc = -1;
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_DeleteDtResult) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_DeleteDtResult");
    if (p_eng->lib.FACEPROC_DeleteDtResult == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_DeleteDtResult ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_DeleteDetection) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_DeleteDetection");
    if (p_eng->lib.FACEPROC_DeleteDetection == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_DeleteDetection ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GetDtFaceCount) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GetDtFaceCount");
    if (p_eng->lib.FACEPROC_GetDtFaceCount == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_GetDtFaceCount ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GetDtFaceInfo) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GetDtFaceInfo");
    if (p_eng->lib.FACEPROC_GetDtFaceInfo == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_GetDtFaceInfo ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtMemorySize) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtMemorySize");
    if (p_eng->lib.FACEPROC_SetDtMemorySize == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtMemorySize ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GetDtRequiredMovieMemSize) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GetDtRequiredMovieMemSize");
    if (p_eng->lib.FACEPROC_GetDtRequiredMovieMemSize == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_GetDtRequiredMovieMemSize ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CreateDetection) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CreateDetection");
    if (p_eng->lib.FACEPROC_CreateDetection == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_CreateDetection ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtMode) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtMode");
    if (p_eng->lib.FACEPROC_SetDtMode == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtMode ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtStep) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtStep");
    if (p_eng->lib.FACEPROC_SetDtStep == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtStep ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtAngle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtAngle");
    if (p_eng->lib.FACEPROC_SetDtAngle == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtAngle ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtDirectionMask) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtDirectionMask");
    if (p_eng->lib.FACEPROC_SetDtDirectionMask == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtDirectionMask ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtFaceSizeRange) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtFaceSizeRange");
    if (p_eng->lib.FACEPROC_SetDtFaceSizeRange == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtFaceSizeRange ", __func__);
      rc = -1;
    }
  }
    if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetDtThreshold) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetDtThreshold");
    if (p_eng->lib.FACEPROC_SetDtThreshold == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_SetDtThreshold ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CreateDtResult) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CreateDtResult");
    if (p_eng->lib.FACEPROC_CreateDtResult == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_CreateDtResult ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_Detection) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_Detection");
    if (p_eng->lib.FACEPROC_Detection == NULL) {
      CDBG_FD("%s Error Loading FACEPROC_Detection ", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FreeBMemoryArea) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FreeBMemoryArea");
    if (p_eng->lib.FACEPROC_FreeBMemoryArea == NULL) {
      CDBG_FD("%s Loading FACEPROC_FreeBMemoryArea error", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FreeWMemoryArea) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FreeWMemoryArea");
    if (p_eng->lib.FACEPROC_FreeWMemoryArea == NULL) {
      CDBG_FD("%s Loading FACEPROC_FreeWMemoryArea error", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetBMemoryArea) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetBMemoryArea");
    if (p_eng->lib.FACEPROC_SetBMemoryArea == NULL) {
      CDBG_FD("%s Loading FACEPROC_SetBMemoryArea error", __func__);
      rc = -1;
    }
  }

  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SetWMemoryArea) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SetWMemoryArea");
    if (p_eng->lib.FACEPROC_SetWMemoryArea == NULL) {
      CDBG_FD("%s Loading FACEPROC_SetWMemoryArea error", __func__);
      rc = -1;
    }
  }
  /*Pt detection */
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_CreateHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_CreateHandle");
    if (p_eng->lib.FACEPROC_PT_CreateHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_CreateHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_DeleteHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_DeleteHandle");
    if (p_eng->lib.FACEPROC_PT_DeleteHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_DeleteHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_CreateResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_CreateResultHandle");
    if (p_eng->lib.FACEPROC_PT_CreateResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_CreateResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_DeleteResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_DeleteResultHandle");
    if (p_eng->lib.FACEPROC_PT_DeleteResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_DeleteResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_SetPositionFromHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_SetPositionFromHandle");
    if (p_eng->lib.FACEPROC_PT_SetPositionFromHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_SetPositionFromHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_SetMode) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_SetMode");
    if (p_eng->lib.FACEPROC_PT_SetMode == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_SetMode error", __func__);
      rc = -1;
    }
  }
   if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_SetConfMode) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_SetConfMode");
    if (p_eng->lib.FACEPROC_PT_SetConfMode == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_SetConfMode error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_DetectPoint) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_DetectPoint");
    if (p_eng->lib.FACEPROC_PT_DetectPoint == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_DetectPoint error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_GetResult) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_GetResult");
    if (p_eng->lib.FACEPROC_PT_GetResult == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_GetResult error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_PT_GetFaceDirection) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_PT_GetFaceDirection");
    if (p_eng->lib.FACEPROC_PT_GetFaceDirection == NULL) {
      CDBG_FD("%s Loading FACEPROC_PT_GetFaceDirection error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_CreateHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_CreateHandle");
    if (p_eng->lib.FACEPROC_CT_CreateHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_CreateHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_DeleteHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_DeleteHandle");
    if (p_eng->lib.FACEPROC_CT_DeleteHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_DeleteHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_CreateResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_CreateResultHandle");
    if (p_eng->lib.FACEPROC_CT_CreateResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_CreateResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_DeleteResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_DeleteResultHandle");
    if (p_eng->lib.FACEPROC_CT_DeleteResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_DeleteResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_SetPointFromHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_SetPointFromHandle");
    if (p_eng->lib.FACEPROC_CT_SetPointFromHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_SetPointFromHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_DetectContour) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_DetectContour");
    if (p_eng->lib.FACEPROC_CT_DetectContour == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_DetectContour error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_GetResult) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_GetResult");
    if (p_eng->lib.FACEPROC_CT_GetResult == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_GetResult error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_SetDetectionMode) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_SetDetectionMode");
    if (p_eng->lib.FACEPROC_CT_SetDetectionMode == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = -1;
    }
  }
  /* Smile API */
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_CreateHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_CreateHandle");
    if (p_eng->lib.FACEPROC_SM_CreateHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_CreateHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_DeleteHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_DeleteHandle");
    if (p_eng->lib.FACEPROC_SM_DeleteHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_DeleteHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_CreateResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_CreateResultHandle");
    if (p_eng->lib.FACEPROC_SM_CreateResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_CreateResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_DeleteResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_DeleteResultHandle");
    if (p_eng->lib.FACEPROC_SM_DeleteResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_DeleteResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_SetPointFromHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_SetPointFromHandle");
    if (p_eng->lib.FACEPROC_SM_SetPointFromHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_SetPointFromHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_Estimate) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_Estimate");
    if (p_eng->lib.FACEPROC_SM_Estimate == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_Estimate error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_SM_GetResult) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_SM_GetResult");
    if (p_eng->lib.FACEPROC_SM_GetResult == NULL) {
      CDBG_FD("%s Loading FACEPROC_SM_GetResult error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_CT_SetDetectionMode) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_CT_SetDetectionMode");
    if (p_eng->lib.FACEPROC_CT_SetDetectionMode == NULL) {
      CDBG_FD("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = -1;
    }
  }
  /* Face Recognition */
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_CreateFeatureHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_CreateFeatureHandle");
    if (p_eng->lib.FACEPROC_FR_CreateFeatureHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_CreateFeatureHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_DeleteFeatureHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_DeleteFeatureHandle");
    if (p_eng->lib.FACEPROC_FR_DeleteFeatureHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_DeleteFeatureHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {

    *(void **)&(p_eng->lib.FACEPROC_FR_CreateAlbumHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_CreateAlbumHandle");
    if (p_eng->lib.FACEPROC_FR_CreateAlbumHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_CreateAlbumHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_DeleteAlbumHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_DeleteAlbumHandle");
    if (p_eng->lib.FACEPROC_FR_DeleteAlbumHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_DeleteAlbumHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_ExtractFeatureFromPtHdl) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_ExtractFeatureFromPtHdl");
    if (p_eng->lib.FACEPROC_FR_ExtractFeatureFromPtHdl == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_ExtractFeatureFromPtHdl error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_RegisterData) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_RegisterData");
    if (p_eng->lib.FACEPROC_FR_RegisterData == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_RegisterData error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_GetRegisteredUserNum) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_GetRegisteredUserNum");
    if (p_eng->lib.FACEPROC_FR_GetRegisteredUserNum == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_GetRegisteredUserNum error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_GetRegisteredUsrDataNum) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_GetRegisteredUsrDataNum");
    if (p_eng->lib.FACEPROC_FR_GetRegisteredUsrDataNum == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_GetRegisteredUsrDataNum error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_ClearAlbum) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_ClearAlbum");
    if (p_eng->lib.FACEPROC_FR_ClearAlbum == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_ClearAlbum error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_GetSerializedAlbumSize) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_GetSerializedAlbumSize");
    if (p_eng->lib.FACEPROC_FR_GetSerializedAlbumSize == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_GetSerializedAlbumSize error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_SerializeAlbum) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_SerializeAlbum");
    if (p_eng->lib.FACEPROC_FR_SerializeAlbum == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_SerializeAlbum error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_RestoreAlbum) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_RestoreAlbum");
    if (p_eng->lib.FACEPROC_FR_RestoreAlbum == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_RestoreAlbum error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_Identify) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_Identify");
    if (p_eng->lib.FACEPROC_FR_Identify == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_Identify error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_Verify) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_Verify");
    if (p_eng->lib.FACEPROC_FR_Verify == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_Verify error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_FR_IsRegistered) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_FR_IsRegistered");
    if (p_eng->lib.FACEPROC_FR_IsRegistered == NULL) {
      CDBG_FD("%s Loading FACEPROC_FR_IsRegistered error", __func__);
      rc = -1;
    }
  }
  /* Blink Detect */
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_CreateHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_CreateHandle");
    if (p_eng->lib.FACEPROC_GB_CreateHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_CreateHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_DeleteHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_DeleteHandle");
    if (p_eng->lib.FACEPROC_GB_DeleteHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_DeleteHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_CreateResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_CreateResultHandle");
    if (p_eng->lib.FACEPROC_GB_CreateResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_CreateResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_DeleteResultHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_DeleteResultHandle");
    if (p_eng->lib.FACEPROC_GB_DeleteResultHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_DeleteResultHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_SetPointFromHandle) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_SetPointFromHandle");
    if (p_eng->lib.FACEPROC_GB_SetPointFromHandle == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_SetPointFromHandle error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_Estimate) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_Estimate");
    if (p_eng->lib.FACEPROC_GB_Estimate == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_Estimate error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_GetEyeCloseRatio) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_GetEyeCloseRatio");
    if (p_eng->lib.FACEPROC_GB_GetEyeCloseRatio == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_GetEyeCloseRatio error", __func__);
      rc = -1;
    }
  }
  if (!rc) {
    *(void **)&(p_eng->lib.FACEPROC_GB_GetGazeDirection) =
      dlsym(p_eng->lib.FaceProc_ptr,
      "FACEPROC_GB_GetGazeDirection");
    if (p_eng->lib.FACEPROC_GB_GetGazeDirection == NULL) {
      CDBG_FD("%s Loading FACEPROC_GB_GetGazeDirection error", __func__);
      rc = -1;
    }
  }
  if (rc < 0) {
    faceproc_unload_lib(p_eng);
  }
  return rc;
}
/*===========================================================================
* FUNCTION    -  faceproc_register_frame -
*
* DESCRIPTION: Register frame for Face Recognition
* .
*==========================================================================*/
int faceproc_register_frame (faceproc_engine_t *p_engine,
  fd_frame_t  *p_input_frame)
{
  INT32 i;
  INT32 num_faces, pnUserDataNum;
  INT32 user_idx, data_idx;

  int rc = p_engine->lib.FACEPROC_Detection(p_engine->hdt,
    (RAWIMAGE *)p_input_frame->buffer,
    p_engine->frame_width,
    p_engine->frame_height,
    ACCURACY_HIGH_TR,
    p_engine->hresult);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_Detection returned error: %d", (uint32_t)rc);
    return FD_RET_FAILURE;
  }
  /*Set Position for PT */

  rc = p_engine->lib.FACEPROC_GetDtFaceCount(p_engine->hresult,
    &num_faces);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    return FD_RET_FAILURE;
  }
  CDBG_FD("%s Num faces detected is %d", __func__, num_faces);
  if (num_faces<=0)
    return FD_RET_SUCCESS;
  for (i = 0; i < num_faces; i++) {
    if (p_engine->recognized[i]==0) {
      CDBG_FD("Face not registered. Adding new Entry");
      user_idx = (p_engine->last_img_registered_idx %MAX_REGISTER_PPL );
      p_engine->UserID[i] = user_idx;
      p_engine->last_img_registered_idx++;
      data_idx = 0;
    } else {
      CDBG_FD("Face already registered. Updating Entry");
      rc = p_engine->lib.
        FACEPROC_FR_GetRegisteredUsrDataNum(p_engine->halbum, p_engine->UserID[i],
        &pnUserDataNum);
      user_idx = p_engine->UserID[i];
      data_idx = pnUserDataNum % MAX_REGISTER_IMG_PER_PERSON;
      CDBG_FD("value of pnUserDataNum %d", pnUserDataNum );
    }
    CDBG_FD("REGISTERING DATA AT userid %d dataid %d ", user_idx, data_idx);
    rc = p_engine->lib.FACEPROC_FR_RegisterData(p_engine->halbum,
      p_engine->hfeature, user_idx, data_idx);
  }
  return FD_RET_SUCCESS;
}  /* faceproc_register_frame */
/*===========================================================================
* FUNCTION    -  faceproc_fd_output -
*
* DESCRIPTION: Output of FD  engine
* .
*==========================================================================*/
static int faceproc_fd_output(fd_ctrl_t *fdCtrl, frame_proc_fd_data_t *fd_data,
  INT32 *num_faces)
{
  POINT aptPoint[PT_POINT_KIND_MAX];
  INT32 anConfidence[PT_POINT_KIND_MAX];
  int rc, leye_close_ratio, reye_close_ratio, pnGazeLeftRight, pnGazeUpDown;
  uint32_t i, j;
  frame_proc_fd_roi_t *p_output;
  faceproc_engine_t *p_engine = (faceproc_engine_t*)fdCtrl->local_fd_obj;
  /* FD START */
  /* Get the number of faces */
  rc = p_engine->lib.FACEPROC_GetDtFaceCount(p_engine->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    return FD_RET_FAILURE;
  }
  /* Parse and store the faces */
  fd_data->num_faces_detected = (uint32_t)*num_faces;
  if (fd_data->num_faces_detected > fdCtrl->num_faces_total) {
    fd_data->num_faces_detected = fdCtrl->num_faces_total;
  }
  if (!fd_data->num_faces_detected) {
    for (i = 0; i < MAX_ROI; i++) {
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
    rc = p_engine->lib.FACEPROC_GetDtFaceInfo(p_engine->hresult, i,
      &face_info);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("FACEPROC_GetDtFaceInfo returned error: %d", (uint32_t)rc);
      fd_data->num_faces_detected--;
      return FD_RET_FAILURE;
    }
    /* Translate the data */
    /* Clip each detected face coordinates to be within the frame boundary */
    CLIP(face_info.ptLeftTop.x,     0, (int32_t)fdCtrl->config.frame_cfg.width);
    CLIP(face_info.ptRightTop.x,    0, (int32_t)fdCtrl->config.frame_cfg.width);
    CLIP(face_info.ptLeftBottom.x,  0, (int32_t)fdCtrl->config.frame_cfg.width);
    CLIP(face_info.ptRightBottom.x, 0, (int32_t)fdCtrl->config.frame_cfg.width);
    CLIP(face_info.ptLeftTop.y,     0, (int32_t)fdCtrl->config.frame_cfg.height);
    CLIP(face_info.ptRightTop.y,    0, (int32_t)fdCtrl->config.frame_cfg.height);
    CLIP(face_info.ptLeftBottom.y,  0, (int32_t)fdCtrl->config.frame_cfg.height);
    CLIP(face_info.ptRightBottom.y, 0, (int32_t)fdCtrl->config.frame_cfg.height);

    /* Find the bounding box */
    left   = MIN4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    top    = MIN4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);
    right  = MAX4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    bottom = MAX4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);
    p_output = &fd_data->roi[i];
    p_output->gaze_angle = face_info.nPose;
    p_output->face_boundary.x = left;
    p_output->face_boundary.y = top;
    p_output->face_boundary.dx = right - left;
    p_output->face_boundary.dy = bottom - top;
    p_output->unique_id = face_info.nID;
    p_output->fd_confidence = face_info.nConfidence;
    /* Pt result */
    rc = p_engine->lib.FACEPROC_PT_GetResult(p_engine->hptresult[i],
      PT_POINT_KIND_MAX, (POINT *)p_output->fp.facePt, p_output->fp.Confidence);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: PT_GetResult failed", __func__);
      return FD_RET_FAILURE;
    }
    rc = p_engine->lib.FACEPROC_PT_GetFaceDirection(p_engine->hptresult[i],
      &(p_output->fp.faceDirectionUpDown),
      &(p_output->fp.faceDirectionLeftRight),
      &(p_output->fp.faceDirectionRoll));
    CDBG_FD("%s: PT Coordinates[%d] LEFT_EYE %d, %d  up %d, left %d, roll %d",
      __func__, i, p_output->fp.facePt[PT_POINT_LEFT_EYE].x,
      p_output->fp.facePt[PT_POINT_LEFT_EYE].y,
      p_output->fp.faceDirectionUpDown,
      p_output->fp.faceDirectionLeftRight,
      p_output->fp.faceDirectionRoll);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: eng_FACEPROC_PT_GetFaceDirection failed", __func__);
      return FD_RET_FAILURE;
    }
#if(FACE_CONTOUR_DETECT)
    rc = p_engine->lib.FACEPROC_CT_GetResult(p_engine->hctresult[i],
      CT_POINT_KIND_MAX, (POINT *)&p_output->ct);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: CT_GetResult failed", __func__);
      return FD_RET_FAILURE;
    }
    CDBG_FD("%s: CT Coordinates[%d] for CT_POINT_EYE_L_UP %d, %d", __func__, i,
      p_output->ct.contour_pt[CT_POINT_EYE_L_UP].x,
      p_output->ct.contour_pt[CT_POINT_EYE_L_UP].y);
    CDBG_FD("%s: CT Coordinates[%d] for CT_POINT_EYE_L_DOWN %d, %d", __func__, i,
      p_output->ct.contour_pt[CT_POINT_EYE_L_DOWN].x,
      p_output->ct.contour_pt[CT_POINT_EYE_L_DOWN].y);

#endif
#if(FACE_SMILE_DETECT)
    rc = p_engine->lib.FACEPROC_SM_GetResult(p_engine->hsmresult[i],
      &(p_output->sm.smile_degree), &(p_output->sm.confidence));
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: CT_GetResult failed", __func__);
      return FD_RET_FAILURE;
    }
    CDBG_FD("%s: SM Coordinates[%d] for %d, %d", __func__, i,
      p_output->sm.smile_degree,
      p_output->sm.confidence);
#endif
    p_output->blink_detected = 0;
    rc = p_engine->lib.FACEPROC_GB_GetEyeCloseRatio(p_engine->hgbresult[i],
      &(leye_close_ratio), &(reye_close_ratio));
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: eng_FACEPROC_GB_GetEyeCloseRatio failed", __func__);
      return FD_RET_FAILURE;
    }
    CDBG_FD("%s: BLINK Ratio  l %d, r %d", __func__, leye_close_ratio,
      reye_close_ratio);
    p_output->left_blink = leye_close_ratio;
    p_output->right_blink = reye_close_ratio;
    if (leye_close_ratio > 600 &&
      reye_close_ratio > 600) {
      CDBG_FD("EYES CLOSED");
      p_output->blink_detected=1;
    }
    rc = p_engine->lib.FACEPROC_GB_GetGazeDirection(p_engine->hgbresult[i],
      &(pnGazeLeftRight),&(pnGazeUpDown));
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("%s: eng_FACEPROC_GB_GetGazeDirection failed", __func__);
      return FD_RET_FAILURE;
    }
    CDBG_FD("%s: Gaze Ratio  left_right %d, top_bottom %d", __func__, pnGazeLeftRight,
      pnGazeUpDown);
    p_output->left_right_gaze = pnGazeLeftRight;
    p_output->top_bottom_gaze = pnGazeUpDown;
  }  /* end of forloop */
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_fr_output -
*
* DESCRIPTION: Output of FR  engine
* .
*==========================================================================*/
static int faceproc_fr_output(faceproc_engine_t *p_engine,
  frame_proc_fd_data_t *fd_data, INT32 num_faces)
{
  int i;
  frame_proc_fd_roi_t *p_output;
  for (i=0; i<num_faces;i++) {
    p_output = &fd_data->roi[i];
    p_output->is_face_recognised = p_engine->recognized[i];
    p_output->unique_id = p_engine->UserID[i];
    p_output->fd_confidence = p_engine->confidence[i];
  }
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_register_output -
*
* DESCRIPTION: Output of FR  engine
* .
*==========================================================================*/
static int faceproc_register_output(faceproc_engine_t *p_engine,
  frame_proc_fd_data_t *fd_data, INT32 num_faces)
{
  int i;
  for (i=0; i<num_faces;i++) {
    fd_data->roi[i].unique_id = p_engine->UserID[i];
  }
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_fd_execute -
*
* DESCRIPTION: Face Detection Execute
* .
*==========================================================================*/
static int faceproc_fd_execute(faceproc_engine_t *p_engine, fd_frame_t  *p_input_frame,
  INT32 * num_faces)
{
  INT32 i;
  int rc = p_engine->lib.FACEPROC_Detection(p_engine->hdt,
    (RAWIMAGE *)p_input_frame->buffer,
    p_engine->frame_width,
    p_engine->frame_height,
    ACCURACY_HIGH_TR,
    p_engine->hresult);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_Detection returned error: %d", (uint32_t)rc);
    return FD_RET_FAILURE;
  }
  /*Set Position for PT */
  rc = p_engine->lib.FACEPROC_GetDtFaceCount(p_engine->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    return FD_RET_FAILURE;
  }
  if (*num_faces<=0)
    return FD_RET_SUCCESS;
  for (i = 0; i < *num_faces; i++) {
    rc = p_engine->lib.FACEPROC_PT_SetPositionFromHandle(p_engine->hpt,
      p_engine->hresult, i);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_PT_SetPositionFromHandle returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    rc = p_engine->lib.FACEPROC_PT_DetectPoint(p_engine->hpt,
      (RAWIMAGE *)p_input_frame->buffer,
      p_engine->frame_width,
      p_engine->frame_height, p_engine->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_PT_DetectPoint returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
#if(FACE_CONTOUR_DETECT)
    rc = p_engine->lib.FACEPROC_CT_SetPointFromHandle(p_engine->hct,
      p_engine->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_CT_SetPositionFromHandle returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    rc = p_engine->lib.FACEPROC_CT_DetectContour(p_engine->hct,
      (RAWIMAGE *)p_input_frame->buffer,
      p_engine->frame_width,
      p_engine->frame_height, p_engine->hctresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_PT_DetectPoint returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
#endif
#if(FACE_SMILE_DETECT)
    rc = p_engine->lib.FACEPROC_SM_SetPointFromHandle(p_engine->hsm,
      p_engine->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_SM_SetPointFromHandle returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    rc = p_engine->lib.FACEPROC_SM_Estimate(p_engine->hsm,
      (RAWIMAGE *)p_input_frame->buffer,
      p_engine->frame_width,
      p_engine->frame_height, p_engine->hsmresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_SM_Estimate returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
#endif
    /* Blink/Gaze Detection */
    rc = p_engine->lib.FACEPROC_GB_SetPointFromHandle(p_engine->hgb,
      p_engine->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_GB_SetPointFromHandle returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    rc = p_engine->lib.FACEPROC_GB_Estimate(p_engine->hgb,
      (RAWIMAGE *)p_input_frame->buffer,
      p_engine->frame_width,
      p_engine->frame_height, p_engine->hgbresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_GB_Estimate returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
  }  /* end of forloop */
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_fr_execute -
*
* DESCRIPTION: Face Recognition execute
* .
*==========================================================================*/
static int faceproc_fr_execute(faceproc_engine_t *p_engine, fd_frame_t  *p_input_frame,
  int num_faces)
{
  int i, rc;
  INT32 anUserID[MAX_REGISTER_PPL], pnUserNum, pnUserDataNum;
  INT32 anScore[MAX_REGISTER_PPL], pnResultNum;
  BOOL pIsRegistered;
  if (num_faces<=0)
    return FD_RET_SUCCESS;

  /* FR START */
  for (i = 0; i < num_faces; i++) {
    p_engine->UserID[i] = -1;
    p_engine->confidence[i] = 0;
#if(FACE_RECOGNITION)
    rc = p_engine->lib.FACEPROC_FR_ExtractFeatureFromPtHdl(p_engine->hfeature,
      (RAWIMAGE *)p_input_frame->buffer,
      p_engine->frame_width,
      p_engine->frame_height, p_engine->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_FR_ExtractFeatureFromPtHdl returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
  #if(FACE_RECOGNIZE_TEST_REGISTER)
    /*TODO move recognie to different fn */
    rc = p_engine->lib.FACEPROC_FR_RegisterData(p_engine->halbum,
      p_engine->hfeature, i, 0);
    return FD_RET_SUCCESS;
  #endif

    /* Find if album is empty */
    rc = p_engine->lib.FACEPROC_FR_GetRegisteredUserNum(p_engine->halbum, &pnUserNum);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_FR_GetRegisteredUsrNum returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    p_engine->last_img_registered_idx = pnUserNum;
    if (!pnUserNum) {
      CDBG_FD("Face Recognition database/Album is empty");
      return FD_RET_FAILURE;
    } else
      CDBG_FD("Number of User entries in album is %d", pnUserNum);
    if (!p_engine->hfeature ||!p_engine->halbum)
      CDBG_FD("hfeature and halbum is null, failed");
    CDBG_FD("Executing Face Recognition");
    rc = p_engine->lib.FACEPROC_FR_Identify(p_engine->hfeature,
      p_engine->halbum, MAX_REGISTER_PPL,
      anUserID, anScore, &pnResultNum);
    if (rc != FACEPROC_NORMAL) {
      CDBG_FD("eng_FACEPROC_FR_Identify returned error: %d", (uint32_t)rc);
      return FD_RET_FAILURE;
    }
    CDBG_ERROR("%s USER ID %d  score %d", __func__, anUserID[0], anScore[0]);
    if (pnResultNum <= MAX_REGISTER_PPL && anScore[0]>=50) {
      p_engine->recognized[i] = 1;
      p_engine->UserID[i] = anUserID[0];
      p_engine->confidence[i] =anScore[0];
    } else {
      p_engine->recognized[i] = 0;
      p_engine->UserID[i] = -1;
      p_engine->confidence[i] = 0;
    }
#endif
    /*FR END */
  }
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_engine_create -
*
* DESCRIPTION: Create handles for faceproc engine
* .
*==========================================================================*/
/* Create */
int faceproc_engine_create(void * Ctrl)
{
  fd_ctrl_t * fdCtrl = (fd_ctrl_t *)Ctrl;
  faceproc_engine_t *p_new_engine;
  int rc;
  p_new_engine = malloc(sizeof(faceproc_engine_t));
  rc = (p_new_engine == NULL);
  if (FD_FAILED(rc))
    return FD_RET_FAILURE;

  memset((void *)p_new_engine, 0, sizeof(faceproc_engine_t));
  rc = faceproc_load_lib(p_new_engine);

  if (FD_FAILED(rc)) {
    free(p_new_engine);
    p_new_engine = NULL;
    return FD_RET_FAILURE;
  }
  /* Output the new engine */
  fdCtrl->local_fd_obj =  (void *)p_new_engine;

  return FD_RET_SUCCESS;
}

/*===========================================================================
* FUNCTION    -  faceproc_engine_config -
*
* DESCRIPTION: Configure faceproc engine
* .
*==========================================================================*/
/* Config */
int faceproc_engine_config(void *Ctrl,
  void *config)
{
  fd_ctrl_t *fdCtrl = (fd_ctrl_t *)Ctrl;
  fd_config_t *p_cfg = (fd_config_t *)config;
  faceproc_engine_t *p_engine = (faceproc_engine_t *)fdCtrl->local_fd_obj;
  UINT32 an_still_angle[POSE_TYPE_COUNT];
  UINT8 major, minor, *buffer;
  FILE *fp;
  FR_ERROR pError;
  int i;
  UINT32 bmem_size_required=0, wmem_size_required_min=0, wmem_size_required_max;
  RECT nil_edge = {-1, -1, -1, -1};
  uint32_t rotation_range = p_cfg->face_cfg.rotation_range;
  int rc = FD_RET_SUCCESS;

  if (!p_engine)
    return FD_RET_FAILURE;

  faceproc_engine_reset(p_engine);

  if (rotation_range == FD_ROT_RANGE_MAX_SUPPORTED)
    rotation_range = 45;

  /* Based on the Face Orientation Hint amd prefered degree of freedom of detection,
    configuring the FD module to detect accordingly */
  if (rotation_range <= 15) {
    /* The entire range detection(360) is divided into 12
    sectors (ANGLE0 through ANGLE 11). Based on the orientation
    we select the angles */
    switch (p_cfg->face_cfg.face_orientation_hint) {
      case FD_FACE_ORIENTATION_0:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_3 | ANGLE_9;
        break;
      case FD_FACE_ORIENTATION_90:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_3 | ANGLE_6;
        break;
      case FD_FACE_ORIENTATION_180:
        an_still_angle[POSE_FRONT] = ANGLE_3 | ANGLE_6 | ANGLE_9;
        break;
      case FD_FACE_ORIENTATION_270:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_6 | ANGLE_9;
        break;
      case FD_FACE_ORIENTATION_UNKNOWN:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_3 | ANGLE_6 | ANGLE_9;
        break;
      default:
        break;
    }
  } else if (rotation_range <= 45) {
    switch (p_cfg->face_cfg.face_orientation_hint) {
      case FD_FACE_ORIENTATION_0:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_1 | ANGLE_2 |
          ANGLE_3 | ANGLE_4 | ANGLE_8 |
          ANGLE_9 |ANGLE_10 | ANGLE_11;
        break;
      case FD_FACE_ORIENTATION_90:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_1 | ANGLE_2 |
          ANGLE_3 | ANGLE_4 | ANGLE_5 |
          ANGLE_6 | ANGLE_7 | ANGLE_11;
        break;
      case FD_FACE_ORIENTATION_180:
        an_still_angle[POSE_FRONT] = ANGLE_2 | ANGLE_3 | ANGLE_4 |
          ANGLE_5 | ANGLE_6 | ANGLE_7 |
          ANGLE_8 | ANGLE_9 | ANGLE_10;
        break;
      case FD_FACE_ORIENTATION_270:
        an_still_angle[POSE_FRONT] = ANGLE_0 | ANGLE_1 | ANGLE_5 |
          ANGLE_6 | ANGLE_7 | ANGLE_8 |
          ANGLE_9 |ANGLE_10 | ANGLE_11;
        break;
      case FD_FACE_ORIENTATION_UNKNOWN:
        an_still_angle[POSE_FRONT] = ANGLE_ALL;
        break;
      default:
        break;
    }
  } else {
    return FD_RET_INVALID_PARM;
  }
  an_still_angle[POSE_FRONT] = ANGLE_ALL;
  an_still_angle[POSE_HALF_PROFILE] = ANGLE_ALL;
  an_still_angle[POSE_PROFILE] = ANGLE_ALL;

  /* Determine the size of memory needed */
  rc = p_engine->lib.FACEPROC_GetDtRequiredMovieMemSize(p_cfg->frame_cfg.width,
    p_cfg->frame_cfg.height,
    p_cfg->face_cfg.min_face_size,
    p_cfg->face_cfg.max_num_face_to_detect +
    p_cfg->face_cfg.max_num_face_to_detect / 2,
    nil_edge, SEARCH_DENSITY,
    nil_edge, SEARCH_DENSITY,
    &bmem_size_required,
    &wmem_size_required_min,
    &wmem_size_required_max);

  /* Allocate scratch memory for Faceproc FD */
  CDBG_FD("GetDtRequiredMovieMemSiz bmem %d wmem %d rc=%d",
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
  wmem_size_required_max +=(150 * KB_SIZE);
  CDBG_FD("After GetDtRequiredMovieMemSiz bmem %d wmem %d rc=%d",
    bmem_size_required, wmem_size_required_max, rc);

  if (FD_SUCCEEDED(rc))
    p_engine->bmem = malloc(bmem_size_required);
  rc = (p_engine->bmem == NULL);
  if (FD_SUCCEEDED(rc))
    p_engine->wmem = malloc(wmem_size_required_max);
  rc = (p_engine->wmem == NULL);

  CDBG_FD("GetDtRequiredMovieMemSiz1 value rc=%d", rc);

  if (FD_FAILED(rc)) {
    faceproc_engine_reset(p_engine);
    return FD_RET_NO_RESOURCE;
  }

  /* Pass the scratch memory to Faceproc's FD */

  rc = (int) p_engine->lib.FACEPROC_SetWMemoryArea(p_engine->wmem,
    wmem_size_required_max);
  CDBG_FD("SetWMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    faceproc_engine_reset(p_engine);
    return FD_RET_NO_RESOURCE;
  }
  p_engine->fdWmemorySet = TRUE;
  rc = (int) p_engine->lib.FACEPROC_SetBMemoryArea(p_engine->bmem,
    bmem_size_required);

  CDBG_FD("SetBMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    faceproc_engine_reset(p_engine);
    return FD_RET_NO_RESOURCE;
  }
  p_engine->fdBmemorySet = TRUE;
  /* Create Faceproc FD handle */
  p_engine->hdt = p_engine->lib.FACEPROC_CreateDetection();
  if (!p_engine->hdt) {
    CDBG_FD("FACEPROC_CreateDetection failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }


  /* Set best Faceproc detection mode for video */
  rc = (int) p_engine->lib.FACEPROC_SetDtMode(
    p_engine->hdt, DT_MODE_MOTION3);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtMode failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  /* Set search density */
  rc = (int) p_engine->lib.FACEPROC_SetDtStep(
    p_engine->hdt, SEARCH_DENSITY, SEARCH_DENSITY);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtStep failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  /* Set Omrom Detection Angles */
  rc = (int) p_engine->lib.FACEPROC_SetDtAngle(
    p_engine->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtAngle failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  rc = (int) p_engine->lib.FACEPROC_SetDtDirectionMask(
    p_engine->hdt, FALSE);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtDirectionMask failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (p_cfg->face_cfg.min_face_size > p_cfg->frame_cfg.height / 2) {
    CDBG_FD("p_cfg->face_cfg.min_face_size > p_cfg->frame_cfg.height / 2");
    faceproc_engine_reset(p_engine);
    return FD_RET_INVALID_PARM;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_engine->lib.FACEPROC_SetDtFaceSizeRange(
    p_engine->hdt, p_cfg->face_cfg.min_face_size ,
    p_cfg->face_cfg.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  /* Set Detection Threshold */
  rc = (int) p_engine->lib.FACEPROC_SetDtThreshold(
    p_engine->hdt, DT_THRESHOLD, DT_THRESHOLD);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  /* Create Faceproc result handle */
  p_engine->hresult = p_engine->lib.FACEPROC_CreateDtResult(
    p_cfg->face_cfg.max_num_face_to_detect ,
    (p_cfg->face_cfg.max_num_face_to_detect/2));
  if (!(p_engine->hresult)) {
    CDBG_FD("FACEPROC_CreateDtResult failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  /* Create PT handle */
  p_engine->hpt = p_engine->lib.FACEPROC_PT_CreateHandle();
  if (!(p_engine->hpt)) {
    CDBG_FD("eng_FACEPROC_PT_CreateHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  for (i = 0; i< MAX_ROI; i++) {
    p_engine->hptresult[i] = p_engine->lib.FACEPROC_PT_CreateResultHandle();
    if (!(p_engine->hptresult[i])) {
      CDBG_FD("eng_FACEPROC_PT_CreateResultHandle failed");
      faceproc_engine_reset(p_engine);
      return FD_RET_FAILURE;
    }
  }
  rc = (int) p_engine->lib.FACEPROC_PT_SetMode(p_engine->hpt, PT_MODE_DEFAULT);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("eng_FACEPROC_PT_SetMode failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  rc = (int) p_engine->lib.FACEPROC_PT_SetConfMode(p_engine->hpt, PT_CONF_NOUSE);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("eng_FACEPROC_PT_SetConfMode failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  /* Config CT API */
  p_engine->hct = p_engine->lib.FACEPROC_CT_CreateHandle();
  if (!(p_engine->hct)) {
    CDBG_FD("eng_FACEPROC_CT_CreateHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  for (i = 0; i< MAX_ROI; i++) {
    p_engine->hctresult[i] = p_engine->lib.FACEPROC_CT_CreateResultHandle();
    if (!(p_engine->hctresult[i])) {
      CDBG_FD("eng_FACEPROC_CT_CreateResultHandle failed");
      faceproc_engine_reset(p_engine);
      return FD_RET_FAILURE;
    }
  }
  rc = (int) p_engine->lib.FACEPROC_CT_SetDetectionMode(p_engine->hct,
    CT_DETECTION_MODE_EYE);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("eng_FACEPROC_CT_SetDetectionMode failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }

  p_engine->hsm = p_engine->lib.FACEPROC_SM_CreateHandle();
  if (!(p_engine->hsm)) {
    CDBG_FD("eng_FACEPROC_SM_CreateHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  for (i = 0; i< MAX_ROI; i++) {
    p_engine->hsmresult[i] = p_engine->lib.FACEPROC_SM_CreateResultHandle();
    if (!(p_engine->hsmresult[i])) {
      CDBG_FD("eng_FACEPROC_SM_CreateResultHandle failed");
      faceproc_engine_reset(p_engine);
      return FD_RET_FAILURE;
    }
  }
  /* Face Recognition */
#if(FACE_RECOGNITION)
  p_engine->hfeature = p_engine->lib.FACEPROC_FR_CreateFeatureHandle();
  if (!(p_engine->hfeature)) {
    CDBG_FD("eng_FACEPROC_FR_CreateFeatureHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  fp = fopen(FACE_ALBUM, "r");
  if (fp==NULL) {
    CDBG_FD("Face Album FILE DOESNT EXIST");
    p_engine->halbum = p_engine->lib.FACEPROC_FR_CreateAlbumHandle(MAX_REGISTER_PPL,
      MAX_REGISTER_IMG_PER_PERSON);
  } else {
    CDBG_FD("Face Album FILE EXISTS");
    buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
    if (buffer !=NULL) {
      fread(buffer, 1, MAX_ALBUM_SIZE, fp);
    } else
      CDBG_FD("%s: buffer is null", __func__);
    p_engine->halbum = p_engine->lib.FACEPROC_FR_RestoreAlbum((UINT8 *)buffer,
      MAX_ALBUM_SIZE, &pError);
    if (p_engine->halbum == NULL) {
      CDBG_FD("Fd album is Corrupted %d, hence recreating new one", pError);
      p_engine->halbum = p_engine->lib.FACEPROC_FR_CreateAlbumHandle(MAX_REGISTER_PPL,

        MAX_REGISTER_IMG_PER_PERSON);
    }
    free(buffer);
    fclose(fp);
  }
  if (!(p_engine->halbum)) {
    CDBG_FD("eng_FACEPROC_FR_CreateAlbumHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  p_engine->last_img_registered_idx=0;
#endif

  /* Blink Detection */
  p_engine->hgb = p_engine->lib.FACEPROC_GB_CreateHandle();
  if (!(p_engine->hgb)) {
    CDBG_FD("eng_FACEPROC_GB_CreateHandle failed");
    faceproc_engine_reset(p_engine);
    return FD_RET_FAILURE;
  }
  for (i = 0; i< MAX_ROI; i++) {
    p_engine->hgbresult[i] = p_engine->lib.FACEPROC_GB_CreateResultHandle();
    if (!(p_engine->hgbresult[i])) {
      CDBG_FD("eng_FACEPROC_GB_CreateResultHandle failed");
      faceproc_engine_reset(p_engine);
      return FD_RET_FAILURE;
    }
  }
  /* Save the input frame dimension */
  p_engine->frame_width  = p_cfg->frame_cfg.width;
  p_engine->frame_height = p_cfg->frame_cfg.height;

  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_engine_run -
*
* DESCRIPTION: Execute faceproc engine algorithms
* .
*==========================================================================*/
/* Run */
int faceproc_engine_run( void *Ctrl)
{
  fd_ctrl_t *fdCtrl = (fd_ctrl_t *)Ctrl;
  INT32 i;
  INT32 num_faces;
  int rc;
  fd_frame_t  *p_input_frame = &(fdCtrl->current_frame);
  faceproc_engine_t *p_engine = (faceproc_engine_t *)fdCtrl->local_fd_obj;
  switch (fdCtrl->mode) {
    case FACE_DETECT:
      rc = faceproc_fd_execute(p_engine, p_input_frame, &num_faces);
      if (rc != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      break;
    case FACE_RECOGNIZE:
      rc = faceproc_fd_execute(p_engine, p_input_frame, &num_faces);
      if (rc != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      rc = faceproc_fr_execute(p_engine, p_input_frame, num_faces);
      if (rc != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      break;
    case FACE_REGISTER:
      rc = faceproc_fd_execute(p_engine, p_input_frame, &num_faces);
      if (rc != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      faceproc_fr_execute(p_engine, p_input_frame, num_faces);
      faceproc_register_frame (p_engine, p_input_frame);
      break;
    default :
      CDBG_FD("%s MODE not selected/recognized", __func__);
  }
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_engine_get_output -
*
* DESCRIPTION: Obtain output of algos
* .
*==========================================================================*/
int faceproc_engine_get_output(
  void *Ctrl,
  void *output)
{
  INT32 num_faces;
  frame_proc_fd_data_t *fd_data = (frame_proc_fd_data_t *)output;
  fd_ctrl_t *fdCtrl = (fd_ctrl_t *)Ctrl;
  switch (fdCtrl->mode) {
    case FACE_DETECT:
      if (faceproc_fd_output(fdCtrl, fd_data, &num_faces) != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      break;
    case FACE_RECOGNIZE:
      if (faceproc_fd_output(fdCtrl, fd_data, &num_faces) != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      if (faceproc_fr_output((faceproc_engine_t*)fdCtrl->local_fd_obj,
          fd_data,num_faces) != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      break;
    case FACE_REGISTER:
      if (faceproc_fd_output(fdCtrl, fd_data, &num_faces) != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      if (faceproc_register_output((faceproc_engine_t*)fdCtrl->local_fd_obj,
          fd_data,num_faces) != FD_RET_SUCCESS)
        return FD_RET_FAILURE;
      break;
    default:
      CDBG_FD("%s: Unsupported mode selected", __func__);
  }
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_engine_destroy -
*
* DESCRIPTION: Release Faceproc engine
* .
*==========================================================================*/
/* Destroy */
int faceproc_engine_destroy(void *Ctrl)
{
  fd_ctrl_t *fdCtrl = (fd_ctrl_t *)Ctrl;
  faceproc_engine_t *p_engine = (faceproc_engine_t *)fdCtrl->local_fd_obj;
  FILE *fp;
  uint8_t *buffer;
  UINT32 punSerializedAlbumSize= 0;
  int rc;
  if (!p_engine)
    return FD_RET_FAILURE;
#if(FACE_RECOGNITION)
  /* Save album */
  rc = p_engine->lib.FACEPROC_FR_GetSerializedAlbumSize(p_engine->halbum,
    (UINT32 *)&punSerializedAlbumSize);
  if (rc != FACEPROC_NORMAL) {
    CDBG_FD("%s: eng_FACEPROC_FR_GetSerializedAlbumSize failed", __func__);
    return FD_RET_FAILURE;
  }
  CDBG_FD("Destroy album size to write %d  %d", punSerializedAlbumSize,
      MAX_ALBUM_SIZE);
  if ((punSerializedAlbumSize>0) && (punSerializedAlbumSize < MAX_ALBUM_SIZE)) {
    buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
    if (buffer != NULL) {
      memset(buffer, 0, MAX_ALBUM_SIZE);
      rc = p_engine->lib.FACEPROC_FR_SerializeAlbum(p_engine->halbum, buffer, MAX_ALBUM_SIZE);
      if (rc != FACEPROC_NORMAL) {
        CDBG_FD("%s: eng_FACEPROC_FR_SerializeAlbum failed %d", __func__, rc);
      } else {
        fp = fopen(FACE_ALBUM, "wb");
        if (fp != NULL) {
          fwrite(buffer, 1, MAX_ALBUM_SIZE, fp);
          CDBG_FD("Writing to /data");
          fclose(fp);
        }
      }
      free(buffer);
      buffer = NULL;
    }
  }
#endif
  CDBG_FD("faceproc engine clean\n");
  rc = faceproc_engine_reset(p_engine);
  if (rc != FD_RET_SUCCESS) {
    CDBG_FD("%s: faceproc_engine_reset failed %d", __func__, rc);
  }
  faceproc_unload_lib(p_engine);
  free((void*)p_engine);
  return FD_RET_SUCCESS;
}
/*===========================================================================
* FUNCTION    -  faceproc_engine_reset -
*
* DESCRIPTION: Reset Faceproc engine
* .
*==========================================================================*/
/* Help functions below */
/* Reset */
static int faceproc_engine_reset(
  faceproc_engine_t *p_engine)
{
  CDBG_FD("faceproc engine reset\n");
  INT32 ret, i;

  if (!p_engine)
    return FD_RET_FAILURE;

  /* Delete Result handle */
  if (p_engine->hresult) {
    ret = p_engine->lib.FACEPROC_DeleteDtResult(p_engine->hresult);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hresult = NULL;
  }
  /* Delete Handle */
  if (p_engine->hdt) {
    ret = p_engine->lib.FACEPROC_DeleteDetection(p_engine->hdt);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hdt = NULL;
  }
  if (p_engine->hpt) {
    ret = p_engine->lib.FACEPROC_PT_DeleteHandle(p_engine->hpt);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hpt = NULL;
  }
  for (i =0; i<MAX_ROI; i++) {
    if (p_engine->hptresult[i]) {
      ret = p_engine->lib.FACEPROC_PT_DeleteResultHandle(p_engine->hptresult[i]);
      if (ret != FACEPROC_NORMAL)
        return FD_RET_FAILURE;
      p_engine->hptresult[i] = NULL;
    }
  }
  if (p_engine->hct) {
    ret = p_engine->lib.FACEPROC_CT_DeleteHandle(p_engine->hct);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hct = NULL;
  }
  for (i =0; i<MAX_ROI; i++) {
    if (p_engine->hctresult[i]) {
      ret = p_engine->lib.FACEPROC_CT_DeleteResultHandle(p_engine->hctresult[i]);
      if (ret != FACEPROC_NORMAL)
        return FD_RET_FAILURE;
      p_engine->hctresult[i] = NULL;
    }
  }
  if (p_engine->hsm) {
    ret = p_engine->lib.FACEPROC_SM_DeleteHandle(p_engine->hsm);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hsm = NULL;
  }
  for (i =0; i<MAX_ROI; i++) {
    if (p_engine->hsmresult[i]) {
      ret = p_engine->lib.FACEPROC_SM_DeleteResultHandle(p_engine->hsmresult[i]);
      if (ret != FACEPROC_NORMAL)
        return FD_RET_FAILURE;
      p_engine->hsmresult[i] = NULL;
    }
  }
  /* Face Recognition */
#if(FACE_RECOGNITION)
  if (p_engine->hfeature) {
    ret = p_engine->lib.FACEPROC_FR_DeleteFeatureHandle(p_engine->hfeature);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hfeature = NULL;
  }
  if (p_engine->halbum) {
    ret = p_engine->lib.FACEPROC_FR_DeleteAlbumHandle(p_engine->halbum);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->halbum = NULL;
  }
#endif
  /* Blink/Gaze Detection */
  if (p_engine->hgb) {
    ret = p_engine->lib.FACEPROC_GB_DeleteHandle(p_engine->hgb);
    if (ret != FACEPROC_NORMAL)
      return FD_RET_FAILURE;
    p_engine->hgb = NULL;
  }
  for (i =0; i<MAX_ROI; i++) {
    if (p_engine->hgbresult[i]) {
      ret = p_engine->lib.FACEPROC_GB_DeleteResultHandle(p_engine->hgbresult[i]);
      if (ret != FACEPROC_NORMAL)
        return FD_RET_FAILURE;
      p_engine->hgbresult[i] = NULL;
    }
  }
  /* API to detach allocated memory from Faceproc engine */
  if (p_engine->fdBmemorySet) {
    ret = p_engine->lib.FACEPROC_FreeBMemoryArea();
    p_engine->fdBmemorySet = FALSE;
  }
  /* API to detach allocated memory from Faceproc */
  if (p_engine->fdWmemorySet) {
    p_engine->lib.FACEPROC_FreeWMemoryArea();
    CDBG_FD("Free Wmemory %d", ret);
    p_engine->fdWmemorySet = FALSE;
  }
  /* Scratch buffers */
  if (p_engine->bmem) {
    free(p_engine->bmem);
    p_engine->bmem = NULL;
  }

  if (p_engine->wmem) {
    free(p_engine->wmem);
    p_engine->wmem = NULL;
  }
  return FD_RET_SUCCESS;
}
