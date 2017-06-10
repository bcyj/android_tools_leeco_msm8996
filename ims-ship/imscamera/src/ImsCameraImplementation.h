/*****************************************************************************
 Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 File: ImsCameraImplementation.h
 Description: IMS camera implementation header files

 Revision History
 ===============================================================================
 Date    |   Author's Name    |  BugID  |        Change Description
 ===============================================================================
 03-Jun-2013   Rakesh K M                 -    First version
 *****************************************************************************/
#ifndef IMSCAMERAIMPLEMENTATION_H_
#define IMSCAMERAIMPLEMENTATION_H_
#include "ImsCameraInternalInterface.h"
#include <ICamera.h>
#include <ICameraClient.h>
#include <ICameraServiceListener.h>

using namespace android;

/* IMS_ARCH_64 */
#define VT_UNUSED(v) (void)v
#define VT_UNUSED_PTR(v) (void *)v


typedef enum {
	ENGINE_UNINITIALIZED = 0,
	ENGINE_INITIALIZING = 1,
	ENGINE_INITIALIZED = 2,
	ENGINE_DEINITIALIZING = 3
} eCameraClientState;

class BufferClient : public BnCameraClient {
public:
	virtual ~BufferClient(){ALOGE("~BufferClient");};

	void            notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2){ALOGE("our notifyCallback");};
	void            dataCallback(int32_t msgType, const sp<IMemory>& data,camera_frame_metadata_t *metadata){ALOGE("our dataCallback");};
	void            dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data);
};
class ImsCameraClient {

public:
	static ImsCameraClient* getInstance();
	int init(void* params) ;
	static int release();
	virtual ~ImsCameraClient();
	eCameraClientState state;
	CameraParameters params;
	sp<ICamera>  iCamera;


private:
	ImsCameraClient();

};

#endif /* IMSCAMERAIMPLEMENTATION_H_ */
