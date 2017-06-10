#ifndef _MMSTILLOMXENC_H_
#define _MMSTILLOMXENC_H_

/* ========================================================================= * 
   Purpose:  Auto generated fuzz Library's header file.
 			 Will contain Api signatures of all Wrapper APIs that will
			 invoke the APIs to be fuzzed.
           -------------------------------------------------------
           Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
                 Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

// Include files
#include "FuzzerLibs.h"

/* ---------------------------------------------------------------------------- *
                                  Prototypes
 * ---------------------------------------------------------------------------- */

extern "C" int init_module();
extern "C" int register_apis(regFunc_t regFunc);
extern "C" int deinit_module();
extern "C" int sigWaitFunc(signalCondVar_t signalCondVar);

/*  DO NOT REMOVE --<StartTag_RegisteredApis>-- */
int JpegOMX_FreeHandle_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_ACbCrOffset_WRAP(FuzzedArgs *vArgList);
int JpegOMX_DeInit_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_Thumbnail_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_UserPreferences_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SendJpegCommand_WRAP(FuzzedArgs *vArgList);
int StartEncode_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_ImageInit_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetAllConfigs_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_Exif_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetConfig_WRAP(FuzzedArgs *vArgList);
int JpegOMX_AllFreeBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_EmptyThisBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_ComponentTunnelRequest_WRAP(FuzzedArgs *vArgList);
int JpegOMX_FreeJpegHandle_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetAllParameters_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetConfig_InputCrop_WRAP(FuzzedArgs *vArgList);
int JpegOMX_FreeBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_AllocateBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetHandle_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetConfig_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_ThumbQuality_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetConfig_OutputCrop_WRAP(FuzzedArgs *vArgList);
int JpegOMX_FillThisBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetState_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_ImagePortFormat_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_BufferOffset_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetComponentVersion_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetExtIdx_WRAP(FuzzedArgs *vArgList);
int JpegOMX_Init_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetJpegHandle_WRAP(FuzzedArgs *vArgList);
int JpegOMX_AllUseBuffers_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SendCommand_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_PortDef_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetConfig_Rotate_WRAP(FuzzedArgs *vArgList);
int JpegOMX_UseBuffer_WRAP(FuzzedArgs *vArgList);
int JpegOMX_SetParameter_QFactor_WRAP(FuzzedArgs *vArgList);
int JpegOMX_GetParameter_WRAP(FuzzedArgs *vArgList);
/*  DO NOT REMOVE --<EndTag_RegisteredApis>----<UpdateCodeTag_ListApiSignatures>-- */

// Function to be called if you need to signal a condition 
// variable
int signalWaitFunc(const char *cpCallbFuncName, void *vRetValue, int nRetBytes);

#endif
