#ifndef _QCAMERA2HALFUZZLIB_H_
#define _QCAMERA2HALFUZZLIB_H_

/* ========================================================================= * 
   Purpose:  Auto generated fuzz Library's header file.
 			 Will contain Api signatures of all Wrapper APIs that will
			 invoke the APIs to be fuzzed.

   Copyright (c) 2005-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
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
int msg_type_enabled_WRAP(FuzzedArgs *vArgList);
int cancel_picture_WRAP(FuzzedArgs *vArgList);
int Fuzzer_HAL_init_WRAP(FuzzedArgs *vArgList);
int auto_focus_WRAP(FuzzedArgs *vArgList);
int cancel_auto_focus_WRAP(FuzzedArgs *vArgList);
int release_WRAP(FuzzedArgs *vArgList);
int set_parameters_WRAP(FuzzedArgs *vArgList);
int preview_enabled_WRAP(FuzzedArgs *vArgList);
int enable_msg_type_WRAP(FuzzedArgs *vArgList);
int store_meta_data_in_buffers_WRAP(FuzzedArgs *vArgList);
int set_preview_window_WRAP(FuzzedArgs *vArgList);
int put_parameters_WRAP(FuzzedArgs *vArgList);
int disable_msg_type_WRAP(FuzzedArgs *vArgList);
int start_preview_WRAP(FuzzedArgs *vArgList);
int get_parameters_WRAP(FuzzedArgs *vArgList);
int camera_device_open_WRAP(FuzzedArgs *vArgList);
int set_callbacks_WRAP(FuzzedArgs *vArgList);
int stop_recording_WRAP(FuzzedArgs *vArgList);
int stop_preview_WRAP(FuzzedArgs *vArgList);
int take_picture_WRAP(FuzzedArgs *vArgList);
int send_command_WRAP(FuzzedArgs *vArgList);
int close_camera_device_WRAP(FuzzedArgs *vArgList);
int Fuzzer_HAL_deinit_WRAP(FuzzedArgs *vArgList);
int start_recording_WRAP(FuzzedArgs *vArgList);
int release_recording_frame_WRAP(FuzzedArgs *vArgList);
int recording_enabled_WRAP(FuzzedArgs *vArgList);
/*  DO NOT REMOVE --<EndTag_RegisteredApis>----<UpdateCodeTag_ListApiSignatures>-- */

// Function to be called if you need to signal a condition 
// variable
int signalWaitFunc(const char *cpCallbFuncName, void *vRetValue, int nRetBytes);

#endif
