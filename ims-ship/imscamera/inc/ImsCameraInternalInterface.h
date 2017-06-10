/*****************************************************************************
 Copyright (c) 2013,2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 File: ImsCameraIntenalInterface.h
 Description: Internal Interface to communicate to IMS camera Interface Library

 Revision History
 ===============================================================================
 Date    |   Author's Name    |  BugID  |        Change Description
 ===============================================================================
 03-Jun-2013   Rakesh K M                 -    First version
  *****************************************************************************/
#include "ImsCameraInterface.h"
#include <utils/StrongPointer.h>
#include "CameraService.h"
#include "CameraHardwareInterface.h"
#include "hardware.h"

using namespace android;

#define CAMERA_ROTATION_CROP_PARAM 1
#define CAMERA_STOP_RECORDING_PARAM 2


typedef void (*ims_custom_params_callback)(void* params);

typedef void (*ims_notify_callback)(int32_t msgType,
                            int32_t ext1,
                            int32_t ext2,
                            void* user);

typedef void (*ims_data_callback)(int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            camera_frame_metadata_t *metadata,
                            void* user);

typedef void (*ims_data_callback_timestamp)(nsecs_t timestamp,
                            int32_t msgType,
                            const sp<IMemory> &dataPtr,
                            void *user);

typedef struct {
	void * cameraParam;
	int cameraMount;
	int cameraFacing;
	int* rotationOffset;
	int width;
	int height;
} CropRotationParam;

typedef struct {
	int type;

	union {
	CropRotationParam cropRotationParam;
	};

} CustomCallbackParam;




#ifdef __cplusplus
extern "C" {
#endif
short registerCallbacks(ims_notify_callback notify_cb,
		ims_data_callback data_cb,
		ims_data_callback_timestamp data_cb_timestamp,
		ims_custom_params_callback custom_cb,
		void* user);
#ifdef __cplusplus
}
#endif
